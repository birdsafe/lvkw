#ifndef LVKW_WINDOW_H_INCLUDED
#define LVKW_WINDOW_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw-core.h"
#include "lvkw-events.h"
#include "lvkw-monitor.h"
#include "lvkw/details/lvkw_details.h"

/**
 * @file lvkw-window.h
 * @brief Window creation, management, and OS surface integration.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Snapshot of window dimensions in both coordinate systems. */
typedef struct LVKW_WindowGeometry {
  LVKW_LogicalVec logicalSize; ///< Scaled coordinates for UI layout.
  LVKW_PixelVec pixelSize;     ///< Raw dimensions for framebuffer/swapchain allocation.
} LVKW_WindowGeometry;

/* --- Window Management --- */

/** @brief Runtime status flags for a window. */
typedef enum LVKW_WindowFlags {
  LVKW_WND_STATE_LOST = 1 << 0,      ///< Window is dead. Operations will fail and it should be destroyed.
  LVKW_WND_STATE_READY = 1 << 1,     ///< Window is fully initialized and ready for rendering/surface creation.
  LVKW_WND_STATE_FOCUSED = 1 << 2,   ///< Window has keyboard/input focus.
  LVKW_WND_STATE_MAXIMIZED = 1 << 3, ///< Window is currently maximized.
} LVKW_WindowFlags;

/**
 * @brief Opaque handle representing an OS-level window.
 * @note **Thread Affinity:** All operations on a window must occur on the thread that created its parent context.
 */
struct LVKW_Window {
  void *userdata;  ///< User-controlled pointer. You CAN override it directly.
  uint32_t flags;  ///< Bitmask of LVKW_WindowFlags. Read-only.
};

/** @brief Cursor visibility and constraint modes. */
typedef enum LVKW_CursorMode {
  LVKW_CURSOR_NORMAL = 0, ///< Visible and free to leave the window.
  LVKW_CURSOR_LOCKED = 2, ///< Hidden and confined to the window. Delivers raw relative motion.
} LVKW_CursorMode;

/** @brief Standard OS cursor shapes. */
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

/** @brief Semantic hints for OS compositor optimization (e.g., Variable Refresh Rate). */
typedef enum LVKW_ContentType {
  LVKW_CONTENT_TYPE_NONE = 0,
  LVKW_CONTENT_TYPE_PHOTO = 1,
  LVKW_CONTENT_TYPE_VIDEO = 2,
  LVKW_CONTENT_TYPE_GAME = 3,
} LVKW_ContentType;

/** @brief Bitmask for selecting which attributes to update in lvkw_wnd_update(). */
typedef enum LVKW_WindowAttributesField {
  LVKW_WND_ATTR_TITLE = 1 << 0,
  LVKW_WND_ATTR_LOGICAL_SIZE = 1 << 1,
  LVKW_WND_ATTR_FULLSCREEN = 1 << 2,
  LVKW_WND_ATTR_CURSOR_MODE = 1 << 3,
  LVKW_WND_ATTR_CURSOR_SHAPE = 1 << 4,
  LVKW_WND_ATTR_MONITOR = 1 << 5,
  LVKW_WND_ATTR_MAXIMIZED = 1 << 6,
  LVKW_WND_ATTR_MIN_SIZE = 1 << 7,
  LVKW_WND_ATTR_MAX_SIZE = 1 << 8,
  LVKW_WND_ATTR_ASPECT_RATIO = 1 << 9,
  LVKW_WND_ATTR_RESIZABLE = 1 << 10,
  LVKW_WND_ATTR_DECORATED = 1 << 11,
} LVKW_WindowAttributesField;

/** @brief Live-updatable window properties. */
typedef struct LVKW_WindowAttributes {
  const char *title;             ///< UTF-8 window title.
  LVKW_LogicalVec logicalSize;   ///< Requested window size in logical units.
  bool fullscreen;               ///< If true, window occupies the entire monitor.
  bool maximized;                ///< If true, window is expanded to fill the workspace.
  LVKW_CursorMode cursor_mode;   ///< Visibility and lock state.
  LVKW_CursorShape cursor_shape; ///< Standard OS pointer icon.
  LVKW_MonitorId monitor;        ///< Monitor for fullscreen/maximization. Use LVKW_MONITOR_ID_INVALID for default.
  LVKW_LogicalVec minSize;       ///< Hard lower bound for resizing. {0,0} for no limit.
  LVKW_LogicalVec maxSize;       ///< Hard upper bound for resizing. {0,0} for no limit.
  LVKW_Ratio aspect_ratio;       ///< Forced proportions. {0,0} for unconstrained.
  bool resizable;                ///< If false, the OS prevents user-initiated resizing.
  bool decorated;                ///< If false, the window has no borders or title bar.
} LVKW_WindowAttributes;

