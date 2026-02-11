#ifndef LVKW_CHECKED_H_INCLUDED
#define LVKW_CHECKED_H_INCLUDED

/**
 * NOTE ON CHECKED API:
 * 
 * This header provides a "Checked" version of the LVKW API. It is primarily 
 * intended for use in language bindings (e.g. Python, Rust) or systems that 
 * require manual runtime validation without aborting.
 * 
 * For standard C/C++ development, use the core API (lvkw.h) and build with 
 * LVKW_ENABLE_DEBUG_DIAGNOSIS for validation.
 */

#include "lvkw/details/lvkw_api_constraints.h"
#include "lvkw/lvkw.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Context Management --- */

/** @brief Creates a new LVKW context (Checked version).
 *
 * @param create_info Pointer to the structure containing creation information.
 * @param out_context Pointer to a pointer where the new context handle will be stored.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_createContext(const LVKW_ContextCreateInfo *create_info,
                                                  LVKW_Context **out_context) {
  LVKW_Status status = _lvkw_api_constraints_ctx_create(create_info, out_context);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_createContext(create_info, out_context);
}

/** @brief Destroys a context and cleans up all its resources (Checked version).
 *
 * @param handle The context handle to destroy.
 */
static inline void lvkw_chk_ctx_destroy(LVKW_Context *handle) {
  if (_lvkw_api_constraints_ctx_destroy(handle) != LVKW_SUCCESS) return;
  lvkw_ctx_destroy(handle);
}

/** @brief Returns the Vulkan instance extensions required by this context (Checked version).
 *
 * @param ctx The context handle.
 * @param count Pointer to a uint32_t that will receive the number of required extensions.
 * @return A pointer to a null-terminated array of extension names managed by the library.
 */
static inline const char *const *lvkw_chk_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count) {
  if (_lvkw_api_constraints_ctx_getVkExtensions(ctx, count) != LVKW_SUCCESS) return NULL;
  return lvkw_ctx_getVkExtensions(ctx, count);
}

/** @brief Polls for waiting events and sends them to your callback (Checked version).
 *
 * @param ctx The context handle.
 * @param event_mask A bitmask specifying which event types to poll for.
 * @param callback The callback function to receive dispatched events.
 * @param userdata User data pointer to be passed to the callback.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                              LVKW_EventCallback callback, void *userdata) {
  LVKW_Status status = _lvkw_api_constraints_ctx_pollEvents(ctx, event_mask, callback, userdata);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_pollEvents(ctx, event_mask, callback, userdata);
}

/** @brief Blocks until events arrive or a timeout expires, then dispatches them (Checked version).
 *
 * @param ctx The context handle.
 * @param timeout_ms The timeout in milliseconds. Use LVKW_IDLE_NEVER to wait indefinitely.
 * @param event_mask A bitmask specifying which event types to poll for.
 * @param callback The callback function to receive dispatched events.
 * @param userdata User data pointer to be passed to the callback.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms,
                                                              LVKW_EventType event_mask,
                                                              LVKW_EventCallback callback, void *userdata) {
  LVKW_Status status = _lvkw_api_constraints_ctx_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
}

/** @brief Updates specific attributes of an existing context (Checked version).
 *
 * @param ctx The context handle.
 * @param field_mask A bitmask of LVKW_ContextAttributesField specifying which fields to update.
 * @param attributes Pointer to the structure containing the new values.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_update(LVKW_Context *ctx, uint32_t field_mask,
                                                            const LVKW_ContextAttributes *attributes) {
  LVKW_Status status = _lvkw_api_constraints_ctx_update(ctx, field_mask, attributes);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_update(ctx, field_mask, attributes);
}

/** @brief Helper to update the idle timeout of a context (Checked version).
 *
 * @param ctx The context handle.
 * @param timeout_ms The new idle timeout in milliseconds.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_ContextAttributes attrs = {0};
  attrs.idle_timeout_ms = timeout_ms;
  return lvkw_chk_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs);
}

/** @brief Helper to toggle idle inhibition of a context (Checked version).
 *
 * @param ctx The context handle.
 * @param enabled True to prevent the system from going idle, false otherwise.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_setIdleInhibition(LVKW_Context *ctx, bool enabled) {
  LVKW_ContextAttributes attrs = {0};
  attrs.inhibit_idle = enabled;
  return lvkw_chk_ctx_update(ctx, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs);
}

/** @brief Helper to set the diagnosis callback of a context (Checked version).
 *
 * @param ctx The context handle.
 * @param callback The new diagnosis callback.
 * @param userdata User data for the callback.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_setDiagnosisCallback(LVKW_Context *ctx, LVKW_DiagnosisCallback callback,
                                                            void *userdata) {
  LVKW_ContextAttributes attrs = {0};
  attrs.diagnosis_cb = callback;
  attrs.diagnosis_userdata = userdata;
  return lvkw_chk_ctx_update(ctx, LVKW_CTX_ATTR_DIAGNOSIS, &attrs);
}

/* --- Monitor Management --- */

/** @brief Enumerates available monitors (Checked version).
 *
 * @param ctx The context handle.
 * @param out_monitors Array to fill, or NULL to query count only.
 * @param count Pointer to capacity (in) / actual count (out).
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_getMonitors(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors,
                                                    uint32_t *count) {
  LVKW_Status status = _lvkw_api_constraints_ctx_getMonitors(ctx, out_monitors, count);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_getMonitors(ctx, out_monitors, count);
}

/** @brief Enumerates video modes for a specific monitor (Checked version).
 *
 * @param ctx The context handle.
 * @param monitor The monitor to query.
 * @param out_modes Array to fill, or NULL to query count only.
 * @param count Pointer to capacity (in) / actual count (out).
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_getMonitorModes(LVKW_Context *ctx, LVKW_MonitorId monitor,
                                                        LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_Status status = _lvkw_api_constraints_ctx_getMonitorModes(ctx, monitor, out_modes, count);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_getMonitorModes(ctx, monitor, out_modes, count);
}

/* --- Window Management --- */

