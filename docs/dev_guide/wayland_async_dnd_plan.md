# Wayland Asynchronous DND Implementation Plan

## Objective
Replace the current blocking implementation of Drag-and-Drop (DND) payload reading in the Wayland backend with a non-blocking, asynchronous mechanism integrated into the main event loop, while keeping event semantics predictable for users.

## The Problem
Currently, `_data_device_handle_enter` calls `_lvkw_wayland_read_data_offer`, which performs a blocking `read()` on a pipe with a 1-second timeout. If the DND source is slow to provide the `text/uri-list` data, or if the payload is large, the entire application UI freezes during the `enter` event. This is unacceptable for a high-performance library.

## Proposed Solution
We will implement an **Asynchronous Accumulation** strategy. Instead of waiting for the data to arrive, we will monitor the pipe's file descriptor (FD) as part of our standard event pumping loop.

## Event Contract (Two-Phase Hover)
To keep behavior consistent and easy for users:
1. On `enter`, emit `LVKW_EVENT_TYPE_DND_HOVER` immediately with:
   - `entered = true`
   - `paths = NULL`, `path_count = 0`
2. When payload reading/parsing completes for the same offer, emit another `LVKW_EVENT_TYPE_DND_HOVER` with:
   - `entered = false`
   - resolved `paths`/`path_count`
3. `drop` uses the completed payload if available. If transfer fails or times out, drop is still emitted with empty paths and a diagnostic.

### 1. State Management (`wayland_internal.h`)
Add fields to `LVKW_Context_WL` to track the pending DND read:
```c
struct {
  int fd;                  // The pipe FD from wl_data_offer_receive
  uint8_t *buffer;         // Accumulation buffer
  size_t size;             // Current data size
  size_t capacity;         // Buffer capacity
  struct wl_data_offer *offer; // The offer being read
  LVKW_Window_WL *window;  // Window associated with this read
  uint32_t serial;         // Offer serial at read start
  bool drop_pending;       // Drop happened before read completion
} dnd_async_read;
```

### 2. Initiation (`wayland_input.c`)
Modify `_data_device_handle_enter`:
- Instead of calling the blocking read helper, call `wl_data_offer_receive`.
- Store the resulting FD in `ctx->dnd_async_read.fd`.
- Store the current `offer` to validate it hasn't changed.
- Set the FD to non-blocking mode (`O_NONBLOCK`).
- Emit immediate `DND_HOVER(entered=true)` with empty payload.

### 3. Event Loop Integration (`wayland_events.c`)
Modify `lvkw_ctx_pumpEvents_WL`:
- Include `ctx->dnd_async_read.fd` in the `pollfd` array if it is `>= 0`.
- In the `poll()` result handling:
    - If the DND FD has `POLLIN`, read available bytes into `ctx->dnd_async_read.buffer`.
    - If `read()` returns 0 (EOF):
        1. Parse the buffer into paths.
        2. If offer/window still match active DND session, emit `DND_HOVER(entered=false)` with payload.
        3. If `drop_pending` is set, emit `DND_DROP` now (with payload or empty payload on parse failure).
        4. Close the FD and reset the state machine.
    - If `read()` returns an error (other than `EAGAIN`):
        - Log a diagnostic and abort the read.

### 4. Safety & Concurrency
- **Offer Mismatch**: If a new `enter` event occurs while `dnd_async_read.fd` is active, the old FD must be closed and the buffer cleared before starting the new read.
- **Leave Events**: On `leave`, cancel pending async read and clear state with no payload caching.
- **Context Destruction**: Ensure the FD is closed during `lvkw_ctx_destroy_WL`.
- **Drop Before Completion**: If `drop` arrives before payload completion, set `drop_pending` and defer final drop emission until completion/failure.
- **Timeouts**: Keep a bounded async timeout to avoid stuck sessions on broken sources.

## Implementation Steps
1.  **Phase 1: Struct Updates**. Update `wayland_internal.h` and initialization/cleanup in `wayland_context.c`.
2.  **Phase 2: Non-Blocking Read Helper**. Refactor `_lvkw_wayland_read_data_offer` logic into a state-machine-friendly function.
3.  **Phase 3: Poll Integration**. Update `wayland_events.c` to handle the DND FD.
4.  **Phase 4: Parsing & Emission**. Move path parsing logic into the async completion handler and implement the two-phase hover contract.

## Verification
- **Test Case 1**: Large file drag (e.g., 500 files at once). Ensure no frame drops.
- **Test Case 2**: Drag enter/leave rapidly. Ensure no FD leaks or crashes.
- **Test Case 3**: Slow source (simulated via a mock source if needed). Ensure UI remains responsive.
- **Test Case 4**: Drop before payload completion. Ensure exactly one drop event is emitted after completion/failure.
