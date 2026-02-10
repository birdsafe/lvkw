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

static inline void lvkw_check_context_create(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  _lvkw_api_constraints_context_create(create_info, out_context);
}

static inline void lvkw_check_context_destroy(LVKW_Context *handle) { _lvkw_api_constraints_context_destroy(handle); }

static inline void lvkw_check_context_getUserData(const LVKW_Context *ctx) {
  _lvkw_api_constraints_context_getUserData(ctx);
}

static inline void lvkw_check_context_getVulkanInstanceExtensions(const LVKW_Context *ctx, uint32_t *count,
                                                                  const char **out_extensions) {
  _lvkw_api_constraints_context_getVulkanInstanceExtensions(ctx, count, out_extensions);
}

static inline void lvkw_check_context_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata) {
  _lvkw_api_constraints_context_pollEvents(ctx, event_mask, callback, userdata);
}

static inline void lvkw_check_context_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata) {
  _lvkw_api_constraints_context_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
}

static inline void lvkw_check_context_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  _lvkw_api_constraints_context_setIdleTimeout(ctx, timeout_ms);
}

/* --- Window Management --- */

static inline void lvkw_check_window_create(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                            LVKW_Window **out_window) {
  _lvkw_api_constraints_window_create(ctx, create_info, out_window);
}

static inline void lvkw_check_window_destroy(LVKW_Window *handle) { _lvkw_api_constraints_window_destroy(handle); }

static inline void lvkw_check_window_createVkSurface(const LVKW_Window *window, VkInstance instance,
                                                     VkSurfaceKHR *out_surface) {
  _lvkw_api_constraints_window_createVkSurface(window, instance, out_surface);
}

static inline void lvkw_check_window_getFramebufferSize(const LVKW_Window *window, LVKW_Size *out_size) {
  _lvkw_api_constraints_window_getFramebufferSize(window, out_size);
}

static inline void lvkw_check_window_getUserData(const LVKW_Window *window) {
  _lvkw_api_constraints_window_getUserData(window);
}

static inline void lvkw_check_window_setFullscreen(LVKW_Window *window, bool enabled) {
  _lvkw_api_constraints_window_setFullscreen(window, enabled);
}

static inline void lvkw_check_window_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  _lvkw_api_constraints_window_setCursorMode(window, mode);
}

static inline void lvkw_check_window_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  _lvkw_api_constraints_window_setCursorShape(window, shape);
}

static inline void lvkw_check_window_requestFocus(LVKW_Window *window) {
  _lvkw_api_constraints_window_requestFocus(window);
}

#endif  // LVKW_API_CHECKS_H_INCLUDED