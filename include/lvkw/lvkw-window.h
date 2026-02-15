// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_WINDOW_H_INCLUDED
#define LVKW_WINDOW_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw-core.h"
#include "lvkw-cursor.h"
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
  LVKW_LogicalVec logicalSize;  ///< Scaled coordinates for UI layout.
  LVKW_PixelVec pixelSize;      ///< Raw dimensions for framebuffer/swapchain allocation.
} LVKW_WindowGeometry;

/* --- Window Management --- */

/** @brief Runtime status flags for a window. */
typedef enum LVKW_WindowFlags {
  LVKW_WND_STATE_LOST = 1 << 0,       ///< Window is dead. Operations will fail and
                                      ///< it should be destroyed.
  LVKW_WND_STATE_READY = 1 << 1,      ///< Window is fully initialized and ready for
                                      ///< rendering/surface creation.
  LVKW_WND_STATE_FOCUSED = 1 << 2,    ///< Window has keyboard/input focus.
  LVKW_WND_STATE_MAXIMIZED = 1 << 3,  ///< Window is currently maximized.
  LVKW_WND_STATE_FULLSCREEN = 1 << 4, ///< Window is currently in fullscreen mode.
} LVKW_WindowFlags;

/**
 * @brief Opaque handle representing an OS-level window.
 *
 * ### Threading At A Glance
 * - Primary-thread-only: create/destroy/update/focus/clipboard/surface APIs.
 * - Any-thread with synchronization:
 *   - @ref lvkw_wnd_getGeometry (synchronize with window/context writers).
 *   - @ref lvkw_wnd_getContext (synchronize handle lifetime with destroy).
 */
struct LVKW_Window {
  void *userdata;  ///< User-controlled pointer. You CAN override it directly.
  uint32_t flags;  ///< Bitmask of LVKW_WindowFlags. Read-only.
};

/** @brief Cursor visibility and constraint modes. */
typedef enum LVKW_CursorMode {
  LVKW_CURSOR_NORMAL = 0,  ///< Visible and free to leave the window.
  LVKW_CURSOR_HIDDEN = 1,  ///< Hidden but free to leave the window.
  LVKW_CURSOR_LOCKED = 2,  ///< Hidden and confined to the window. Delivers raw relative motion.
} LVKW_CursorMode;

/** @brief Semantic hints for OS compositor optimization (e.g., Variable Refresh
 * Rate). */
typedef enum LVKW_ContentType {
  LVKW_CONTENT_TYPE_NONE = 0,
  LVKW_CONTENT_TYPE_PHOTO = 1,
  LVKW_CONTENT_TYPE_VIDEO = 2,
  LVKW_CONTENT_TYPE_GAME = 3,
} LVKW_ContentType;

/** @brief Supported Drag and Drop actions for OS feedback. */
typedef enum LVKW_DndAction {
  LVKW_DND_ACTION_NONE = 0,  ///< Reject the drop.
  LVKW_DND_ACTION_COPY = 1,  ///< Feedback: Copy (Default).
  LVKW_DND_ACTION_MOVE = 2,  ///< Feedback: Move.
  LVKW_DND_ACTION_LINK = 3,  ///< Feedback: Link/Alias.
} LVKW_DndAction;

/** @brief Semantic hints for the Input Method Editor. */
typedef enum LVKW_TextInputType {
  LVKW_TEXT_INPUT_TYPE_NONE = 0,  ///< Disable IME. Prevents IME from intercepting keys.
  LVKW_TEXT_INPUT_TYPE_TEXT,      ///< General purpose text.
  LVKW_TEXT_INPUT_TYPE_PASSWORD,  ///< Inhibits suggestions and auto-correct.
  LVKW_TEXT_INPUT_TYPE_EMAIL,     ///< Hints for email-specific layouts.
  LVKW_TEXT_INPUT_TYPE_NUMERIC,   ///< Hints for numeric keypad.
} LVKW_TextInputType;

/** @brief Bitmask for selecting which attributes to update in
 * lvkw_wnd_update(). */
