#ifndef LVKW_API_CHECKS_H_INCLUDED
#define LVKW_API_CHECKS_H_INCLUDED

/*
 * This file provides internal check functions that bridge the public API
 * constraints defined in lvkw_api_constraints.h with the internal diagnosis
 * reporting and assertion system.
 */

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

#define _LVKW_CTX_ARG_CONSTRAINT(ctx, cond, msg) LVKW_CTX_ASSERT_ARG(ctx, cond, msg)
#define _LVKW_WND_ARG_CONSTRAINT(wnd, cond, msg) LVKW_WND_ASSERT_ARG(wnd, cond, msg)
#define _LVKW_CTX_ARG_PRECONDITION(ctx, cond, msg) LVKW_CTX_ASSERT_PRECONDITION(ctx, cond, msg)
#define _LVKW_WND_ARG_PRECONDITION(wnd, cond, msg) LVKW_WND_ASSERT_PRECONDITION(wnd, cond, msg)

#include "lvkw/details/lvkw_api_constraints.h"
/**/
/* --- Context Management --- */

static inline void lvkw_check_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  _lvkw_api_constraints_ctx_create(create_info, out_context);
}

static inline void lvkw_check_ctx_destroy(LVKW_Context *handle) { _lvkw_api_constraints_ctx_destroy(handle); }

static inline void lvkw_check_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count,
                                                                  const char **out_extensions) {
  _lvkw_api_constraints_ctx_getVkExtensions(ctx, count, out_extensions);
}

static inline void lvkw_check_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata) {
  _lvkw_api_constraints_ctx_pollEvents(ctx, event_mask, callback, userdata);
}

static inline void lvkw_check_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata) {
  _lvkw_api_constraints_ctx_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
}

static inline void lvkw_check_ctx_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  _lvkw_api_constraints_ctx_setIdleTimeout(ctx, timeout_ms);
}

/* --- Window Management --- */

static inline void lvkw_check_ctx_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                            LVKW_Window **out_window) {
  _lvkw_api_constraints_ctx_createWindow(ctx, create_info, out_window);
}

static inline void lvkw_check_wnd_destroy(LVKW_Window *handle) { _lvkw_api_constraints_wnd_destroy(handle); }

static inline void lvkw_check_wnd_createVkSurface(LVKW_Window *window, VkInstance instance,
                                                     VkSurfaceKHR *out_surface) {
  _lvkw_api_constraints_wnd_createVkSurface(window, instance, out_surface);
}

static inline void lvkw_check_wnd_getFramebufferSize(LVKW_Window *window, LVKW_Size *out_size) {
  _lvkw_api_constraints_wnd_getFramebufferSize(window, out_size);
}

static inline void lvkw_check_wnd_setFullscreen(LVKW_Window *window, bool enabled) {
  _lvkw_api_constraints_wnd_setFullscreen(window, enabled);
}

static inline void lvkw_check_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  _lvkw_api_constraints_wnd_setCursorMode(window, mode);
}

static inline void lvkw_check_wnd_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  _lvkw_api_constraints_wnd_setCursorShape(window, shape);
}

static inline void lvkw_check_wnd_requestFocus(LVKW_Window *window) {
  _lvkw_api_constraints_wnd_requestFocus(window);
}

#endif  // LVKW_API_CHECKS_H_INCLUDED