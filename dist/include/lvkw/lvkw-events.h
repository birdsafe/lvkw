#ifndef LVKW_EVENTS_H_INCLUDED
#define LVKW_EVENTS_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw-core.h"
#include "lvkw-input.h"
#include "lvkw-monitor.h"
#include "lvkw-window.h"
#include "lvkw/details/lvkw_details.h"

/**
 * @file lvkw-events.h
 * @brief Event system and input handling.
 */

#ifdef LVKW_ENABLE_CONTROLLER
#include "lvkw-ext-controller.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* --- Event Payloads --- */

/** @brief Fired when the OS/Windowing system has fully allocated resources for the window.
 *  Vulkan surface creation is only safe after this event. */
typedef struct LVKW_WindowReadyEvent {
  char _unused;  // Empty structs are not allowed in standard C
} LVKW_WindowReadyEvent;

/** @brief Fired when the user or system requests the window to close.
 *  The window is NOT automatically destroyed; you need to call lvkw_window_destroy() for that. */
typedef struct LVKW_WindowCloseEvent {
  char _unused;  // Empty structs are not allowed in standard C
} LVKW_WindowCloseEvent;

/** @brief Fired when the window dimensions or its backbuffer size changes. */
typedef struct LVKW_WindowResizedEvent {
  LVKW_WindowGeometry geometry;  ///< The new geometry of the window after resizing.
} LVKW_WindowResizedEvent;

/** @brief Fired when the window is maximized or restored to normal size. */
typedef struct LVKW_WindowMaximizationEvent {
  bool maximized;  ///< True if the window is now maximized, false if it has been restored.
} LVKW_WindowMaximizationEvent;

/** @brief Fired when a physical keyboard key is pressed or released. */
typedef struct LVKW_KeyboardEvent {
  LVKW_Key key;                  ///< Scancode-independent key identifier.
  LVKW_ButtonState state;        ///< Pressed or Released.
  LVKW_ModifierFlags modifiers;  ///< Bitmask of active modifiers (Shift, Ctrl, etc.).
} LVKW_KeyboardEvent;

/** @brief Fired when the mouse pointer moves. */
typedef struct LVKW_MouseMotionEvent {
  LVKW_LogicalVec position;   ///< Absolute position in logical units.
  LVKW_LogicalVec delta;      ///< Accelerated relative movement since last event.
  LVKW_LogicalVec raw_delta;  ///< Unaccelerated "raw" movement (useful for camera control).
} LVKW_MouseMotionEvent;

/** @brief Fired when a mouse button is pressed or released. */
typedef struct LVKW_MouseButtonEvent {
  LVKW_MouseButton button;       ///< Which button was pressed or released.
  LVKW_ButtonState state;        ///< Pressed or Released.
  LVKW_ModifierFlags modifiers;  ///< Bitmask of active modifiers (Shift, Ctrl, etc.).
} LVKW_MouseButtonEvent;

/** @brief Fired when a scroll wheel or touchpad gesture occurs. */
typedef struct LVKW_MouseScrollEvent {
  LVKW_LogicalVec delta;  ///< Scroll offset in logical units.
} LVKW_MouseScrollEvent;

/** @brief Fired when the system's idle state changes based on the context's idle_timeout. */
typedef struct LVKW_IdleEvent {
  uint32_t timeout_ms;  ///< The threshold that was crossed.
  bool is_idle;         ///< True if the system is now idle.
} LVKW_IdleEvent;

/** @brief Fired when a display monitor is connected or disconnected. */
typedef struct LVKW_MonitorConnectionEvent {
  LVKW_Monitor *monitor;  ///< Information about the monitor that was connected or disconnected.
  bool connected;            ///< True if the monitor was connected, false if it was disconnected.
} LVKW_MonitorConnectionEvent;

/** @brief Fired when a monitor's mode (resolution/refresh rate) or scale changes. */
typedef struct LVKW_MonitorModeEvent {
  LVKW_Monitor *monitor;  ///< Information about the monitor that changed mode.
} LVKW_MonitorModeEvent;

/** @brief Fired when the user inputs text, potentially involving IMEs or complex key combinations.
 *  This provides processed UTF-8 characters, whereas LVKW_KeyboardEvent provides raw physical keys. */
typedef struct LVKW_TextInputEvent {
  const char *text;  ///< Null-terminated UTF-8 string. Valid only during callback.
  uint32_t length;   ///< Length of the string in bytes (excluding null terminator).
} LVKW_TextInputEvent;

/** @brief Fired during active IME composition.
 *  Allows the application to render "preedit" text directly in its UI. */
typedef struct LVKW_TextCompositionEvent {
  const char *text;          ///< The current preedit string (UTF-8). Valid only during callback.
  uint32_t length;           ///< Length of the string in bytes (excluding null terminator).
  uint32_t cursor_index;     ///< Byte offset of the cursor within the preedit string.
  uint32_t selection_length; ///< Byte length of the selection within the preedit string.
} LVKW_TextCompositionEvent;

/** @brief Fired when the window gains or loses focus from the OS. */
typedef struct LVKW_FocusEvent {
  bool focused;  ///< True if the window is now focused, false if it has lost focus.
} LVKW_FocusEvent;

/** @brief Feedback provided by the application during a DND session. */
typedef struct LVKW_DndFeedback {
  LVKW_DndAction *action;        ///< [Out] Application writes its desired action here.
  void **session_userdata;       ///< Persistent state for the duration of this DND session.
} LVKW_DndFeedback;

