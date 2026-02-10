#ifndef LVKW_DETAILS_API_CONSTRAINT_H_DEFINED
#define LVKW_DETAILS_API_CONSTRAINT_H_DEFINED
/*
 * NOTE: This file MUST remain in the public headers.
 *
 * It contains all API validation logic (preconditions and argument checks).
 * These serve a dual purpose:
 *
 * 1. Runtime validation for the checked API (lvkw_checked.h).
 * 2. Debug assertions for the core library (via src/lvkw/base/lvkw_api_checks.h).
 *
 * Keeping them here ensures that both the checked API and the core library
 * share the exact same validation logic.
 */
#include "lvkw/lvkw.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifndef _LVKW_CTX_ARG_CONSTRAINT
#define _LVKW_CTX_ARG_CONSTRAINT(ctx, cond, msg)                                                   \
  if (!(cond)) {                                                                                   \
    _lvkw_reportDiagnosis((LVKW_Context *)(ctx), NULL, LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg); \
    return LVKW_ERROR;                                                                        \
  }
#define _LVKW_WND_ARG_CONSTRAINT(wnd, cond, msg)                                                         \
  if (!(cond)) {                                                                                         \
    _lvkw_reportDiagnosis(lvkw_window_getContext((LVKW_Window *)(wnd)), (LVKW_Window *)(wnd), \
                         LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg);                                          \
    return LVKW_ERROR;                                                                              \
  }
#define _LVKW_CTX_ARG_PRECONDITION(ctx, cond, msg)                                                     \
  if (!(cond)) {                                                                                       \
    _lvkw_reportDiagnosis((LVKW_Context *)(ctx), NULL, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg); \
    return LVKW_ERROR;                                                                            \
  }
#define _LVKW_WND_ARG_PRECONDITION(wnd, cond, msg)                                                       \
  if (!(cond)) {                                                                                         \
    _lvkw_reportDiagnosis(lvkw_window_getContext((LVKW_Window *)(wnd)), (LVKW_Window *)(wnd), \
                         LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg);                                      \
    return LVKW_ERROR;                                                                              \
  }
#endif
#define _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx)                                                     \
  if ((ctx) && (ctx)->is_lost) {                                                               \
    _lvkw_reportDiagnosis((LVKW_Context *)(ctx), NULL, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, "Context is lost"); \
    return LVKW_ERROR_CONTEXT_LOST;                                                            \
  }
#define _LVKW_ASSERT_WINDOW_NOT_LOST(window)                                                              \
  do {                                                                                                    \
    if ((window) && (window)->is_lost) {                                                                  \
      _lvkw_reportDiagnosis(lvkw_window_getContext((LVKW_Window *)(window)), (LVKW_Window *)(window),      \
                           LVKW_DIAGNOSIS_PRECONDITION_FAILURE, "Window is lost");                        \
      return LVKW_ERROR_WINDOW_LOST;                                                                      \
    }                                                                                                     \
    if ((window) && lvkw_window_getContext(window)->is_lost) {                                            \
      _lvkw_reportDiagnosis(lvkw_window_getContext((LVKW_Window *)(window)), (LVKW_Window *)(window),      \
                           LVKW_DIAGNOSIS_PRECONDITION_FAILURE, "Window context is lost");                \
      return LVKW_ERROR_CONTEXT_LOST;                                                                     \
    }                                                                                                     \
  } while (0)
#define _LVKW_ASSERT_WINDOW_READY(window)                                      \
  _LVKW_WND_ARG_PRECONDITION(window, !(window) || (window)->is_ready, \
                             "Window is not ready. Wait for LVKW_EVENT_TYPE_WINDOW_READY")
/* --- Context Management --- */
static inline LVKW_Status _lvkw_api_constraints_createContext(const LVKW_ContextCreateInfo *create_info,
                                                               LVKW_Context **out_context) {
  if (create_info == NULL) return LVKW_ERROR;
  if (out_context == NULL) {
    if (create_info->diagnosis_cb) {
      LVKW_DiagnosisInfo info = {
          .diagnosis = LVKW_DIAGNOSIS_INVALID_ARGUMENT,
          .message = "out_context handle must not be NULL",
          .context = NULL,
          .window = NULL,
      };
      create_info->diagnosis_cb(&info, create_info->diagnosis_userdata);
    }
    return LVKW_ERROR;
  }
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_destroyContext(LVKW_Context *handle) {
  _LVKW_CTX_ARG_CONSTRAINT(handle, handle != NULL, "Context handle must not be NULL");
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_context_getVulkanInstanceExtensions(LVKW_Context *ctx,
                                                                                    uint32_t *count,
                                                                                    const char **out_extensions) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, count != NULL, "Count pointer must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  (void)out_extensions;
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_context_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                                    LVKW_EventCallback callback, void *userdata) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  _LVKW_CTX_ARG_CONSTRAINT(ctx, callback != NULL, "callback must not be NULL");
  (void)event_mask;
  (void)userdata;
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_context_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms,
                                                                    LVKW_EventType event_mask,
                                                                    LVKW_EventCallback callback, void *userdata) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  _LVKW_CTX_ARG_CONSTRAINT(ctx, callback != NULL, "callback must not be NULL");
  (void)timeout_ms;
  (void)event_mask;
  (void)userdata;
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_context_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  (void)timeout_ms;
  return LVKW_SUCCESS;
}
/* --- Window Management --- */
static inline LVKW_Status _lvkw_api_constraints_context_createWindow(LVKW_Context *ctx,
                                                              const LVKW_WindowCreateInfo *create_info,
                                                              LVKW_Window **out_window) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, create_info != NULL, "create_info must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, out_window != NULL, "out_window must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->size.width > 0, "Window must have a non-zero size");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->size.height > 0, "Window must have a non-zero size");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_destroyWindow(LVKW_Window *handle) {
  _LVKW_WND_ARG_CONSTRAINT(handle, handle != NULL, "Window handle must not be NULL");
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_window_createVkSurface(LVKW_Window *window, VkInstance instance,
                                                                       VkSurfaceKHR *out_surface) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_WND_ARG_CONSTRAINT(window, instance != NULL, "VkInstance must not be NULL");
  _LVKW_WND_ARG_CONSTRAINT(window, out_surface != NULL, "out_surface must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_window_getFramebufferSize(LVKW_Window *window,
                                                                          LVKW_Size *out_size) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_WND_ARG_CONSTRAINT(window, out_size != NULL, "out_size must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_window_setFullscreen(LVKW_Window *window, bool enabled) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  (void)enabled;
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_window_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  (void)mode;
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_window_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  (void)shape;
  return LVKW_SUCCESS;
}
static inline LVKW_Status _lvkw_api_constraints_window_requestFocus(LVKW_Window *window) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  return LVKW_SUCCESS;
}
#ifdef __cplusplus
}
#endif
#endif