/** @brief Creates a new window instance within the given context (Checked version).
 *
 * @param ctx The context handle.
 * @param create_info Pointer to the structure containing window creation information.
 * @param out_window Pointer to a pointer where the new window handle will be stored.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_ctx_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                                        LVKW_Window **out_window) {
  LVKW_Status status = _lvkw_api_constraints_ctx_createWindow(ctx, create_info, out_window);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_createWindow(ctx, create_info, out_window);
}

/** @brief Updates specific attributes of an existing window (Checked version).
 *
 * @param window The window handle.
 * @param field_mask A bitmask of LVKW_WindowAttributesField specifying which fields to update.
 * @param attributes Pointer to the structure containing the new values.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_update(LVKW_Window *window, uint32_t field_mask,
                                                            const LVKW_WindowAttributes *attributes) {
  LVKW_Status status = _lvkw_api_constraints_wnd_update(window, field_mask, attributes);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_update(window, field_mask, attributes);
}

/** @brief Helper to update the title of a window (Checked version).
 *
 * @param window The window handle.
 * @param title The new title (UTF-8).
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_setTitle(LVKW_Window *window, const char *title) {
  LVKW_WindowAttributes attrs = {0};
  attrs.title = title;
  return lvkw_chk_wnd_update(window, LVKW_WND_ATTR_TITLE, &attrs);
}

/** @brief Helper to update the logical size of a window (Checked version).
 *
 * @param window The window handle.
 * @param size The new logical size.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_setSize(LVKW_Window *window, LVKW_Size size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.logicalSize = size;
  return lvkw_chk_wnd_update(window, LVKW_WND_ATTR_LOGICAL_SIZE, &attrs);
}

/** @brief Helper to toggle fullscreen mode of a window (Checked version).
 *
 * @param window The window handle.
 * @param enabled True to enable fullscreen, false for windowed.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_setFullscreen(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.fullscreen = enabled;
  return lvkw_chk_wnd_update(window, LVKW_WND_ATTR_FULLSCREEN, &attrs);
}

/** @brief Helper to update the cursor mode of a window (Checked version).
 *
 * @param window The window handle.
 * @param mode The new cursor mode.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_mode = mode;
  return lvkw_chk_wnd_update(window, LVKW_WND_ATTR_CURSOR_MODE, &attrs);
}

/** @brief Helper to update the cursor shape of a window (Checked version).
 *
 * @param window The window handle.
 * @param shape The new cursor shape.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_shape = shape;
  return lvkw_chk_wnd_update(window, LVKW_WND_ATTR_CURSOR_SHAPE, &attrs);
}

/** @brief Destroys a window and cleans up its resources (Checked version).
 *
 * @param handle The window handle to destroy.
 */
static inline void lvkw_chk_wnd_destroy(LVKW_Window *handle) {
  if (_lvkw_api_constraints_wnd_destroy(handle) != LVKW_SUCCESS) return;
  lvkw_wnd_destroy(handle);
}

/** @brief Creates a Vulkan surface for a window (Checked version).
 *
 * @param window The window handle.
 * @param instance The Vulkan instance.
 * @param out_surface Pointer to a VkSurfaceKHR that will receive the created surface.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_createVkSurface(LVKW_Window *window, VkInstance instance,
                                                                VkSurfaceKHR *out_surface) {
  LVKW_Status status = _lvkw_api_constraints_wnd_createVkSurface(window, instance, out_surface);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_createVkSurface(window, instance, out_surface);
}

/** @brief Gets the current geometry (logical and physical size) of a window (Checked version).
 *
 * @param window The window handle.
 * @param out_geometry Pointer to a LVKW_WindowGeometry structure.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_getGeometry(LVKW_Window *window, LVKW_WindowGeometry *out_geometry) {
  LVKW_Status status = _lvkw_api_constraints_wnd_getGeometry(window, out_geometry);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_getGeometry(window, out_geometry);
}

/** @brief Asks the OS to give this window input focus (Checked version).
 *
 * @param window The window handle.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_requestFocus(LVKW_Window *window) {
  LVKW_Status status = _lvkw_api_constraints_wnd_requestFocus(window);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_requestFocus(window);
}

/** @brief Sets the system clipboard content to a UTF-8 string (Checked version).
 *
 * @param window The window that will own the clipboard selection.
 * @param text The null-terminated UTF-8 string to copy.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_Status status = _lvkw_api_constraints_wnd_setClipboardText(window, text);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_setClipboardText(window, text);
}

/** @brief Retrieves the current system clipboard content as a UTF-8 string (Checked version).
 *
 * @param window The window used to request the clipboard content.
 * @param out_text Pointer to a const char* that will receive the address of the text.
 * @return LVKW_SUCCESS on success, or LVKW_ERROR on failure.
 */
static inline LVKW_Status lvkw_chk_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_Status status = _lvkw_api_constraints_wnd_getClipboardText(window, out_text);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_getClipboardText(window, out_text);
}

#ifdef __cplusplus
}
#endif

#endif  // LVKW_CHECKED_H_INCLUDED