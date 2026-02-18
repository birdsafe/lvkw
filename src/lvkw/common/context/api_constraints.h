// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_DETAILS_API_CONSTRAINT_H_DEFINED
#define LVKW_DETAILS_API_CONSTRAINT_H_DEFINED

#include <stdlib.h>

#include "diagnostic_internal.h"
#include "types_internal.h"

#ifdef LVKW_VALIDATE_API_CALLS

#ifdef LVKW_RECOVERABLE_API_CALLS
#define LVKW_CONSTRAINT_CTX_CHECK(ctx_base, cond, diagnostic, msg)                   \
  do {                                                                               \
    if (!(cond)) {                                                                   \
      _lvkw_reportDiagnostic((LVKW_Context *)(ctx_base), NULL, (diagnostic), (msg)); \
      return LVKW_ERROR_INVALID_USAGE;                                               \
    }                                                                                \
  } while (0)

#define LVKW_CONSTRAINT_WND_CHECK(window_base, cond, diagnostic, msg)           \
  do {                                                                          \
    if (!(cond)) {                                                              \
      LVKW_REPORT_WIND_DIAGNOSTIC((window_base), (diagnostic), (msg));         \
      return LVKW_ERROR_INVALID_USAGE;                                          \
    }                                                                           \
  } while (0)

#define LVKW_BOOTSTRAP_CHECK(create_info, cond, diagnostic, msg)      \
  do {                                                                \
    if (!(cond)) {                                                    \
      LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, diagnostic, msg); \
      return LVKW_ERROR_INVALID_USAGE;                                \
    }                                                                 \
  } while (0)

#define LVKW_API_VALIDATE(fn, ...)                             \
  do {                                                         \
    LVKW_Status res = _lvkw_api_constraints_##fn(__VA_ARGS__); \
    if (res != LVKW_SUCCESS) return res;                       \
  } while (0)
#else
#define LVKW_CONSTRAINT_CTX_CHECK(ctx_base, cond, diagnostic, msg)                   \
  do {                                                                               \
    if (!(cond)) {                                                                   \
      _lvkw_reportDiagnostic((LVKW_Context *)(ctx_base), NULL, (diagnostic), (msg)); \
      abort();                                                                       \
    }                                                                                \
  } while (0)

#define LVKW_CONSTRAINT_WND_CHECK(window_base, cond, diagnostic, msg) \
  do {                                                                \
    if (!(cond)) {                                                    \
      LVKW_REPORT_WIND_DIAGNOSTIC(window_base, diagnostic, msg);      \
      abort();                                                        \
    }                                                                 \
  } while (0)

// Bootstrap is a special condition where we dojn''t have a context yet, so we report diagnostics
// through the create_info callback instead.
#define LVKW_BOOTSTRAP_CHECK(create_info, cond, diagnostic, msg)      \
  do {                                                                \
    if (!(cond)) {                                                    \
      LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, diagnostic, msg); \
      abort();                                                        \
    }                                                                 \
  } while (0)

#define LVKW_API_VALIDATE(fn, ...)                             \
  do {                                                         \
    LVKW_Status res = _lvkw_api_constraints_##fn(__VA_ARGS__); \
    if (res != LVKW_SUCCESS) abort();                          \
  } while (0)

#endif
#else
#define LVKW_CONSTRAINT_CTX_CHECK(ctx_base, cond, diagnostic, msg) (void)0
#define LVKW_CONSTRAINT_WND_CHECK(window_base, cond, diagnostic, msg) (void)0
#define LVKW_BOOTSTRAP_CHECK(create_info, cond, diagnostic, msg) (void)0

#define LVKW_API_VALIDATE(fn, ...) (void)0
#endif

#define LVKW_CONTEXT_ARG_CONSTRAINT(ctx, cond, msg) \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, cond, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, msg)

#define LVKW_WINDOW_ARG_CONSTRAINT(wnd, cond, msg) \
  LVKW_CONSTRAINT_WND_CHECK(wnd, cond, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, msg)

#if LVKW_API_VALIDATION > 0
#define LVKW_CONSTRAINT_CTX_AFFINITY(ctx)                                             \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, (ctx)->prv.creator_thread == _lvkw_get_current_thread_id(), \
                            LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,                            \
                            "API called from wrong thread.")

