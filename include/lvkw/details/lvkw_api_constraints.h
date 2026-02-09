#ifndef LVKW_DETAILS_API_CONSTRAINT_H_DEFINED
#define LVKW_DETAILS_API_CONSTRAINT_H_DEFINED

#include "lvkw/lvkw.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LVKW_CTX_ARG_CONSTRAINT

#define _LVKW_CTX_ARG_CONSTRAINT(ctx, cond, msg)                                                   \
  if (!(cond)) {                                                                                   \
    lvkw_reportDiagnosis((const LVKW_Context *)(ctx), NULL, LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg); \
    return LVKW_ERROR_NOOP;                                                                        \
  }

#define _LVKW_WND_ARG_CONSTRAINT(wnd, cond, msg)                                                         \
  if (!(cond)) {                                                                                         \
    lvkw_reportDiagnosis(lvkw_window_getContext((const LVKW_Window *)(wnd)), (const LVKW_Window *)(wnd), \
                         LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg);                                          \
    return LVKW_ERROR_NOOP;                                                                              \
  }

#define _LVKW_CTX_ARG_PRECONDITION(ctx, cond, msg)                                                     \
  if (!(cond)) {                                                                                       \
    lvkw_reportDiagnosis((const LVKW_Context *)(ctx), NULL, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg); \
    return LVKW_ERROR_NOOP;                                                                            \
  }

#define _LVKW_WND_ARG_PRECONDITION(wnd, cond, msg)                                                       \
  if (!(cond)) {                                                                                         \
    lvkw_reportDiagnosis(lvkw_window_getContext((const LVKW_Window *)(wnd)), (const LVKW_Window *)(wnd), \
                         LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg);                                      \
    return LVKW_ERROR_NOOP;                                                                              \
  }

#endif

#define _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx) \
  _LVKW_CTX_ARG_PRECONDITION(ctx, !(ctx) || !lvkw_context_isLost(ctx), "Context is lost")

#define _LVKW_ASSERT_WINDOW_NOT_LOST(window)                                                              \
  do {                                                                                                    \
    _LVKW_WND_ARG_PRECONDITION(window, !(window) || !lvkw_window_isLost(window), "Window is lost");       \
    _LVKW_WND_ARG_PRECONDITION(window, !(window) || !lvkw_context_isLost(lvkw_window_getContext(window)), \
                               "Window context is lost");                                                 \
  } while (0)

#define _LVKW_ASSERT_WINDOW_READY(window)                                      \
  _LVKW_WND_ARG_PRECONDITION(window, !(window) || lvkw_window_isReady(window), \
                             "Window is not ready. Wait for LVKW_EVENT_TYPE_WINDOW_READY")

/* --- Context Management --- */

static inline LVKW_Result _lvkw_api_constraints_context_create(const LVKW_ContextCreateInfo *create_info,
                                                               LVKW_Context **out_context) {
  if (create_info == NULL) return LVKW_ERROR_NOOP;
  if (out_context == NULL) {
    if (create_info->diagnosis_callback) {
      LVKW_DiagnosisInfo info = {
          .diagnosis = LVKW_DIAGNOSIS_INVALID_ARGUMENT,
          .message = "out_context handle must not be NULL",
          .context = NULL,
          .window = NULL,
      };
      create_info->diagnosis_callback(&info, create_info->diagnosis_user_data);
    }
    return LVKW_ERROR_NOOP;
  }

  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_context_destroy(LVKW_Context *handle) {
  _LVKW_CTX_ARG_CONSTRAINT(handle, handle != NULL, "Context handle must not be NULL");
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_context_getUserData(const LVKW_Context *ctx) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_context_getVulkanInstanceExtensions(const LVKW_Context *ctx,
                                                                                    uint32_t *count,
                                                                                    const char **out_extensions) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, count != NULL, "Count pointer must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  (void)out_extensions;
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_context_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                                   LVKW_EventCallback callback, void *userdata) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  _LVKW_CTX_ARG_CONSTRAINT(ctx, callback != NULL, "callback must not be NULL");
  (void)event_mask;
  (void)userdata;
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_context_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  (void)timeout_ms;
  return LVKW_OK;
}

/* --- Window Management --- */

static inline LVKW_Result _lvkw_api_constraints_window_create(LVKW_Context *ctx,
                                                              const LVKW_WindowCreateInfo *create_info,
                                                              LVKW_Window **out_window) {
  _LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, create_info != NULL, "create_info must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, out_window != NULL, "out_window must not be NULL");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->size.width > 0, "Window must have a nmon-zero size");
  _LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->size.height > 0, "Window must have a nmon-zero size");
  _LVKW_ASSERT_CONTEXT_NOT_LOST(ctx);
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_destroy(LVKW_Window *handle) {
  _LVKW_WND_ARG_CONSTRAINT(handle, handle != NULL, "Window handle must not be NULL");
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_createVkSurface(const LVKW_Window *window, VkInstance instance,
                                                                       VkSurfaceKHR *out_surface) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_WND_ARG_CONSTRAINT(window, instance != NULL, "VkInstance must not be NULL");
  _LVKW_WND_ARG_CONSTRAINT(window, out_surface != NULL, "out_surface must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_getFramebufferSize(const LVKW_Window *window,
                                                                          LVKW_Size *out_size) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_WND_ARG_CONSTRAINT(window, out_size != NULL, "out_size must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_getUserData(const LVKW_Window *window) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_setFullscreen(LVKW_Window *window, bool enabled) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  (void)enabled;
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  (void)mode;
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  (void)shape;
  return LVKW_OK;
}

static inline LVKW_Result _lvkw_api_constraints_window_requestFocus(LVKW_Window *window) {
  _LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  _LVKW_ASSERT_WINDOW_NOT_LOST(window);
  _LVKW_ASSERT_WINDOW_READY(window);
  return LVKW_OK;
}

#ifdef __cplusplus
}
#endif

#endif