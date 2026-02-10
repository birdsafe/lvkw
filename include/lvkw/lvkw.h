#ifndef LVKW_LIBRARY_H_INCLUDED
#define LVKW_LIBRARY_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "lvkw/details/lvkw_details.h"
#include "lvkw/details/lvkw_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Version Info --- */

/** @brief A structure representing the library version. */
typedef struct LVKW_Version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
} LVKW_Version;

/** @brief Returns the version of the library.
 * @return The library version. */
LVKW_Version lvkw_getVersion(void);

/* --- Opaque Handles --- */

/** @brief An opaque handle to the library's main context. */
typedef struct LVKW_Context LVKW_Context;
/** @brief An opaque handle to a window instance. */
typedef struct LVKW_Window LVKW_Window;

/* --- Basic Types --- */

/** @brief Simple 2D dimensions (width and height). */
typedef struct LVKW_Size {
  uint32_t width;
  uint32_t height;
} LVKW_Size;

/** @brief Status codes returned by the API. */
typedef enum LVKW_Status {
  /** @brief The operation succeeded. */
  LVKW_SUCCESS = 0,
  /** @brief The operation failed, but your handles are still safe. */
  LVKW_ERROR = 1,
  /** @brief The operation failed and the window handle is now dead. */
  LVKW_ERROR_WINDOW_LOST = 2,
  /** @brief The operation failed and the entire context is dead. */
  LVKW_ERROR_CONTEXT_LOST = 3,
} LVKW_Status;

/** @brief Specific diagnostic codes used for detailed error reporting. */
typedef enum LVKW_Diagnosis {
  LVKW_DIAGNOSIS_NONE = 0,
  LVKW_DIAGNOSIS_OUT_OF_MEMORY,
  LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE,
  LVKW_DIAGNOSIS_DYNAMIC_LIB_FAILURE,
  LVKW_DIAGNOSIS_FEATURE_UNSUPPORTED,
  LVKW_DIAGNOSIS_BACKEND_FAILURE,
  LVKW_DIAGNOSIS_VULKAN_FAILURE,
  LVKW_DIAGNOSIS_UNKNOWN,

  /* Debug Diagnoses (Unrecoverable / UB in Release) */

  // This means you have messed up something in the parameters of an api call.
  LVKW_DIAGNOSIS_INVALID_ARGUMENT,

  // This means the API call you have made is invalid in the current app state
  LVKW_DIAGNOSIS_PRECONDITION_FAILURE,

  // This means something unexpected happened. It almost certainly
  // implies a bug or oversight within lvkw itself. Sorry :(. Please report it
  // if you can.
  LVKW_DIAGNOSIS_INTERNAL,
} LVKW_Diagnosis;

/* --- Events --- */

/** @brief Tracks whether a button is currently pressed or released. */
typedef enum LVKW_ButtonState {
  LVKW_BUTTON_STATE_RELEASED = 0,
  LVKW_BUTTON_STATE_PRESSED = 1,
} LVKW_ButtonState;

/** @brief Identifiers for physical keyboard keys. */
typedef enum LVKW_Key {
#include "lvkw/details/lvkw_keys.inc.h"
} LVKW_Key;

/** @brief Flags for modifier keys like Shift, Ctrl, and Alt. */
typedef enum LVKW_ModifierFlags {
  LVKW_MODIFIER_SHIFT = 1,
  LVKW_MODIFIER_CONTROL = 2,
  LVKW_MODIFIER_ALT = 4,
  LVKW_MODIFIER_SUPER = 8,
  LVKW_MODIFIER_CAPS_LOCK = 16,
  LVKW_MODIFIER_NUM_LOCK = 32,
} LVKW_ModifierFlags;

