# The Event System & Input Pipeline

## Consuming Events

### Direct Dispatch Model
Event consumption in LVKW follows a **Push-based, Direct Dispatch** model. There is no internal double-buffered queue managed by the core library.

1.  **Register Callback:** During context creation, you provide an `event_callback` and optional `event_userdata` in the `LVKW_ContextAttributes`.
2.  **`lvkw_events_pump`:** Triggers the backend to process OS events. As the backend identifies events, it immediately invokes your registered callback.
3.  **Sync Events:** After a logical group of events (like a single Wayland frame) or at the end of a `pumpEvents` call, a `LVKW_EVENT_TYPE_SYNC` event is dispatched to signify that the application state should be updated or rendered.

### Event Lifetime & Safety
**CRITICAL:** The `LVKW_Event*` pointer passed to your callback is **transient**. Many event payloads contain pointers (marked as `LVKW_TRANSIENT` in headers) that are only valid for the duration of the callback.

**Do not store these pointers or the event pointer itself.** If you need to save event data for later processing, copy the data (e.g., `memcpy` the struct or `strdup` strings).

### Event Masking
The **Context Attribute** `event_mask` acts as a global filter. Events not included in this mask are ignored by the backend and never trigger a callback. This is useful for performance optimization if your application only cares about a subset of inputs.

## Thread Safety

`lvkw_events_pump` must be called from the primary thread (the thread that created the context).

However, you can safely post events from any thread using **`lvkw_events_post`**. These events are queued in a lock-free notification ring and will be dispatched by the primary thread during the next `lvkw_events_pump` call.

## Keyboard Handling Layers

LVKW separates keyboard input into three distinct layers.

### 1. Physical Keys (`LVKW_KeyboardEvent`)
*   Raw Keyboard events.
*   `LVKW_Key` values are based on the standard US QWERTY layout. For example, `LVKW_KEY_A` refers to the key to the right of Caps Lock, regardless of whether the user is using a QWERTY, AZERTY, or Dvorak software layout.
*   Key repeats do NOT trigger this event. Only the initial press and eventual release make their way to your callback.
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

**The raw_delta Contract:**
1.  `raw_delta` **must** represent unaccelerated hardware movement.
2.  If raw hardware data is unavailable (due to backend limitations or current window state), `raw_delta` **must** be `{0, 0}`.
3.  LVKW will **never** simulate `raw_delta` by de-accelerating `delta` or using other heuristics. 0 means 0.

**Note:** `raw_delta` is most reliable on some backends (like Wayland) only when the cursor is in `LVKW_CURSOR_LOCKED` mode.

## Drag and Drop

LVKW integrates file drag-and-drop into the event system.

1.  **`LVKW_EVENT_TYPE_DND_HOVER`:** Fired repeatedly while a file is dragged over your window.
    *   **Feedback:** You must update `*event->dnd_hover.feedback->action` to tell the OS if you accept the drop (e.g., set it to `LVKW_DND_ACTION_COPY`). The default is typically `LVKW_DND_ACTION_NONE` (reject).
2.  **`LVKW_EVENT_TYPE_DND_DROP`:** Fired when the user releases the mouse button.
    *   Contains the final list of paths and the position.
    *   **Lifetime:** The path strings are valid only during the callback. Copy them if you need to load files asynchronously.

### Persistent Session Userdata

A DND operation is a sequence of events (`HOVER`... `HOVER`... `DROP` or `LEAVE`). You often need to calculate whether a drop is valid once (e.g., parsing file extensions) and reuse that result in subsequent hover frames.

*   **Mechanism:** `*event->dnd_hover.feedback->session_userdata` (or `dnd_drop.session_userdata` / `dnd_leave.session_userdata`).
*   **Behavior:** This pointer persists for the duration of the drag session. You can allocate a structure on the first hover event, store it here, and access/free it on the Drop or Leave event.