#define LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(ctx)                                             \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, (ctx)->prv.creator_thread == _lvkw_get_current_thread_id(), \
                            LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,                            \
                            "This API function MUST be called from "                         \
                            "the creator thread.")
#else
#define LVKW_CONSTRAINT_CTX_AFFINITY(ctx) (void)0
#define LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(ctx) (void)0
#endif

/* Base context validity checks that do not imply any thread policy. */
#define LVKW_CONSTRAINT_CTX_VALID(ctx)                                          \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, ctx != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, \
                            "Context handle must not be NULL");                 \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, !((ctx)->pub.flags & LVKW_CONTEXT_STATE_LOST),     \
                            LVKW_DIAGNOSTIC_PRECONDITION_FAILURE, "Context is lost")

/* Context health including default affinity policy. */
#define LVKW_CONSTRAINT_CTX_HEALTHY(ctx) \
  LVKW_CONSTRAINT_CTX_VALID(ctx);        \
  LVKW_CONSTRAINT_CTX_AFFINITY(ctx)

#define LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(ctx) LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(ctx)
#define LVKW_CONSTRAINT_CTX_THREAD_AFFINE(ctx) LVKW_CONSTRAINT_CTX_AFFINITY(ctx)
#define LVKW_CONSTRAINT_CTX_THREAD_ANY(ctx) (void)0

#define LVKW_CONSTRAINT_WND_VALID_AND_READY(wnd)                                                  \
  LVKW_CONSTRAINT_WND_CHECK(wnd, wnd != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,                   \
                            "Window handle must not be NULL");                                    \
  LVKW_CONSTRAINT_CTX_VALID(                                                                      \
      (LVKW_Context_Base *)(((const LVKW_Window_Base *)(wnd))->prv.ctx_base));                    \
  LVKW_CONSTRAINT_WND_CHECK(wnd, !(((LVKW_Window_Base *)(wnd))->pub.flags & LVKW_WINDOW_STATE_LOST), \
                            LVKW_DIAGNOSTIC_PRECONDITION_FAILURE, "Window is lost");              \
  LVKW_CONSTRAINT_WND_CHECK(wnd, ((LVKW_Window_Base *)(wnd))->pub.flags &LVKW_WINDOW_STATE_READY,    \
                            LVKW_DIAGNOSTIC_PRECONDITION_FAILURE, "Window is not ready")

/* --- Context Management --- */

static inline LVKW_Status _lvkw_api_constraints_createContext(
    const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  LVKW_BOOTSTRAP_CHECK(create_info, create_info != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                       "create_info must not be NULL");
  LVKW_BOOTSTRAP_CHECK(create_info, out_context != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                       "out_context handle must not be NULL");

  if (create_info->tuning) {
    // Tuning validation
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *ctx, LVKW_MetricsCategory category,
                                  void *out_data, bool reset);

static inline LVKW_Status _lvkw_api_constraints_ctx_getMetrics(LVKW_Context *ctx,
                                                                 LVKW_MetricsCategory category,
                                                                 void *out_data, bool reset) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_ANY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, category != LVKW_METRICS_CATEGORY_NONE,
                          "category must not be NONE");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, out_data != NULL, "out_data must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_destroy(LVKW_Context *handle) {
  LVKW_CONTEXT_ARG_CONSTRAINT(handle, handle != NULL, "Context handle must not be NULL");
  if (handle) {
    LVKW_CONSTRAINT_CTX_STRICT_AFFINITY((LVKW_Context_Base *)handle);
  }
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_getVkExtensions(
    LVKW_Context *ctx, uint32_t *count, const char *const **out_extensions) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_ANY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, count != NULL, "Count pointer must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, out_extensions != NULL, "out_extensions must not be NULL");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_pumpEvents(LVKW_Context *ctx,
                                                               uint32_t timeout_ms) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY((LVKW_Context_Base *)ctx);

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_postEvent(LVKW_Context *ctx,
                                                              LVKW_EventType type,
                                                              LVKW_Window *window,
                                                              const LVKW_Event *evt) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_ANY((LVKW_Context_Base *)ctx);
  // postEvent is explicitly documented as safe to call from any thread
  
  uint32_t u_type = (uint32_t)type;
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, (u_type >= (uint32_t)LVKW_EVENT_TYPE_USER_0 && 
                                u_type <= (uint32_t)LVKW_EVENT_TYPE_USER_3),
                          "postEvent only supports USER_n event types");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_update(
    LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, attributes != NULL, "attributes must not be NULL");

  return LVKW_SUCCESS;
}