/** @brief Identifiers for mouse buttons. */
typedef enum LVKW_MouseButton {
  LVKW_MOUSE_BUTTON_LEFT = 0,
  LVKW_MOUSE_BUTTON_RIGHT = 1,
  LVKW_MOUSE_BUTTON_MIDDLE = 2,
  LVKW_MOUSE_BUTTON_4 = 3,
  LVKW_MOUSE_BUTTON_5 = 4,
  LVKW_MOUSE_BUTTON_6 = 5,
  LVKW_MOUSE_BUTTON_7 = 6,
  LVKW_MOUSE_BUTTON_8 = 7,
} LVKW_MouseButton;

/** @brief Tells you the window is ready to start rendering. */
typedef struct LVKW_WindowReadyEvent {
  char _unused;  // Empty structs are not allowed in standard C
} LVKW_WindowReadyEvent;

/** @brief Tells you the user (or system) wants to close the window. */
typedef struct LVKW_WindowCloseEvent {
  char _unused;
} LVKW_WindowCloseEvent;

/** @brief Fired when the window or its framebuffer changes size. */
typedef struct LVKW_WindowResizedEvent {
  LVKW_Size size;
  LVKW_Size framebufferSize;
} LVKW_WindowResizedEvent;

/** @brief Fired when a keyboard key is pressed or released. */
typedef struct LVKW_KeyboardEvent {
  LVKW_Key key;
  LVKW_ButtonState state;
  LVKW_ModifierFlags modifiers;
} LVKW_KeyboardEvent;

/** @brief Fired when the mouse moves (includes absolute and relative motion). */
typedef struct LVKW_MouseMotionEvent {
  double x;  /**< Current absolute X position. */
  double y;  /**< Current absolute Y position. */
  double dx; /**< Relative X motion since last event. */
  double dy; /**< Relative Y motion since last event. */
} LVKW_MouseMotionEvent;

/** @brief Fired when a mouse button is pressed or released. */
typedef struct LVKW_MouseButtonEvent {
  LVKW_MouseButton button;
  LVKW_ButtonState state;
} LVKW_MouseButtonEvent;

/** @brief Fired when the mouse wheel or touchpad is scrolled. */
typedef struct LVKW_MouseScrollEvent {
  double dx; /**< Horizontal scroll amount. */
  double dy; /**< Vertical scroll amount. */
} LVKW_MouseScrollEvent;

/** @brief Tells you if the system is currently idle or active. */
typedef struct LVKW_IdleEvent {
  uint32_t timeout_ms; /**< The threshold used for this idle check. */
  bool is_idle;        /**< True if we just went idle; false if we're back. */
} LVKW_IdleEvent;

/** @brief The various types of events the library can generate. */
typedef enum LVKW_EventType {
  LVKW_EVENT_TYPE_CLOSE_REQUESTED = 1,     /**< Someone wants to close the window. */
  LVKW_EVENT_TYPE_WINDOW_RESIZED = 2,      /**< The window changed size. */
  LVKW_EVENT_TYPE_KEY = 4,                 /**< A keyboard key was toggled. */
  LVKW_EVENT_TYPE_WINDOW_READY = 8,        /**< The window is ready for work. */
  LVKW_EVENT_TYPE_MOUSE_MOTION = 16,       /**< The mouse moved. */
  LVKW_EVENT_TYPE_MOUSE_BUTTON = 32,       /**< A mouse button was toggled. */
  LVKW_EVENT_TYPE_MOUSE_SCROLL = 64,       /**< The mouse wheel spun. */
  LVKW_EVENT_TYPE_IDLE_NOTIFICATION = 128, /**< The idle state changed. */
  LVKW_EVENT_TYPE_ALL = 0xFFFFFFFF,        /**< A mask to catch every event type. */
} LVKW_EventType;

/** @brief A container for any event generated by the library. */
typedef struct LVKW_Event {
  LVKW_EventType type;
  LVKW_Window *window;
  union {
    LVKW_WindowReadyEvent window_ready;
    LVKW_WindowCloseEvent close_requested;
    LVKW_WindowResizedEvent resized;
    LVKW_KeyboardEvent key;
    LVKW_MouseMotionEvent mouse_motion;
    LVKW_MouseButtonEvent mouse_button;
    LVKW_MouseScrollEvent mouse_scroll;
    LVKW_IdleEvent idle;
  };
} LVKW_Event;