/** @brief Fired when a drag operation is active over the window. */
typedef struct LVKW_DndHoverEvent {
  LVKW_LogicalVec position;      ///< Current cursor position in logical units.
  LVKW_DndFeedback *feedback;    ///< Pointers to session feedback state.
  const char **paths;            ///< Array of UTF-8 file paths. Valid only during the callback.
  uint16_t path_count;           ///< Number of file paths.
  LVKW_ModifierFlags modifiers;  ///< Active modifiers (Shift, Ctrl, etc.).
  bool entered;                  ///< True if this is the first hover event of the sequence.
} LVKW_DndHoverEvent;

/** @brief Fired when the drag operation leaves the window without dropping. */
typedef struct LVKW_DndLeaveEvent {
  void **session_userdata;       ///< Access to the session state for cleanup.
} LVKW_DndLeaveEvent;

/** @brief Fired when items are dropped onto the window. */
typedef struct LVKW_DndDropEvent {
  LVKW_LogicalVec position;      ///< Drop position in logical units.
  void **session_userdata;       ///< Access to the session state for final processing/cleanup.
  const char **paths;            ///< Array of UTF-8 file paths. Valid only during the callback.
  uint16_t path_count;           ///< Number of file paths.
  LVKW_ModifierFlags modifiers;  ///< Active modifiers (Shift, Ctrl, etc.).
} LVKW_DndDropEvent;

/** @brief Bitmask for filtering events during polling or waiting. */
typedef enum LVKW_EventType {
  LVKW_EVENT_TYPE_ALL = 0xFFFFFFFF,  ///< Catch-all for all supported event types.

  LVKW_EVENT_TYPE_CLOSE_REQUESTED = 1 << 0,
  LVKW_EVENT_TYPE_WINDOW_RESIZED = 1 << 1,
  LVKW_EVENT_TYPE_KEY = 1 << 2,
  LVKW_EVENT_TYPE_WINDOW_READY = 1 << 3,
  LVKW_EVENT_TYPE_MOUSE_MOTION = 1 << 4,
  LVKW_EVENT_TYPE_MOUSE_BUTTON = 1 << 5,
  LVKW_EVENT_TYPE_MOUSE_SCROLL = 1 << 6,
  LVKW_EVENT_TYPE_IDLE_NOTIFICATION = 1 << 7,
  LVKW_EVENT_TYPE_MONITOR_CONNECTION = 1 << 8,
  LVKW_EVENT_TYPE_MONITOR_MODE = 1 << 9,
  LVKW_EVENT_TYPE_TEXT_INPUT = 1 << 11,
  LVKW_EVENT_TYPE_FOCUS = 1 << 12,
  LVKW_EVENT_TYPE_WINDOW_MAXIMIZED = 1 << 13,
  LVKW_EVENT_TYPE_DND_HOVER = 1 << 14,
  LVKW_EVENT_TYPE_DND_LEAVE = 1 << 15,
  LVKW_EVENT_TYPE_DND_DROP = 1 << 16,
  LVKW_EVENT_TYPE_TEXT_COMPOSITION = 1 << 17,

  /* ----- Extention events. ----- */

  /* ----- LVKW_ENABLE_CONTROLLER ----- */
  LVKW_EVENT_TYPE_CONTROLLER_CONNECTION = 1 << 30,
} LVKW_EventType;

/** @brief Unified event structure passed to application callbacks. 
*   @note On a 64-bit system, This is kept at 48/32 bytes in double/floats configs.
*/
typedef struct LVKW_Event {
  union {
    LVKW_WindowReadyEvent window_ready;
    LVKW_WindowCloseEvent close_requested;
    LVKW_WindowResizedEvent resized;
    LVKW_WindowMaximizationEvent maximized;
    LVKW_KeyboardEvent key;
    LVKW_MouseMotionEvent mouse_motion;
    LVKW_MouseButtonEvent mouse_button;
    LVKW_MouseScrollEvent mouse_scroll;
    LVKW_IdleEvent idle;
    LVKW_MonitorConnectionEvent monitor_connection;
    LVKW_MonitorModeEvent monitor_mode;
#ifdef LVKW_ENABLE_CONTROLLER
    LVKW_CtrlConnectionEvent controller_connection;
#endif
    LVKW_TextInputEvent text_input;
    LVKW_TextCompositionEvent text_composition;
    LVKW_FocusEvent focus;
    LVKW_DndHoverEvent dnd_hover;
    LVKW_DndLeaveEvent dnd_leave;
    LVKW_DndDropEvent dnd_drop;
  };
} LVKW_Event;

/**
 * @brief Application callback for event processing.
 * @note **Lifetime:** The @p evt pointer in the callback is valid only during that call.
 * @param evt Read-only pointer to the event data.
 * @param userdata User-provided pointer from the poll/wait call.
 */
typedef void (*LVKW_EventCallback)(  LVKW_EventType type,  LVKW_Window *window, const LVKW_Event *evt, void *userdata);

/**
 * @brief Dispatches all currently pending events to the provided callback.
 * @note **Non-blocking:** Returns immediately if no events are in the queue.
 * @param ctx_handle Active context.
 * @param event_mask Bitmask of LVKW_EventType to process.
 * @param callback Function to invoke for each matching event.
 * @param userdata Passed through to the callback.
 */
LVKW_HOT LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                         LVKW_EventCallback callback, void *userdata);

/**
 * @brief Blocks the thread until events are available or a timeout occurs.
 * @note **Blocking:** This will put the thread to sleep until an event arrives or the timeout is reached. Use
 * LVKW_NEVER for infinite wait.
 * @param ctx_handle Active context.
 * @param timeout_ms Max time to wait. Use LVKW_NEVER for infinite wait.
 * @param event_mask Bitmask of LVKW_EventType to process.
 * @param callback Function to invoke for each matching event.
 * @param userdata Passed through to the callback.
 */
LVKW_HOT LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                         LVKW_EventCallback callback, void *userdata);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_EVENTS_H_INCLUDED