typedef enum LVKW_WindowAttributesField {
  LVKW_WND_ATTR_TITLE = 1 << 0,
  LVKW_WND_ATTR_LOGICAL_SIZE = 1 << 1,
  LVKW_WND_ATTR_FULLSCREEN = 1 << 2,
  LVKW_WND_ATTR_CURSOR_MODE = 1 << 3,
  LVKW_WND_ATTR_CURSOR = 1 << 4,
  LVKW_WND_ATTR_MONITOR = 1 << 5,
  LVKW_WND_ATTR_MAXIMIZED = 1 << 6,
  LVKW_WND_ATTR_MIN_SIZE = 1 << 7,
  LVKW_WND_ATTR_MAX_SIZE = 1 << 8,
  LVKW_WND_ATTR_ASPECT_RATIO = 1 << 9,
  LVKW_WND_ATTR_RESIZABLE = 1 << 10,
  LVKW_WND_ATTR_DECORATED = 1 << 11,
  LVKW_WND_ATTR_MOUSE_PASSTHROUGH = 1 << 12,
  LVKW_WND_ATTR_ACCEPT_DND = 1 << 13,
  LVKW_WND_ATTR_TEXT_INPUT_TYPE = 1 << 14,
  LVKW_WND_ATTR_TEXT_INPUT_RECT = 1 << 15,
} LVKW_WindowAttributesField;

/** @brief Live-updatable window properties. */
typedef struct LVKW_WindowAttributes {
  const char *title;            ///< UTF-8 window title.
  LVKW_LogicalVec logicalSize;  ///< Requested window size in logical units.
  bool fullscreen;              ///< If true, window occupies the entire monitor.
  bool maximized;               ///< If true, window is expanded to fill the workspace.
  LVKW_CursorMode cursor_mode;  ///< Visibility and lock state.
  LVKW_Cursor *cursor;          ///< Hardware cursor to use. NULL for system default.
  LVKW_Monitor *monitor;        ///< Monitor for fullscreen/maximization. Use NULL for default.
  LVKW_LogicalVec minSize;      ///< Hard lower bound for resizing. {0,0} for no limit.
  LVKW_LogicalVec maxSize;      ///< Hard upper bound for resizing. {0,0} for no limit.
  LVKW_Ratio aspect_ratio;      ///< Forced proportions. {0,0} for unconstrained.
                                ///< Wayland currently stores this value but does not
                                ///< enforce it at the compositor level.
  bool resizable;               ///< If false, the OS prevents user-initiated resizing.
  bool decorated;               ///< If false, the window has no borders or title bar.
  bool mouse_passthrough;       ///< If true, mouse events pass through the window to
                                ///< those below.
  bool accept_dnd;              ///< If true, the window acts as a drop target for files.
  LVKW_TextInputType text_input_type;  ///< Current IME mode.
  LVKW_LogicalRect text_input_rect;    ///< Bounds of the caret in logical units.
} LVKW_WindowAttributes;

/** @brief Parameters for lvkw_ctx_createWindow(). */
typedef struct LVKW_WindowCreateInfo {
  LVKW_WindowAttributes attributes;  ///< Initial runtime attributes.
  const char *app_id;                ///< Platform-specific ID for shell integration (e.g.,
                                     ///< desktop file name).
  LVKW_ContentType content_type;     ///< Hint for compositor latency/refresh optimizations.
  bool transparent;                  ///< Enable alpha blending with the desktop. Set at
                                     ///< creation only.
  void *userdata;                    ///< Initial value for LVKW_Window::userdata.
} LVKW_WindowCreateInfo;

/**
 * @brief Default initialization macro for LVKW_WindowCreateInfo.
 */
#define LVKW_WINDOW_CREATE_INFO_DEFAULT                         \
  {.attributes = {.title = "LVKW Window",                       \
                  .logicalSize = {800, 600},                    \
                  .fullscreen = false,                          \
                  .maximized = false,                           \
                  .cursor_mode = LVKW_CURSOR_NORMAL,            \
                  .cursor = NULL,                               \
                  .monitor = NULL,                              \
                  .minSize = {0, 0},                            \
                  .maxSize = {0, 0},                            \
                  .aspect_ratio = {0, 0},                       \
                  .resizable = true,                            \
                  .decorated = true,                            \
                  .mouse_passthrough = false,                   \
                  .accept_dnd = false,                          \
                  .text_input_type = LVKW_TEXT_INPUT_TYPE_NONE, \
                  .text_input_rect = {{0, 0}, {0, 0}}},         \
   .app_id = "lvkw.app",                                        \
   .content_type = LVKW_CONTENT_TYPE_NONE,                      \
   .transparent = false}

/**
 * @brief Creates a new window.
 * @note Must be called on the context's primary thread.
 * @param ctx_handle Active context.
 * @param create_info Configuration for the new window.
 * @param[out] out_window Receives the pointer to the new window handle.
 */
LVKW_COLD LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle,
                                            const LVKW_WindowCreateInfo *create_info,
                                            LVKW_Window **out_window);