/* --- Custom Allocation and Diagnosis Management --- */

typedef void *(*LVKW_AllocationFunction)(size_t size, void *userdata);
typedef void (*LVKW_FreeFunction)(void *ptr, void *userdata);

/** @brief A structure for plugging in your own memory allocator. */
typedef struct LVKW_Allocator {
  LVKW_AllocationFunction alloc_cb; /**< Your custom allocation function. */
  LVKW_FreeFunction free_cb;        /**< Your custom deallocation function. */
} LVKW_Allocator;

/** @brief Holds details about a diagnostic message or error. */
typedef struct LVKW_DiagnosisInfo {
  LVKW_Diagnosis diagnosis; /**< The specific diagnostic code. */
  const char *message;      /**< A human-readable explanation of what happened. */
  LVKW_Context *context;    /**< The context where this happened. */
  LVKW_Window *window;      /**< The window where this happened (if any). */
} LVKW_DiagnosisInfo;

/** @brief A callback you provide to receive diagnostic messages. */
typedef void (*LVKW_DiagnosisCallback)(const LVKW_DiagnosisInfo *info, void *userdata);

/** @brief Internal structure for the library context. */
struct LVKW_Context {
  void *userdata;                      /**< Your custom context-wide data. */
  LVKW_DiagnosisCallback diagnosis_cb; /**< Your error reporting callback. */
  void *diagnosis_userdata;            /**< User data for your diagnostic callback. */
  bool is_lost;                        /**< True if this context is now dead. */
};

/** @brief Internal structure for a window instance. */
struct LVKW_Window {
  void *userdata; /**< Your custom window-specific data. */
  bool is_lost;   /**< True if this window is now dead. */
  bool is_ready;  /**< True if this window is ready for rendering. */
};

/* --- Context Management --- */

#define LVKW_IDLE_NEVER 0

/** @brief Flags for updatable context attributes. */
typedef enum LVKW_ContextAttributesField {
  LVKW_CTX_ATTR_IDLE_TIMEOUT = 1 << 0, /**< Update the idle timeout. */
  LVKW_CTX_ATTR_INHIBIT_IDLE = 1 << 1, /**< Toggle idle inhibition. */
} LVKW_ContextAttributesField;

/** @brief A set of global properties that can be updated for a context. */
typedef struct LVKW_ContextAttributes {
  uint32_t idle_timeout_ms; /**< The threshold for idle notifications (ms). */
  bool inhibit_idle;        /**< Prevent the system from going idle/sleeping. */
} LVKW_ContextAttributes;

/** @brief Tells the library which backend to use. */
typedef enum LVKW_BackendType {
  /** @brief Let the library pick the best available backend. */
  LVKW_BACKEND_AUTO = 0,
  /** @brief Use Wayland (Linux only). */
  LVKW_BACKEND_WAYLAND = 1,
  /** @brief Use X11 (Linux only). */
  LVKW_BACKEND_X11 = 2,
  /** @brief Use the Win32 API (Windows only). */
  LVKW_BACKEND_WIN32 = 3,
} LVKW_BackendType;

/** @brief Options for creating a new library context. */
typedef struct LVKW_ContextCreateInfo {
  LVKW_Allocator allocator;            /**< Custom allocator (or NULL for default). */
  LVKW_DiagnosisCallback diagnosis_cb; /**< Callback for error and status reports. */
  void *diagnosis_userdata;            /**< User data for the diagnosis callback. */
  void *userdata;                      /**< Global user data for the context. */
  LVKW_BackendType backend;            /**< Preferred backend (default is AUTO). */
  LVKW_ContextAttributes attributes;   /**< Initial global attributes. */
} LVKW_ContextCreateInfo;

/** @brief A macro for a creation structure filled with safe defaults. */
#define LVKW_CONTEXT_CREATE_INFO_DEFAULT                                                                     \
  {                                                                                                          \
    .backend = LVKW_BACKEND_AUTO, .attributes = {.idle_timeout_ms = LVKW_IDLE_NEVER, .inhibit_idle = false } \
  }

