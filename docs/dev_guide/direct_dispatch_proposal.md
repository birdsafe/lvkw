# Change Proposal: Stateless Direct Event Dispatch

## 1. Objective
To move `lvkw` to a stateless, queue-less architecture. This eliminates the "double-queuing" penalty while providing a high-fidelity representation of OS input.

## 2. The "Raw Stream" Architecture
Core `lvkw` will no longer buffer, coalesce, or store events. It acts as a stateless translator between the OS and the user.

### 2.1 Context Attributes
The callback is registered via `LVKW_ContextAttributes`.

```c
typedef struct LVKW_ContextAttributes {
  // ...
  LVKW_EventCallback event_callback;
  void *event_userdata;
} LVKW_ContextAttributes;
```

### 2.2 Immediate Dispatch
Backends will trigger the `event_callback` immediately upon receiving an event from the OS, with the exception of **Protocol Atomic Frames** (see 2.4).

### 2.3 The Sync Point (`LVKW_EVENT_TYPE_SYNC`)
A new event type is introduced to indicate the end of a logical group of events.
*   **Wayland:** Triggered by `wl_pointer.frame`, `wl_touch.frame`, etc.
*   **Others:** Triggered once at the end of each `lvkw_events_pump()` call.

### 2.4 Wayland Marshalling (Internal Frame Buffering)
To preserve protocol atomicity, the Wayland backend will internally buffer events belonging to a single frame.
*   Callbacks are **deferred** until the `.frame` event is received.
*   If `lvkw_events_pump()` returns while a frame is incomplete, the buffered events persist in the backend and are dispatched in the subsequent `pump()` call once the frame completes.
*   This ensures the user never receives a "split" frame.

## 3. Transient Data Lifetime
The lifetime of `LVKW_TRANSIENT` data (e.g., `LVKW_TextInputEvent.text`) is limited to the **scope of the callback**.
*   Users wishing to defer processing must copy the data.
*   This removes the need for a global `transient_pool` in the core library.

## 4. The Utility Queue (`lvkw_util_queue.h`)
The existing sophisticated queuing and coalescing logic is extracted into an optional utility library.

### 4.1 Features
1.  **Ordered Coalescing:** Merging adjacent `MOUSE_MOTION` events while respecting interleaving events.
2.  **Cross-Pump Accumulation:** Allowing multiple `pump()` calls to fill a single stable event frame.
3.  **Transient Pool:** Interning `LVKW_TRANSIENT` data so it remains valid until the next user-triggered `commit`.

## 5. Summary of Benefits
*   **No "Middleman" Tax:** Engines receive events directly with zero redundant copies.
*   **No Data Loss:** High-frequency sub-frame data is preserved for specialized apps.
*   **Architectural Purity:** `lvkw` remains a "Low-Level" tool, fulfilling its core mission.
