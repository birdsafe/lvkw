#ifndef LVKW_CHECKED_H_INCLUDED
#define LVKW_CHECKED_H_INCLUDED

#include "lvkw/details/lvkw_api_constraints.h"
#include "lvkw/lvkw.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Context Management --- */

static inline LVKW_Status lvkw_chk_createContext(const LVKW_ContextCreateInfo *create_info,
                                                  LVKW_Context **out_context) {
  LVKW_Status status = _lvkw_api_constraints_createContext(create_info, out_context);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_createContext(create_info, out_context);
}

static inline void lvkw_chk_destroyContext(LVKW_Context *handle) {
  if (_lvkw_api_constraints_destroyContext(handle) != LVKW_SUCCESS) return;
  lvkw_destroyContext(handle);
}

static inline void lvkw_chk_context_getVulkanInstanceExtensions(LVKW_Context *ctx, uint32_t *count,
                                                                const char **out_extensions) {
  if (_lvkw_api_constraints_context_getVulkanInstanceExtensions(ctx, count, out_extensions) != LVKW_SUCCESS) return;
  lvkw_context_getVulkanInstanceExtensions(ctx, count, out_extensions);
}

static inline LVKW_Status lvkw_chk_context_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                              LVKW_EventCallback callback, void *userdata) {
  LVKW_Status status = _lvkw_api_constraints_context_pollEvents(ctx, event_mask, callback, userdata);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_context_pollEvents(ctx, event_mask, callback, userdata);
}

static inline LVKW_Status lvkw_chk_context_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms,
                                                              LVKW_EventType event_mask,
                                                              LVKW_EventCallback callback, void *userdata) {
  LVKW_Status status = _lvkw_api_constraints_context_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_context_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
}

static inline LVKW_Status lvkw_chk_context_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_Status status = _lvkw_api_constraints_context_setIdleTimeout(ctx, timeout_ms);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_context_setIdleTimeout(ctx, timeout_ms);
}

/* --- Window Management --- */

static inline LVKW_Status lvkw_chk_context_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                                        LVKW_Window **out_window) {
  LVKW_Status status = _lvkw_api_constraints_context_createWindow(ctx, create_info, out_window);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_context_createWindow(ctx, create_info, out_window);
}

static inline void lvkw_chk_destroyWindow(LVKW_Window *handle) {
  if (_lvkw_api_constraints_destroyWindow(handle) != LVKW_SUCCESS) return;
  lvkw_destroyWindow(handle);
}

static inline LVKW_Status lvkw_chk_window_createVkSurface(LVKW_Window *window, VkInstance instance,
                                                                VkSurfaceKHR *out_surface) {
  LVKW_Status status = _lvkw_api_constraints_window_createVkSurface(window, instance, out_surface);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_window_createVkSurface(window, instance, out_surface);
}

static inline LVKW_Status lvkw_chk_window_getFramebufferSize(LVKW_Window *window, LVKW_Size *out_size) {
  LVKW_Status status = _lvkw_api_constraints_window_getFramebufferSize(window, out_size);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_window_getFramebufferSize(window, out_size);
}

static inline LVKW_Status lvkw_chk_window_setFullscreen(LVKW_Window *window, bool enabled) {
  LVKW_Status status = _lvkw_api_constraints_window_setFullscreen(window, enabled);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_window_setFullscreen(window, enabled);
}

static inline LVKW_Status lvkw_chk_window_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  LVKW_Status status = _lvkw_api_constraints_window_setCursorMode(window, mode);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_window_setCursorMode(window, mode);
}

static inline LVKW_Status lvkw_chk_window_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  LVKW_Status status = _lvkw_api_constraints_window_setCursorShape(window, shape);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_window_setCursorShape(window, shape);
}

static inline LVKW_Status lvkw_chk_window_requestFocus(LVKW_Window *window) {
  LVKW_Status status = _lvkw_api_constraints_window_requestFocus(window);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_window_requestFocus(window);
}

#ifdef __cplusplus
}
#endif

#endif  // LVKW_CHECKED_H_INCLUDED