/** @brief Creates a new LVKW context.
 *
 * NOTE: The diagnosis_cb in create_info is the PRIMARY mechanism for
 * detecting detailed failures.
 *
 * When a context or window is lost (check the `is_lost` field), it MUST be
 * destroyed.
 *
 * In release builds (where LVKW_ENABLE_DIAGNOSIS is not defined), diagnosis
 * reporting may be entirely disabled for performance reasons.
 *
 * @param create_info Pointer to the structure containing creation information.
 * @param out_context Pointer to a pointer where the new context handle will be
 * stored.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);

/** @brief Destroys a context and cleans up all its resources.
 *
 * @param ctx_handle The context handle to destroy.
 */
void lvkw_ctx_destroy(LVKW_Context *ctx_handle);

/** @brief Updates specific attributes of an existing context.
 *
 * @param ctx_handle The context handle.
 * @param field_mask A bitmask of LVKW_ContextAttributesField specifying which
 * fields to update.
 * @param attributes Pointer to the structure containing the new values.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask, const LVKW_ContextAttributes *attributes);

/** @brief Helper to update the idle timeout of a context.
 *
 * @param ctx The context handle.
 * @param timeout_ms The new idle timeout in milliseconds.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_ctx_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_ContextAttributes attrs = {0};
  attrs.idle_timeout_ms = timeout_ms;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs);
}

/** @brief Helper to toggle idle inhibition of a context.
 *
 * @param ctx The context handle.
 * @param enabled True to prevent the system from going idle, false otherwise.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_ctx_setIdleInhibition(LVKW_Context *ctx, bool enabled) {
  LVKW_ContextAttributes attrs = {0};
  attrs.inhibit_idle = enabled;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs);
}

/* --- Vulkan Integration --- */

/** @brief Returns the Vulkan instance extensions required by this context.
 *
 * @param ctx_handle The context handle.
 * @param out_count Pointer to a uint32_t that will receive the number of required
 * extensions.
 * @return A pointer to a null-terminated array of extension names managed by the
 * library.
 */
const char *const *lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *out_count);

/* --- Event Polling --- */

typedef void (*LVKW_EventCallback)(const LVKW_Event *evt, void *userdata);

/** @brief Polls for waiting events and sends them to your callback.
 *
 * @param ctx_handle The context handle.
 * @param event_mask A bitmask specifying which event types to poll for.
 * @param callback The callback function to receive dispatched events.
 * @param userdata User data pointer to be passed to the callback.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata);

/** @brief Blocks until events arrive or a timeout expires, then dispatches them.
 *
 * Unlike pollEvents, this function blocks until at least one event is available
 * or the timeout expires.
 *
 * @param ctx_handle The context handle.
 * @param timeout_ms The timeout in milliseconds. Use LVKW_IDLE_NEVER to wait indefinitely.
 * @param event_mask A bitmask specifying which event types to poll for.
 * @param callback The callback function to receive dispatched events.
 * @param userdata User data pointer to be passed to the callback.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata);

/* --- Window Management --- */

/** @brief Different ways the cursor can behave. */
typedef enum LVKW_CursorMode {
  /** @brief Default behavior: visible and free. */
  LVKW_CURSOR_NORMAL = 0,
  /** @brief Hide the cursor and lock it to the window. */
  LVKW_CURSOR_LOCKED = 2,
} LVKW_CursorMode;