/* --- Monitor Management --- */

static inline LVKW_Status _lvkw_api_constraints_ctx_getMonitors(LVKW_Context *ctx,
                                                                LVKW_MonitorRef **out_refs,
                                                                uint32_t *count) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_ANY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, count != NULL, "count must not be NULL");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_getMonitorModes(LVKW_Context *ctx,
                                                                    const LVKW_Monitor *monitor,
                                                                    LVKW_VideoMode *out_modes,
                                                                    uint32_t *count) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_ANY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, count != NULL, "count must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, monitor != NULL, "Monitor handle must not be NULL");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_monitor_createRef(LVKW_MonitorRef *monitor_ref,
                                                                  LVKW_Monitor **out_monitor) {
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, monitor_ref != NULL,
                            LVKW_DIAGNOSTIC_INVALID_ARGUMENT, "Monitor ref must not be NULL");
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, out_monitor != NULL,
                            LVKW_DIAGNOSTIC_INVALID_ARGUMENT, "out_monitor must not be NULL");
  LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(((LVKW_Monitor_Base *)monitor_ref)->prv.ctx_base);
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_monitor_destroy(LVKW_Monitor *monitor) {
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, monitor != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                            "Monitor handle must not be NULL");
  LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(((LVKW_Monitor_Base *)monitor)->prv.ctx_base);
  LVKW_CONSTRAINT_CTX_CHECK(((LVKW_Monitor_Base *)monitor)->prv.ctx_base,
                            ((LVKW_Monitor_Base *)monitor)->prv.user_refcount > 0,
                            LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                            "Monitor handle was already destroyed");
  return LVKW_SUCCESS;
}

/* --- Window Management --- */

