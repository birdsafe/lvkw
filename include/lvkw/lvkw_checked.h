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
  LVKW_Status status = _lvkw_api_constraints_ctx_create(create_info, out_context);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_createContext(create_info, out_context);
}

static inline void lvkw_chk_ctx_destroy(LVKW_Context *handle) {
  if (_lvkw_api_constraints_ctx_destroy(handle) != LVKW_SUCCESS) return;
  lvkw_ctx_destroy(handle);
}

static inline void lvkw_chk_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count,
                                                                const char **out_extensions) {
  if (_lvkw_api_constraints_ctx_getVkExtensions(ctx, count, out_extensions) != LVKW_SUCCESS) return;
  lvkw_ctx_getVkExtensions(ctx, count, out_extensions);
}

static inline LVKW_Status lvkw_chk_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                              LVKW_EventCallback callback, void *userdata) {
  LVKW_Status status = _lvkw_api_constraints_ctx_pollEvents(ctx, event_mask, callback, userdata);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_pollEvents(ctx, event_mask, callback, userdata);
}

static inline LVKW_Status lvkw_chk_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms,
                                                              LVKW_EventType event_mask,
                                                              LVKW_EventCallback callback, void *userdata) {
  LVKW_Status status = _lvkw_api_constraints_ctx_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
}

static inline LVKW_Status lvkw_chk_ctx_updateAttributes(LVKW_Context *ctx, uint32_t field_mask,
                                                            const LVKW_ContextAttributes *attributes) {
  LVKW_Status status = _lvkw_api_constraints_ctx_updateAttributes(ctx, field_mask, attributes);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_updateAttributes(ctx, field_mask, attributes);
}

/* --- Window Management --- */

static inline LVKW_Status lvkw_chk_ctx_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                                        LVKW_Window **out_window) {
  LVKW_Status status = _lvkw_api_constraints_ctx_createWindow(ctx, create_info, out_window);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_ctx_createWindow(ctx, create_info, out_window);
}

static inline LVKW_Status lvkw_chk_wnd_updateAttributes(LVKW_Window *window, uint32_t field_mask,
                                                            const LVKW_WindowAttributes *attributes) {
  LVKW_Status status = _lvkw_api_constraints_wnd_updateAttributes(window, field_mask, attributes);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_updateAttributes(window, field_mask, attributes);
}

static inline void lvkw_chk_wnd_destroy(LVKW_Window *handle) {
  if (_lvkw_api_constraints_wnd_destroy(handle) != LVKW_SUCCESS) return;
  lvkw_wnd_destroy(handle);
}

static inline LVKW_Status lvkw_chk_wnd_createVkSurface(LVKW_Window *window, VkInstance instance,
                                                                VkSurfaceKHR *out_surface) {
  LVKW_Status status = _lvkw_api_constraints_wnd_createVkSurface(window, instance, out_surface);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_createVkSurface(window, instance, out_surface);
}

static inline LVKW_Status lvkw_chk_wnd_getFramebufferSize(LVKW_Window *window, LVKW_Size *out_size) {
  LVKW_Status status = _lvkw_api_constraints_wnd_getFramebufferSize(window, out_size);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_getFramebufferSize(window, out_size);
}

static inline LVKW_Status lvkw_chk_wnd_setFullscreen(LVKW_Window *window, bool enabled) {
  LVKW_Status status = _lvkw_api_constraints_wnd_setFullscreen(window, enabled);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_setFullscreen(window, enabled);
}

static inline LVKW_Status lvkw_chk_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  LVKW_Status status = _lvkw_api_constraints_wnd_setCursorMode(window, mode);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_setCursorMode(window, mode);
}

static inline LVKW_Status lvkw_chk_wnd_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  LVKW_Status status = _lvkw_api_constraints_wnd_setCursorShape(window, shape);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_setCursorShape(window, shape);
}

static inline LVKW_Status lvkw_chk_wnd_requestFocus(LVKW_Window *window) {
  LVKW_Status status = _lvkw_api_constraints_wnd_requestFocus(window);
  if (status != LVKW_SUCCESS) return status;
  return lvkw_wnd_requestFocus(window);
}

#ifdef __cplusplus
}
#endif

#endif  // LVKW_CHECKED_H_INCLUDED