/** @brief Standard OS cursor shapes you can use. */
typedef enum LVKW_CursorShape {
  LVKW_CURSOR_SHAPE_DEFAULT = 1,
  LVKW_CURSOR_SHAPE_CONTEXT_MENU = 2,
  LVKW_CURSOR_SHAPE_HELP = 3,
  LVKW_CURSOR_SHAPE_POINTER = 4,
  LVKW_CURSOR_SHAPE_PROGRESS = 5,
  LVKW_CURSOR_SHAPE_WAIT = 6,
  LVKW_CURSOR_SHAPE_CELL = 7,
  LVKW_CURSOR_SHAPE_CROSSHAIR = 8,
  LVKW_CURSOR_SHAPE_TEXT = 9,
  LVKW_CURSOR_SHAPE_VERTICAL_TEXT = 10,
  LVKW_CURSOR_SHAPE_ALIAS = 11,
  LVKW_CURSOR_SHAPE_COPY = 12,
  LVKW_CURSOR_SHAPE_MOVE = 13,
  LVKW_CURSOR_SHAPE_NO_DROP = 14,
  LVKW_CURSOR_SHAPE_NOT_ALLOWED = 15,
  LVKW_CURSOR_SHAPE_GRAB = 16,
  LVKW_CURSOR_SHAPE_GRABBING = 17,
  LVKW_CURSOR_SHAPE_E_RESIZE = 18,
  LVKW_CURSOR_SHAPE_N_RESIZE = 19,
  LVKW_CURSOR_SHAPE_NE_RESIZE = 20,
  LVKW_CURSOR_SHAPE_NW_RESIZE = 21,
  LVKW_CURSOR_SHAPE_S_RESIZE = 22,
  LVKW_CURSOR_SHAPE_SE_RESIZE = 23,
  LVKW_CURSOR_SHAPE_SW_RESIZE = 24,
  LVKW_CURSOR_SHAPE_W_RESIZE = 25,
  LVKW_CURSOR_SHAPE_EW_RESIZE = 26,
  LVKW_CURSOR_SHAPE_NS_RESIZE = 27,
  LVKW_CURSOR_SHAPE_NESW_RESIZE = 28,
  LVKW_CURSOR_SHAPE_NWSE_RESIZE = 29,
  LVKW_CURSOR_SHAPE_COL_RESIZE = 30,
  LVKW_CURSOR_SHAPE_ROW_RESIZE = 31,
  LVKW_CURSOR_SHAPE_ALL_SCROLL = 32,
  LVKW_CURSOR_SHAPE_ZOOM_IN = 33,
  LVKW_CURSOR_SHAPE_ZOOM_OUT = 34,
} LVKW_CursorShape;

/** @brief Hints to the OS about what kind of content you're showing. */
typedef enum LVKW_ContentType {
  LVKW_CONTENT_TYPE_NONE = 0,
  LVKW_CONTENT_TYPE_PHOTO = 1,
  LVKW_CONTENT_TYPE_VIDEO = 2,
  LVKW_CONTENT_TYPE_GAME = 3,
} LVKW_ContentType;

/** @brief Flags for live-updatable window attributes. */

typedef enum LVKW_WindowAttributesField {
  LVKW_WND_ATTR_TITLE = 1 << 0,        /**< Update the window title. */
  LVKW_WND_ATTR_SIZE = 1 << 1,         /**< Update the window size. */
  LVKW_WND_ATTR_FULLSCREEN = 1 << 2,   /**< Toggle fullscreen mode. */
  LVKW_WND_ATTR_CURSOR_MODE = 1 << 3,  /**< Change cursor behavior. */
  LVKW_WND_ATTR_CURSOR_SHAPE = 1 << 4, /**< Change cursor appearance. */
} LVKW_WindowAttributesField;

/** @brief A set of window properties that can be updated after creation. */

typedef struct LVKW_WindowAttributes {
  const char *title;             /**< The title of the window (UTF-8). */
  LVKW_Size size;                /**< The logical width and height. */
  bool fullscreen;               /**< Whether the window is fullscreen. */
  LVKW_CursorMode cursor_mode;   /**< How the cursor behaves. */
  LVKW_CursorShape cursor_shape; /**< What the cursor looks like. */
} LVKW_WindowAttributes;

/** @brief Options for creating a new window. */