static inline LVKW_Status _lvkw_api_constraints_ctx_createWindow(
    LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info, LVKW_Window **out_window) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info != NULL, "create_info must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, out_window != NULL, "out_window must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->attributes.logical_size.x > 0,
                          "Window must have a non-zero size");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->attributes.logical_size.y > 0,
                          "Window must have a non-zero size");

  if (create_info->attributes.min_size.x != (LVKW_Scalar)0.0 || create_info->attributes.min_size.y != (LVKW_Scalar)0.0) {
    LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->attributes.min_size.x >= 0,
                            "min_size.x must be non-negative");
    LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->attributes.min_size.y >= 0,
                            "min_size.y must be non-negative");
  }

  if (create_info->attributes.max_size.x != (LVKW_Scalar)0.0 || create_info->attributes.max_size.y != (LVKW_Scalar)0.0) {
    LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->attributes.max_size.x >= 0,
                            "max_size.x must be non-negative");
    LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->attributes.max_size.y >= 0,
                            "max_size.y must be non-negative");

    if (create_info->attributes.min_size.x != (LVKW_Scalar)0.0 && create_info->attributes.max_size.x != (LVKW_Scalar)0.0) {
      LVKW_CONTEXT_ARG_CONSTRAINT(
          ctx, create_info->attributes.min_size.x <= create_info->attributes.max_size.x,
          "min_size.x must be <= max_size.x");
    }
    if (create_info->attributes.min_size.y != (LVKW_Scalar)0.0 && create_info->attributes.max_size.y != (LVKW_Scalar)0.0) {
      LVKW_CONTEXT_ARG_CONSTRAINT(
          ctx, create_info->attributes.min_size.y <= create_info->attributes.max_size.y,
          "min_size.y must be <= max_size.y");
    }
  }

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_update(
    LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY((LVKW_Window_Base *)window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  LVKW_WINDOW_ARG_CONSTRAINT(window, attributes != NULL, "attributes must not be NULL");

  if (attributes->min_size.x != (LVKW_Scalar)0.0 || attributes->min_size.y != (LVKW_Scalar)0.0) {
    LVKW_WINDOW_ARG_CONSTRAINT(window, attributes->min_size.x >= 0, "min_size.x must be non-negative");
    LVKW_WINDOW_ARG_CONSTRAINT(window, attributes->min_size.y >= 0, "min_size.y must be non-negative");
  }

  if (attributes->max_size.x != (LVKW_Scalar)0.0 || attributes->max_size.y != (LVKW_Scalar)0.0) {
    LVKW_WINDOW_ARG_CONSTRAINT(window, attributes->max_size.x >= 0, "max_size.x must be non-negative");
    LVKW_WINDOW_ARG_CONSTRAINT(window, attributes->max_size.y >= 0, "max_size.y must be non-negative");

    if (attributes->min_size.x != (LVKW_Scalar)0.0 && attributes->max_size.x != (LVKW_Scalar)0.0) {
      LVKW_WINDOW_ARG_CONSTRAINT(window, attributes->min_size.x <= attributes->max_size.x,
                              "min_size.x must be <= max_size.x");
    }
    if (attributes->min_size.y != (LVKW_Scalar)0.0 && attributes->max_size.y != (LVKW_Scalar)0.0) {
      LVKW_WINDOW_ARG_CONSTRAINT(window, attributes->min_size.y <= attributes->max_size.y,
                              "min_size.y must be <= max_size.y");
    }
  }

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_destroy(LVKW_Window *handle) {
  // N.B. Window does not need to be healthy to be destroyed, but it needs to exist.
  LVKW_WINDOW_ARG_CONSTRAINT(handle, handle != NULL, "Window handle must not be NULL");
  if (handle) {
    LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(((const LVKW_Window_Base *)handle)->prv.ctx_base);
  }
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_createVkSurface(LVKW_Window *window,
                                                                    VkInstance instance,
                                                                    VkSurfaceKHR *out_surface) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, instance != NULL, "VkInstance must not be NULL");
  LVKW_WINDOW_ARG_CONSTRAINT(window, out_surface != NULL, "out_surface must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getGeometry(LVKW_Window *window,
                                                                LVKW_WindowGeometry *out_geometry) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_ANY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, out_geometry != NULL, "out_geometry must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getContext(LVKW_Window *window,
                                                               LVKW_Context **out_context) {
  // Window does not need to be healty or ready to get its context, but it needs to exist.
  LVKW_WINDOW_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  LVKW_WINDOW_ARG_CONSTRAINT(window, out_context != NULL, "out_context must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_requestFocus(LVKW_Window *window) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_data_getClipboardText(LVKW_Window *window, const char **out_text);
LVKW_Status lvkw_data_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data,
                                      uint32_t count);
LVKW_Status lvkw_data_getClipboardData(LVKW_Window *window, const char *mime_type,
                                      const void **out_data, size_t *out_size);
LVKW_Status lvkw_data_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types,
                                           uint32_t *count);
LVKW_Status lvkw_data_pushText(LVKW_Window *window, LVKW_DataExchangeTarget target, const char *text);
LVKW_Status lvkw_data_pullText(LVKW_Window *window, LVKW_DataExchangeTarget target, const char **out_text);
LVKW_Status lvkw_data_pushData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const LVKW_DataBuffer *data, uint32_t count);
LVKW_Status lvkw_data_pullData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *mime_type, const void **out_data, size_t *out_size);
LVKW_Status lvkw_data_listBufferMimeTypes(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                          const char ***out_mime_types, uint32_t *count);
static inline LVKW_Status _lvkw_api_constraints_wnd_setClipboardText(LVKW_Window *window,
                                                                     const char *text);
static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardText(LVKW_Window *window,
                                                                     const char **out_text);
static inline LVKW_Status _lvkw_api_constraints_wnd_setClipboardData(
    LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count);
static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardData(
    LVKW_Window *window, const char *mime_type, const void **out_data, size_t *out_size);
static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardMimeTypes(
    LVKW_Window *window, const char ***out_mime_types, uint32_t *count);

