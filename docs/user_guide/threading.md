# Threading Deep Dive

This page defines the practical concurrency contract for one `LVKW_Context`.

## Core Model

1. The thread that calls `lvkw_context_create` becomes that context's primary thread.
2. Primary-thread APIs must run on that thread.
3. Some read/event APIs are callable from worker threads, but only with the synchronization rules below.
4. LVKW does not provide global internal locking for arbitrary concurrent calls on the same context.

## API Concurrency Classes

### Class A: Primary-thread-only APIs

These must execute on the context's primary thread.

- `lvkw_context_destroy`
- `lvkw_context_update`
- `lvkw_events_pump`
- `lvkw_events_commit`
- `lvkw_display_createWindow`
- `lvkw_display_destroyWindow`
- `lvkw_display_updateWindow`
- `lvkw_display_createVkSurface`
- `lvkw_display_requestWindowFocus`
- `lvkw_data_setClipboardText`
- `lvkw_data_getClipboardText`
- `lvkw_data_setClipboardData`
- `lvkw_data_getClipboardData`
- `lvkw_data_getClipboardMimeTypes`
- `lvkw_display_createCursor`
- `lvkw_display_destroyCursor`
- `lvkw_input_createController` / `lvkw_input_destroyController` / `lvkw_input_getControllerInfo` / `lvkw_input_setControllerHapticLevels`

### Class B: Any-thread APIs (with lifetime synchronization)

Callable from any thread, but do not race against destruction of the referenced context/window.

- `lvkw_display_listVkExtensions`
- `lvkw_display_getStandardCursor`

### Class C: Any-thread APIs (with event/state synchronization)

Callable from any thread, but must be externally synchronized with event/state writers.

- `lvkw_events_scan`
- `lvkw_display_listMonitors`
- `lvkw_display_listMonitorModes`
- `lvkw_instrumentation_getMetrics`
- `lvkw_display_getWindowGeometry`

### Class D: Any-thread lock-free API

- `lvkw_events_post`

`lvkw_events_post` is intended for cross-thread wakeups/user events and is safe without external synchronization.

### Class E: Process-global pure function

- `lvkw_core_getVersion`

## The Critical Rule: `scanEvents` vs `commitEvents`

Treat these as operations on one shared event snapshot:

- `lvkw_events_scan` = reader
- `lvkw_events_commit` = writer

On the same context, calls to these must be externally synchronized. A reader/writer lock is ideal:

- multiple concurrent scanners are fine
- no scanner may run while `commitEvents` runs

If you do not want an RW lock, a plain mutex is also correct (more conservative).

## Synchronization Patterns

### Pattern 1: Single mutex (simple and safe)

Use one mutex for all Class C accesses on a context.

```cpp
std::mutex ctx_mtx;

// Primary thread
{
  std::lock_guard<std::mutex> lock(ctx_mtx);
  lvkw_events_pump(ctx, 0);
  lvkw_events_commit(ctx);
  lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, callback, userdata);
}

// Worker
{
  std::lock_guard<std::mutex> lock(ctx_mtx);
  LVKW_WindowGeometry g;
  lvkw_display_getWindowGeometry(window, &g);
}
```

### Pattern 2: RW lock for event-heavy engines

Use shared lock for `scanEvents`; exclusive lock for `commitEvents`.

```cpp
std::shared_mutex evt_rw;

// Reader thread(s)
{
  std::shared_lock<std::shared_mutex> lock(evt_rw);
  lvkw_events_scan(ctx, mask, callback, userdata);
}

// Primary thread writer
lvkw_events_pump(ctx, timeout_ms);  // no shared snapshot mutation here
{
  std::unique_lock<std::shared_mutex> lock(evt_rw);
  lvkw_events_commit(ctx);
}
```

### Pattern 3: Lifetime lock

Protect destroy paths and handle users with a lifetime lock/order rule:

- no handle dereference may overlap `lvkw_context_destroy` / `lvkw_display_destroyWindow`
- Class B APIs require this even though they are any-thread

## Event Payload Lifetime Rules

The callback gets `const LVKW_Event* evt` and `LVKW_Window* window`.

1. `evt` pointer is callback-scoped. Never store it.
2. Pointer fields inside payloads (text/IME/DnD paths/feedback pointers) are borrowed. Copy them if needed later or cross-thread.
3. Handle fields (for example monitor pointers in monitor events) require lifetime synchronization before cross-thread use.

Practical rule: if it is a pointer, either copy it now or synchronize lifetime explicitly.

## No Cross-Thread Opt-In Flag

LVKW does not use a context-creation opt-in for cross-thread usage.

Always use the per-function contract above to decide legality, and provide the required external synchronization when using any-thread APIs.

## Multi-Context Parallelism

Separate contexts can run on separate threads in parallel.

This is often the cleanest way to scale, because synchronization is then local to each context rather than process-wide.