/** @brief Parameters for lvkw_ctx_createWindow(). */
typedef struct LVKW_WindowCreateInfo {
  LVKW_WindowAttributes attributes; ///< Initial runtime attributes.
  const char *app_id;               ///< Platform-specific ID for shell integration (e.g., desktop file name).
  LVKW_ContentType content_type;    ///< Hint for compositor latency/refresh optimizations.
  bool transparent;                 ///< Enable alpha blending with the desktop. Set at creation only.
  void *userdata;                   ///< Initial value for LVKW_Window::userdata.
} LVKW_WindowCreateInfo;

/**
 * @brief Default initialization macro for LVKW_WindowCreateInfo.
 */
#define LVKW_WINDOW_CREATE_INFO_DEFAULT                      \
  {                                                          \
    .attributes = {                                          \
      .title = "LVKW Window",                                \
      .logicalSize = {800, 600},                             \
      .fullscreen = false,                                   \
      .maximized = false,                                    \
      .cursor_mode = LVKW_CURSOR_NORMAL,                     \
      .cursor_shape = LVKW_CURSOR_SHAPE_DEFAULT,             \
      .monitor = LVKW_MONITOR_ID_INVALID,                    \
      .minSize = {0, 0},                                     \
      .maxSize = {0, 0},                                     \
      .aspect_ratio = {0, 0},                                \
      .resizable = true,                                     \
      .decorated = true                                      \
    },                                                       \
    .app_id = "lvkw.app",                                    \
    .content_type = LVKW_CONTENT_TYPE_NONE,                  \
    .transparent = false                                     \
  }

/**
 * @brief Creates a new window.
 * @param ctx_handle Active context.
 * @param create_info Configuration for the new window.
 * @param[out] out_window Receives the pointer to the new window handle.
 */
LVKW_COLD LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                            LVKW_Window **out_window);

/**
 * @brief Destroys a window and releases OS resources.
 */
LVKW_COLD LVKW_Status lvkw_wnd_destroy(LVKW_Window *window_handle);

/**
 * @brief Updates one or more window attributes.
 * @note Non-flagged values are ignored and can be left uninitialized.
 * @param window_handle Target window.
 * @param field_mask Mask of LVKW_WindowAttributesField indicating which fields to read.
 * @param attributes Source struct containing the new values.
 */
LVKW_COLD LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask,
                                      const LVKW_WindowAttributes *attributes);

/* ----- ATTRIBUTE ASSIGNMENT HELPERS ----- */

/** @name Attribute Shorthands
 *  Non-ABI inline helpers for lvkw_wnd_update().
 *  @{ */
static LVKW_Status lvkw_wnd_setTitle(LVKW_Window *window, const char *title);
static LVKW_Status lvkw_wnd_setSize(LVKW_Window *window, LVKW_LogicalVec size);
static LVKW_Status lvkw_wnd_setFullscreen(LVKW_Window *window, bool enabled);
static LVKW_Status lvkw_wnd_setMaximized(LVKW_Window *window, bool enabled);
static LVKW_Status lvkw_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode);
static LVKW_Status lvkw_wnd_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape);
static LVKW_Status lvkw_wnd_setMonitor(LVKW_Window *window, LVKW_MonitorId monitor);
static LVKW_Status lvkw_wnd_setMinSize(LVKW_Window *window, LVKW_LogicalVec min_size);
static LVKW_Status lvkw_wnd_setMaxSize(LVKW_Window *window, LVKW_LogicalVec max_size);
static LVKW_Status lvkw_wnd_setAspectRatio(LVKW_Window *window, LVKW_Ratio aspect_ratio);
static LVKW_Status lvkw_wnd_setResizable(LVKW_Window *window, bool enabled);
static LVKW_Status lvkw_wnd_setDecorated(LVKW_Window *window, bool enabled);
/** @} */

/**
 * @brief Creates a Vulkan surface for a window.
 * @note **Precondition:** The window MUST have @ref LVKW_WND_STATE_READY set in its flags.
 * @note **Lifetime:** The caller is responsible for calling vkDestroySurfaceKHR().
 */
LVKW_COLD LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                               VkSurfaceKHR *out_surface);

/**
 * @brief Retrieves current window dimensions.
 */
LVKW_COLD LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry);

/**
 * @brief Returns the parent context.
 */
LVKW_COLD LVKW_Status lvkw_wnd_getContext(LVKW_Window *window_handle, LVKW_Context **out_context);

/**
 * @brief Requests the OS to transfer input focus to this window.
 */
LVKW_COLD LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle);

/**
 * @brief Sets the system clipboard content.
 * @param window Target window (ownership requirement for some backends).
 * @param text Null-terminated UTF-8 string.
 */
LVKW_COLD LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text);

/**
 * @brief Retrieves the current system clipboard content.
 * @note **Lifetime:** The returned string is managed by the library. It remains valid until the next
 * call to @ref lvkw_wnd_getClipboardText on the same context or until context destruction.
 * @param window Requesting window.
 * @param[out] out_text Receives the pointer to the UTF-8 text.
 */
LVKW_COLD LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text);

#ifdef __cplusplus
}
#endif

#include "lvkw/details/lvkw_attribute_wnd_shorthand_impls.h"

#endif  // LVKW_WINDOW_H_INCLUDED