static inline LVKW_Status _lvkw_api_constraints_data_target(LVKW_Window *window,
                                                            LVKW_DataExchangeTarget target) {
  LVKW_WINDOW_ARG_CONSTRAINT(window,
                             target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD ||
                                 target == LVKW_DATA_EXCHANGE_TARGET_PRIMARY,
                             "target is out of range");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_data_pushText(LVKW_Window *window,
                                                              LVKW_DataExchangeTarget target,
                                                              const char *text) {
  LVKW_Status status = _lvkw_api_constraints_wnd_setClipboardText(window, text);
  if (status != LVKW_SUCCESS) return status;
  return _lvkw_api_constraints_data_target(window, target);
}

static inline LVKW_Status _lvkw_api_constraints_data_pullText(LVKW_Window *window,
                                                              LVKW_DataExchangeTarget target,
                                                              const char **out_text) {
  LVKW_Status status = _lvkw_api_constraints_wnd_getClipboardText(window, out_text);
  if (status != LVKW_SUCCESS) return status;
  return _lvkw_api_constraints_data_target(window, target);
}

static inline LVKW_Status _lvkw_api_constraints_data_pushData(LVKW_Window *window,
                                                              LVKW_DataExchangeTarget target,
                                                              const LVKW_DataBuffer *data,
                                                              uint32_t count) {
  LVKW_Status status =
      _lvkw_api_constraints_wnd_setClipboardData(window, (const LVKW_ClipboardData *)data, count);
  if (status != LVKW_SUCCESS) return status;
  return _lvkw_api_constraints_data_target(window, target);
}

static inline LVKW_Status _lvkw_api_constraints_data_pullData(LVKW_Window *window,
                                                              LVKW_DataExchangeTarget target,
                                                              const char *mime_type,
                                                              const void **out_data,
                                                              size_t *out_size) {
  LVKW_Status status =
      _lvkw_api_constraints_wnd_getClipboardData(window, mime_type, out_data, out_size);
  if (status != LVKW_SUCCESS) return status;
  return _lvkw_api_constraints_data_target(window, target);
}

static inline LVKW_Status _lvkw_api_constraints_data_listBufferMimeTypes(
    LVKW_Window *window, LVKW_DataExchangeTarget target, const char ***out_mime_types,
    uint32_t *count) {
  LVKW_Status status =
      _lvkw_api_constraints_wnd_getClipboardMimeTypes(window, out_mime_types, count);
  if (status != LVKW_SUCCESS) return status;
  return _lvkw_api_constraints_data_target(window, target);
}

static inline LVKW_Status _lvkw_api_constraints_wnd_setClipboardText(LVKW_Window *window,
                                                                     const char *text) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, text != NULL, "text must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardText(LVKW_Window *window,
                                                                     const char **out_text) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, out_text != NULL, "out_text must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_setClipboardData(LVKW_Window *window,
                                                                     const LVKW_ClipboardData *data,
                                                                     uint32_t count) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, data != NULL || count == 0, "data must not be NULL if count > 0");
  for (uint32_t i = 0; i < count; ++i) {
    LVKW_WINDOW_ARG_CONSTRAINT(window, data[i].mime_type != NULL, "mime_type must not be NULL");
    LVKW_WINDOW_ARG_CONSTRAINT(window, data[i].data != NULL || data[i].size == 0,
                            "data must not be NULL if size > 0");
  }
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardData(LVKW_Window *window,
                                                                     const char *mime_type,
                                                                     const void **out_data,
                                                                     size_t *out_size) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, mime_type != NULL, "mime_type must not be NULL");
  LVKW_WINDOW_ARG_CONSTRAINT(window, out_data != NULL, "out_data must not be NULL");
  LVKW_WINDOW_ARG_CONSTRAINT(window, out_size != NULL, "out_size must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardMimeTypes(
    LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  LVKW_CONSTRAINT_WND_VALID_AND_READY(window);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Window_Base *)window)->prv.ctx_base);
  LVKW_WINDOW_ARG_CONSTRAINT(window, count != NULL, "count must not be NULL");
  return LVKW_SUCCESS;
}

/* --- Cursor Management --- */