/**
 * @brief Destroys a window and releases OS resources.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_wnd_destroy(LVKW_Window *window_handle);

/**
 * @brief Updates one or more window attributes.
 * @note Non-flagged values are ignored and can be left uninitialized.
 * @param window_handle Target window.
 * @param field_mask Mask of LVKW_WindowAttributesField indicating which fields
 * to read.
 * @param attributes Source struct containing the new values.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask,
                                      const LVKW_WindowAttributes *attributes);

/**
 * @brief Creates a Vulkan surface for a window.
 * @note **Precondition:** The window MUST have @ref LVKW_WND_STATE_READY set in
 * its flags.
 * @note **Lifetime:** The caller is responsible for calling
 * vkDestroySurfaceKHR().
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                               VkSurfaceKHR *out_surface);

/**
 * @brief Retrieves current window dimensions.
 * @note Threading: callable from any thread, but caller must synchronize with
 * window/context writers (at minimum @ref lvkw_wnd_update,
 * @ref lvkw_ctx_syncEvents, @ref lvkw_wnd_destroy).
 */
LVKW_COLD LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle,
                                           LVKW_WindowGeometry *out_geometry);

/**
 * @brief Returns the parent context.
 * @note Threading: callable from any thread, but handle lifetime must be
 * synchronized with @ref lvkw_wnd_destroy.
 */
LVKW_COLD LVKW_Status lvkw_wnd_getContext(LVKW_Window *window_handle, LVKW_Context **out_context);

/**
 * @brief Requests the OS to transfer input focus to this window.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle);

/** @brief Container for arbitrary clipboard data. */
typedef struct LVKW_ClipboardData {
  const char *mime_type;  ///< e.g., "text/plain", "image/png", "text/html"
  const void *data;       ///< Pointer to the raw data bytes.
  size_t size;            ///< Size of the data in bytes.
} LVKW_ClipboardData;

/**
 * @brief Sets the system clipboard content.
 * @param window Target window (ownership requirement for some backends).
 * @param text Null-terminated UTF-8 string.
 * @note On Wayland, this requires a recent valid input serial and
 * `wl_data_device_manager` support. If either precondition is not met, returns
 * @ref LVKW_ERROR with diagnostics.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text);

/**
 * @brief Retrieves the current system clipboard content.
 * @note **Lifetime:** The returned string is managed by the library. It remains
 * valid until the next call to @ref lvkw_wnd_getClipboardText or
 * @ref lvkw_wnd_getClipboardData on any window in the same context, or until
 * context destruction.
 * @note Must be called on the context's primary thread.
 * @param window Requesting window.
 * @param[out] out_text Receives the pointer to the UTF-8 text.
 */
LVKW_COLD LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text);

/**
 * @brief Sets the system clipboard with multiple data formats (MIME types).
 * @param window Target window.
 * @param data Array of clipboard data items.
 * @param count Number of items in the array.
 * @note On Wayland, this requires a recent valid input serial and
 * `wl_data_device_manager` support. If either precondition is not met, returns
 * @ref LVKW_ERROR with diagnostics.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_wnd_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data,
                                                uint32_t count);

/**
 * @brief Retrieves specific MIME type data from the clipboard.
 * @note **Lifetime:** Managed by the library. It remains valid until the next
 * call to @ref lvkw_wnd_getClipboardText or @ref lvkw_wnd_getClipboardData on
 * any window in the same context, or until context destruction.
 * @note If `mime_type` is not currently available, returns @ref LVKW_ERROR
 * (not success with empty data).
 * @note Must be called on the context's primary thread.
 * @param window Requesting window.
 * @param mime_type The desired MIME type.
 * @param[out] out_data Receives the pointer to the raw data.
 * @param[out] out_size Receives the size of the data in bytes.
 */
LVKW_COLD LVKW_Status lvkw_wnd_getClipboardData(LVKW_Window *window, const char *mime_type,
                                                const void **out_data, size_t *out_size);

/**
 * @brief Enumerates all MIME types currently available on the clipboard.
 * @note The returned array and strings are managed by LVKW and remain valid
 * until the next call to @ref lvkw_wnd_getClipboardMimeTypes on any window
 * belonging to the same context, or until the context is destroyed.
 * @param window Requesting window.
 * @param[out] out_mime_types Receives a pointer to an internal array of MIME
 * type strings. Can be NULL for count-only queries.
 * @param[out] count Receives the number of MIME types currently available.
 * @note Must be called on the context's primary thread.
 */
LVKW_COLD LVKW_Status lvkw_wnd_getClipboardMimeTypes(LVKW_Window *window,
                                                     const char ***out_mime_types, uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_WINDOW_H_INCLUDED
