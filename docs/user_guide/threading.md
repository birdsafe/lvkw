# Threading Models & Concurrency

## The Default Model: Thread-Bound

**Rule:** All LVKW API calls for a given context (and its associated windows/monitors) **MUST** originate from the thread that created the context.

*   **Implication:**
    *   `syncEvents` (and the `poll`/`wait` shorthands) must be on the main thread.
    *   Window creation (`createWindow`) and destruction (`destroyWindow`) are main-thread bound.
    *   Attribute updates (`wnd_setTitle`) and geometry queries (`getGeometry`) are main-thread bound.
*   **Why:** This aligns with the single-threaded nature of most OS windowing APIs (Win32, Cocoa) and ensures predictable behavior for event processing and window management.
*   **Debug Mode:** In debug builds (with `LVKW_VALIDATE_API_CALLS`), LVKW rigorously checks thread affinity and will `abort()` if a call is made from the wrong thread.

## The Hybrid Model (Opt-in)

The Hybrid model allows specific, safe operations from worker threads. That does NOT mean that the operation is guaranteed to be commited immediately. Certain changes on certain backends may have to wait until the next `syncEvents` call.

To enable, pass the flag during context creation:

```c
LVKW_ContextCreateInfo create_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
create_info.flags = LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API; 
// ...
```

### Allowed Operations
With this flag, the following operations become thread-safe **IF** external synchronization is provided:

*   **Event Scanning:** `lvkw_ctx_scanEvents` (Non-destructive inspection of the current queue).
*   **Attribute Updates:** `lvkw_wnd_update` (e.g., changing title, size, fullscreen state).
*   **Geometry Queries:** `lvkw_wnd_getGeometry`.
*   **Haptics:** `lvkw_ctrl_setHapticLevels`.
*   **Monitor Info:** `lvkw_ctx_getMonitors`, `lvkw_ctx_getMonitorModes`.
*   **Pointer dereferences:** Access to data (both reads and writes) via pointeres provided by the library.

### Still Forbidden
Even with the Hybrid flag, the following remain strictly main-thread bound:

*   **Context/Window Lifecycle:** `createContext`, `destroyContext`, `createWindow`, `destroyWindow`.
*   **Event Pumping:** `syncEvents`.
*   **Vulkan Surface Creation:** `wnd_createVkSurface`.

### Crucial: External Synchronization

The Hybrid model does **NOT** make LVKW internally thread-safe. It just directs backends to take measures against environment-specific thread affinity requirements.

**You represent:** That no two threads will call *any* LVKW function on the same context simultaneously.

**Implementation:** You must wrap all LVKW calls (both main thread and worker threads) with a mutex or similar synchronization primitive.

```cpp
// Example: Safe usage with std::mutex
std::mutex lvkw_mutex;

// Main Thread
{
    std::lock_guard<std::mutex> lock(lvkw_mutex);
    ctx.syncEvents();
    lvkw::scanEvents(ctx, ...);
     
}

// Worker Thread
{
    std::lock_guard<std::mutex> lock(lvkw_mutex);
    window.setTitle("Loading: 50%"); // Safe!
}
```

**Failure to synchronize will result in undefined behavior, crashes, or data corruption.**

## Multi-Context Parallelism

You can safely run multiple independent LVKW contexts in separate threads. Each context manages its own event loop and resources. This is the recommended approach for applications that need to manage windows on multiple displays from separate high-performance threads.