static inline LVKW_Status _lvkw_api_constraints_ctx_getStandardCursor(LVKW_Context *ctx,
                                                                       LVKW_CursorShape shape,
                                                                       LVKW_Cursor **out_cursor) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_ANY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, out_cursor != NULL, "out_cursor must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, (uint32_t)shape >= (uint32_t)LVKW_CURSOR_SHAPE_DEFAULT &&
                                   (uint32_t)shape <= (uint32_t)LVKW_CURSOR_SHAPE_NWSE_RESIZE,
                          "shape is out of range");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_createCursor(
    LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info, LVKW_Cursor **out_cursor) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info != NULL, "create_info must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, out_cursor != NULL, "out_cursor must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->size.x > 0, "Cursor must have a non-zero size");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->size.y > 0, "Cursor must have a non-zero size");
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, create_info->pixels != NULL, "pixels must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_cursor_destroy(LVKW_Cursor *handle) {
  // Cursor does not need to be healthy, but it needs to exist.
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, handle != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                            "Cursor handle must not be NULL");
  LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(((LVKW_Cursor_Base *)(void *)handle)->prv.ctx_base);
  return LVKW_SUCCESS;
}

/* --- Controller Management --- */

#ifdef LVKW_ENABLE_CONTROLLER
static inline LVKW_Status _lvkw_api_constraints_ctrl_list(LVKW_Context *ctx,
                                                          LVKW_ControllerRef **out_refs,
                                                          uint32_t *out_count) {
  LVKW_CONSTRAINT_CTX_VALID((LVKW_Context_Base *)ctx);
  LVKW_CONSTRAINT_CTX_THREAD_ANY((LVKW_Context_Base *)ctx);
  LVKW_CONTEXT_ARG_CONSTRAINT(ctx, out_count != NULL, "out_count must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctrl_create(LVKW_ControllerRef *ref,
                                                            LVKW_Controller **out_controller) {
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, ref != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                            "Controller ref must not be NULL");
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, out_controller != NULL,
                            LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                            "out_controller must not be NULL");
  LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(((LVKW_Controller_Base *)ref)->prv.ctx_base);
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctrl_destroy(LVKW_Controller *handle) {
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, handle != NULL,
                            LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                            "Controller handle must not be NULL");
  LVKW_CONSTRAINT_CTX_STRICT_AFFINITY(((LVKW_Controller_Base *)handle)->prv.ctx_base);
  LVKW_CONSTRAINT_CTX_CHECK(((LVKW_Controller_Base *)handle)->prv.ctx_base,
                            ((LVKW_Controller_Base *)handle)->prv.user_refcount > 0,
                            LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                            "Controller handle was already destroyed");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctrl_getInfo(LVKW_Controller *controller,
                                                             LVKW_CtrlInfo *out_info) {
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, controller != NULL,
                            LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                            "Controller handle must not be NULL");
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Controller_Base *)controller)->prv.ctx_base);
  LVKW_CONTEXT_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub,
                          out_info != NULL, "out_info must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctrl_setHapticLevels(
    LVKW_Controller *controller, uint32_t first_haptic, uint32_t count,
    const LVKW_Scalar *intensities) {
  LVKW_CONSTRAINT_CTX_CHECK((LVKW_Context_Base *)NULL, controller != NULL,
                            LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                            "Controller handle must not be NULL");
  LVKW_CONSTRAINT_CTX_VALID(((LVKW_Controller_Base *)controller)->prv.ctx_base);
  LVKW_CONSTRAINT_CTX_THREAD_PRIMARY(((LVKW_Controller_Base *)controller)->prv.ctx_base);
  LVKW_CONTEXT_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub,
                          intensities != NULL, "intensities array must not be NULL");
  LVKW_CONTEXT_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub,
                          first_haptic + count <= controller->haptic_count,
                          "Haptic channel index out of range");

  for (uint32_t i = 0; i < count; ++i) {
    LVKW_CONTEXT_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub,
                            intensities[i] >= (LVKW_Scalar)0.0 && intensities[i] <= (LVKW_Scalar)1.0,
                            "Intensity must be between 0.0 and 1.0");
  }

  return LVKW_SUCCESS;
}
#endif

#endif  // LVKW_DETAILS_API_CONSTRAINT_H_DEFINED
