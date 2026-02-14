# The Event System & Input Pipeline

## Consuming Events

### Synchronization & Scanning
Event consumption in LVKW follows a **Synchronize -> Scan** model, powered by a double-buffered queue:

1.  **`lvkw_ctx_syncEvents`:** Advanced the event state. It performs two atomic actions:
    *   **Promotion:** All events received since the last sync are promoted to a "stable" snapshot.
    *   **Pumping:** It optionally blocks until new events arrive (based on the `timeout` parameter).
    *   Use a timeout of `0` for non-blocking polling, or `LVKW_NEVER` to wait indefinitely.
2.  **`lvkw_ctx_scanEvents`:** Iterates over the **stable snapshot** and invokes your callback for matching events. Crucially, this is **non-destructive** and **thread-safe** (with external synchronization); you can scan the same set of events multiple times (e.g., from different systems) and they will remain consistent until the next `syncEvents` call.

### High-Level Shorthands
For most applications, the `lvkw-shortcuts.h` header provides convenient one-call wrappers:
*   **`lvkw_ctx_pollEvents`:** A non-blocking `sync` + `scan` cycle.
*   **`lvkw_waitEvents`:** A blocking `sync` + `scan` cycle.

### Event Lifetime & Safety
**CRITICAL:** The `LVKW_Event*` pointer passed to your callback is **transient**. What happens to the memory it points to once your callback returns is entirely backend and event-type dependant.

**Do not store this pointer.** If you need to save an event for later processing, copy the data (e.g., `memcpy` the struct).

### Event Masking
Both scan and sync functions are affected by masking. 
*   The **Context Attribute** `event_mask` acts as a global filter; events not in this mask are never even added to the queue.
*   The `event_mask` passed to `scanEvents` (and the shorthands) allows you to selectively process only what you need during a specific pass.

## The Event Queue

LVKW uses a **double-buffered** event queue to ensure temporal coherence and thread isolation. 

*   **Isolation:** While you are scanning the "stable" buffer, the backend can safely continue pushing new asynchronous events into the "active" buffer without corrupting your scan.
*   **Consistency:** Every scan performed between two `syncEvents` calls is guaranteed to see the exact same sequence of events.

### Tail Compression

To prevent the event queue from flooding your application with redundant updates (especially on high-polling-rate mice), LVKW automatically merges consecutive events of the same type for the same window in the **active** buffer.

The following event types are subject to tail compression:
*   `LVKW_EVENT_TYPE_MOUSE_MOTION`: Consecutive motion events are merged. The `position` is updated to the latest value, and `delta` / `raw_delta` are accumulated.
*   `LVKW_EVENT_TYPE_MOUSE_SCROLL`: Consecutive scroll events are merged by accumulating the `delta`.
*   `LVKW_EVENT_TYPE_WINDOW_RESIZED`: Only the latest geometry is kept.

In other words, you may receive a single `MouseMotionEvent` that represents multiple hardware updates, and you'll be none-the-wiser.

You may also receive multiple `MouseMotionEvent` in a given frame. This can happen when the user clicks or presses a key, etc. In that case you will get `MouseMotionEvent` -> `MouseButtonEvent` -> `MouseMotionEvent` all within the same frame. This allows you to keep as much precision as possible. But it also means that you shouldn't assume that a `MouseMotionEvent` being received means that this is automatically where the mouse ends up at the end of the frame.

### Queue Eviction

The event queue has a fixed maximum capacity (configurable via `LVKW_ContextTuning`) to avoid arbitrarily large memory usage during degenerate conditions. There has to be *some* kind of upper bound. 

While this should be, especially with tuning, an exceptionally rare event, it will always remain possible for the queue to become full. In the unlikely situation where that happens, LVKW follows the following logic:

1.  The queue attempts to find and discard an older event that belongs to a compressible category (e.g., an older mouse motion event).
2.  If no compressible event is found and the queue cannot grow (reached `max_capacity`), the new event is dropped.

You can monitor if and how often events are being dropped using the [Telemetry system](telemetry.md).

### A note on cache performance

Events have a **hard** limit on how big they are allowed to get (enforced in [lvkw_abi_checks.h](../include/lvkw/details/lvkw_abi_checks.h)) To ensure they remain cache-friendly. So you may occasionally run into unintuitive field ordering for the sake of squeezing all the necessary info into the tight budget. 

## Keyboard Handling Layers

LVKW separates keyboard input into three distinct layers.

### 1. Physical Keys (`LVKW_KeyboardEvent`)
*   Raw Keyboard events.
*   `LVKW_Key` values are based on the standard US QWERTY layout. For example, `LVKW_KEY_A` refers to the key to the right of Caps Lock, regardless of whether the user is using a QWERTY, AZERTY, or Dvorak software layout.
*   Key repeats do NOT trigger this event. Only the initial press and eventual released make their way to your callback.
*   **Use Case:** Game controls

### 2. Text Input (`LVKW_TextInputEvent`)
*   The operating system's interpretation of the key press as a character.
*   Provides a UTF-8 encoded string.
*   You **will** receive repeated copies of this event when the key is held down (subject to OS settings).
*   **Use Case:** Simple text input

### 3. IME Composition (`LVKW_TextCompositionEvent`)
*   Intermediate state for Input Method Editors (IMEs), commonly used for languages like Japanese, Chinese, or Korean.
*   **Behavior:**
    *   As the user types, you receive `TextCompositionEvent` updates. The expectation is that you would render this "pre-edit" text (often underlined) in your UI.
    *   When the user confirms the composition (e.g., presses Enter), you receive a final `TextInputEvent`, and the composition ends.
*   **Use Case:** High-quality text input support for international users.

## Mouse Input

### Logical vs. Raw Delta
The `LVKW_MouseMotionEvent` struct provides two delta values:

*   **`delta`:** The movement of the mouse according to the OS. This usually includes acceleration and ballistics. Use this for moving a UI cursor.
*   **`raw_delta`:** The unaccelerated, raw movement from the hardware. Use this for 3D camera controls (e.g., first-person look) to ensure 1:1 mouse-to-view movement.

**Note:** `raw_delta` is most reliable on some backends when the cursor is in `LVKW_CURSOR_LOCKED` mode.

## Drag and Drop

LVKW integrates file drag-and-drop into the event system.

1.  **`LVKW_EVENT_TYPE_DND_HOVER`:** Fired repeatedly while a file is dragged over your window.
    *   You receive a list of file paths.
    *   **Feedback:** You must update `event->dnd_hover.feedback->action` to tell the OS if you accept the drop (e.g., set it to `LVKW_DND_ACTION_COPY`). The default is typically `LVKW_DND_ACTION_NONE` (reject), so if you don't set this, the OS will likely show a "forbidden" cursor.
2.  **`LVKW_EVENT_TYPE_DND_DROP`:** Fired when the user releases the mouse button.
    *   Contains the final list of paths and the position.
    *   **Lifetime:** The path strings are valid only during the callback. Copy them if you need to load files asynchronously.

### Persistent Session Userdata

A DND operation is a sequence of events (`HOVER`... `HOVER`... `DROP` or `LEAVE`). You often need to calculate whether a drop is valid once (e.g., parsing file extensions) and reuse that result in subsequent hover frames to avoid expensive re-checks.

*   **Mechanism:** `event->dnd_hover.feedback->session_userdata` (or `dnd_drop.session_userdata` / `dnd_leave.session_userdata`).
*   **Behavior:** This pointer persists for the duration of the drag session. You can allocate a structure on the first hover event, store it here, and access/free it on the Drop or Leave event.
