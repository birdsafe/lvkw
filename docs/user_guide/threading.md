# Threading Deep Dive

This page defines the practical concurrency contract for one `LVKW_Context`.

## Core Model

1. The thread that calls `lvkw_createContext` becomes that context's primary thread.
2. Primary-thread APIs must run on that thread.
3. Some read/event APIs are callable from worker threads, but only with the synchronization rules below.
4. LVKW does not provide global internal locking for arbitrary concurrent calls on the same context.

## API Concurrency Classes

### Class A: Primary-thread-only APIs

These must execute on the context's primary thread.

- `lvkw_ctx_destroy`
- `lvkw_ctx_update`
- `lvkw_ctx_syncEvents`
- `lvkw_ctx_createWindow`
- `lvkw_wnd_destroy`
- `lvkw_wnd_update`
- `lvkw_wnd_createVkSurface`
- `lvkw_wnd_requestFocus`
- `lvkw_wnd_setClipboardText`
- `lvkw_wnd_getClipboardText`
- `lvkw_wnd_setClipboardData`
- `lvkw_wnd_getClipboardData`
- `lvkw_wnd_getClipboardMimeTypes`
- `lvkw_ctx_createCursor`
- `lvkw_cursor_destroy`
- `lvkw_ctrl_create` / `lvkw_ctrl_destroy` / `lvkw_ctrl_getInfo` / `lvkw_ctrl_setHapticLevels`

### Class B: Any-thread APIs (with lifetime synchronization)

Callable from any thread, but do not race against destruction of the referenced context/window.

- `lvkw_ctx_getVkExtensions`
- `lvkw_ctx_getStandardCursor`
- `lvkw_wnd_getContext`

### Class C: Any-thread APIs (with event/state synchronization)

Callable from any thread, but must be externally synchronized with event/state writers.

- `lvkw_ctx_scanEvents`
- `lvkw_ctx_getMonitors`
- `lvkw_ctx_getMonitorModes`
- `lvkw_ctx_getTelemetry`
- `lvkw_wnd_getGeometry`

### Class D: Any-thread lock-free API

- `lvkw_ctx_postEvent`

`lvkw_ctx_postEvent` is intended for cross-thread wakeups/user events and is safe without external synchronization.

### Class E: Process-global pure function

- `lvkw_getVersion`

## The Critical Rule: `scanEvents` vs `syncEvents`

Treat these as operations on one shared event snapshot:

- `lvkw_ctx_scanEvents` = reader
- `lvkw_ctx_syncEvents` = writer

On the same context, calls to these must be externally synchronized. A reader/writer lock is ideal:

- multiple concurrent scanners are fine
- no scanner may run while `syncEvents` runs

If you do not want an RW lock, a plain mutex is also correct (more conservative).

## Synchronization Patterns

### Pattern 1: Single mutex (simple and safe)

Use one mutex for all Class C accesses on a context.

```cpp
std::mutex ctx_mtx;

// Primary thread
{
  std::lock_guard<std::mutex> lock(ctx_mtx);
  lvkw_ctx_syncEvents(ctx, 0);
  lvkw_ctx_scanEvents(ctx, LVKW_EVENT_TYPE_ALL, callback, userdata);
}

// Worker
{
  std::lock_guard<std::mutex> lock(ctx_mtx);
  LVKW_WindowGeometry g;
  lvkw_wnd_getGeometry(window, &g);
}
```

### Pattern 2: RW lock for event-heavy engines

Use shared lock for `scanEvents`; exclusive lock for `syncEvents`.

```cpp
std::shared_mutex evt_rw;

// Reader thread(s)
{
  std::shared_lock<std::shared_mutex> lock(evt_rw);
  lvkw_ctx_scanEvents(ctx, mask, callback, userdata);
}

// Primary thread writer
{
  std::unique_lock<std::shared_mutex> lock(evt_rw);
  lvkw_ctx_syncEvents(ctx, timeout_ms);
}
```

### Pattern 3: Lifetime lock

Protect destroy paths and handle users with a lifetime lock/order rule:

- no handle dereference may overlap `lvkw_ctx_destroy` / `lvkw_wnd_destroy`
- Class B APIs require this even though they are any-thread

## Event Payload Lifetime Rules

The callback gets `const LVKW_Event* evt` and `LVKW_Window* window`.

1. `evt` pointer is callback-scoped. Never store it.
2. Pointer fields inside payloads (text/IME/DnD paths/feedback pointers) are borrowed. Copy them if needed later or cross-thread.
3. Handle fields (for example monitor pointers in monitor events) require lifetime synchronization before cross-thread use.

Practical rule: if it is a pointer, either copy it now or synchronize lifetime explicitly.

## About `LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API`

This flag does not convert LVKW into a fully internally-synchronized API.

It permits cross-thread use for APIs that explicitly support it, but synchronization duties remain with the application.

Always use the per-function contract above, not the flag alone, to decide legality.

## Multi-Context Parallelism

Separate contexts can run on separate threads in parallel.

This is often the cleanest way to scale, because synchronization is then local to each context rather than process-wide.