typedef struct LVKW_WindowCreateInfo {
  LVKW_WindowAttributes attributes; /**< Mutable properties (title, size, etc). */
  const char *app_id;               /**< An ID used for shell integration. */
  LVKW_ContentType content_type;    /**< A hint about the window's content. */
  bool transparent;                 /**< Enable window transparency (Creation only). */
  void *userdata;                   /**< Custom data for this specific window. */
} LVKW_WindowCreateInfo;

/** @brief A macro for a window creation structure filled with safe defaults. */
#define LVKW_WINDOW_CREATE_INFO_DEFAULT                       \
  {.attributes = {.title = "LVKW Window",                     \
                  .size = {800, 600},                         \
                  .fullscreen = false,                        \
                  .cursor_mode = LVKW_CURSOR_NORMAL,          \
                  .cursor_shape = LVKW_CURSOR_SHAPE_DEFAULT}, \
   .app_id = "lvkw.app",                                      \
   .content_type = LVKW_CONTENT_TYPE_NONE,                    \
   .transparent = false}

/** @brief Creates a new window instance within the given context.
 *
 * @param ctx_handle The context handle.
 * @param create_info Pointer to the structure containing window creation
 * information.
 * @param out_window Pointer to a pointer where the new window handle will be
 * stored.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window);

/** @brief Destroys a window and cleans up its resources.
 *
 * @param window_handle The window handle to destroy.
 */
void lvkw_wnd_destroy(LVKW_Window *window_handle);

/** @brief Updates specific attributes of an existing window.
 *
 * @param window_handle The window handle.
 * @param field_mask A bitmask of LVKW_WindowAttributesField specifying which
 * fields to update.
 * @param attributes Pointer to the structure containing the new values.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask, const LVKW_WindowAttributes *attributes);

/** @brief Helper to update the title of a window.
 *
 * @param window The window handle.
 * @param title The new title (UTF-8).
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_wnd_setTitle(LVKW_Window *window, const char *title) {
  LVKW_WindowAttributes attrs = {0};
  attrs.title = title;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_TITLE, &attrs);
}

/** @brief Helper to update the logical size of a window.
 *
 * @param window The window handle.
 * @param size The new logical size.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_wnd_setSize(LVKW_Window *window, LVKW_Size size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.size = size;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_SIZE, &attrs);
}

/** @brief Helper to toggle fullscreen mode of a window.
 *
 * @param window The window handle.
 * @param enabled True to enable fullscreen, false for windowed.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_wnd_setFullscreen(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.fullscreen = enabled;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_FULLSCREEN, &attrs);
}

/** @brief Helper to update the cursor mode of a window.
 *
 * @param window The window handle.
 * @param mode The new cursor mode.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_mode = mode;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR_MODE, &attrs);
}

/** @brief Helper to update the cursor shape of a window.
 *
 * @param window The window handle.
 * @param shape The new cursor shape.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_wnd_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_shape = shape;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR_SHAPE, &attrs);
}

/** @brief Creates a Vulkan surface for a window.
 *
 * @param window_handle The window handle.
 * @param instance The Vulkan instance.
 * @param out_surface Pointer to a VkSurfaceKHR that will receive the created
 * surface.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface);

/** @brief Gets the current size of the window's framebuffer.
 *
 * @param window_handle The window handle.
 * @param out_size Pointer to a LVKW_Size structure that will receive the
 * framebuffer dimensions.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_wnd_getFramebufferSize(LVKW_Window *window_handle, LVKW_Size *out_size);

/** @brief Returns the context that created this window.
 *
 * @param window_handle The window handle.
 * @return The context handle that owns this window.
 */
LVKW_Context *lvkw_wnd_getContext(LVKW_Window *window_handle);

/** @brief Asks the OS to give this window input focus.
 *
 * @param window_handle The window handle.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle);

/********** PRIVATE API METHODS ***********/
void _lvkw_reportDiagnosis(LVKW_Context *ctx_handle, LVKW_Window *window_handle, LVKW_Diagnosis diagnosis,
                           const char *message);

#ifdef __cplusplus
}
#endif

#endif