#ifndef LVKW_DETAILS_API_CONSTRAINT_H_DEFINED
#define LVKW_DETAILS_API_CONSTRAINT_H_DEFINED

#include <stdlib.h>

#include "lvkw_diagnostic_internal.h"
#include "lvkw_types_internal.h"

#ifdef LVKW_VALIDATE_API_CALLS

#ifdef LVKW_RECOVERABLE_API_CALLS
#define LVKW_CONSTRAINT_CTX_CHECK                                                    \
  (ctx_base, cond, diagnostic, msg) do {                                             \
    if (!(cond)) {                                                                   \
      _lvkw_reportDiagnostic((LVKW_Context *)(ctx_base), NULL, (diagnostic), (msg)); \
      return LVKW_ERROR_INVALID_USAGE;                                               \
    }                                                                                \
  }                                                                                  \
  while (0)

#define LVKW_BOOTSTRAP_CHECK(create_info, cond, diagnostic, msg)      \
  do {                                                                \
    if (!(cond)) {                                                    \
      LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, diagnostic, msg); \
      return LVKW_ERROR_INVALID_ARGUMENT;                             \
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

// Bootstrap is a special condition where we dojn''t have a context yet, so we report diagnostics through the
// create_info callback instead.
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

#define LVKW_CTX_ARG_CONSTRAINT(ctx, cond, msg) \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, cond, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, msg)

#define LVKW_WND_ARG_CONSTRAINT(wnd, cond, msg) \
  LVKW_CONSTRAINT_WND_CHECK(wnd, cond, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, msg)

#define LVKW_CONSTRAINT_CTX_HEALTHY(ctx)                                                                            \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, ctx != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, "Context handle must not be NULL"); \
  LVKW_CONSTRAINT_CTX_CHECK(ctx, !((ctx)->flags & LVKW_CTX_STATE_LOST), LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,       \
                            "Context is lost")

#define LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(wnd)                                                                 \
  LVKW_CONSTRAINT_WND_CHECK(wnd, wnd != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, "Window handle must not be NULL"); \
  LVKW_CONSTRAINT_CTX_HEALTHY(&((const LVKW_Window_Base *)(wnd))->prv.ctx_base->pub);                              \
  LVKW_CONSTRAINT_WND_CHECK(wnd, !((wnd)->flags & LVKW_WND_STATE_LOST), LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,      \
                            "Window is lost");                                                                     \
  LVKW_CONSTRAINT_WND_CHECK(wnd, (wnd)->flags &LVKW_WND_STATE_READY, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,         \
                            "Window is not ready")

/* --- Context Management --- */

static inline LVKW_Status _lvkw_api_constraints_createContext(const LVKW_ContextCreateInfo *create_info,
                                                              LVKW_Context **out_context) {
  LVKW_BOOTSTRAP_CHECK(create_info, create_info != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                       "create_info must not be NULL");
  LVKW_BOOTSTRAP_CHECK(create_info, out_context != NULL, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                       "out_context handle must not be NULL");

  if (create_info->tuning) {
    const LVKW_EventTuning *et = &create_info->tuning->events;
    LVKW_BOOTSTRAP_CHECK(create_info, et->initial_capacity > 0, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                         "initial_capacity must be > 0");
    LVKW_BOOTSTRAP_CHECK(create_info, et->max_capacity > 0, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                         "max_capacity must be > 0");
    LVKW_BOOTSTRAP_CHECK(create_info, et->max_capacity >= et->initial_capacity, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                         "max_capacity must be >= initial_capacity");
    LVKW_BOOTSTRAP_CHECK(create_info, et->growth_factor > 1.0, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                         "growth_factor must be > 1.0");
  }

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_destroy(LVKW_Context *handle) {
  LVKW_CTX_ARG_CONSTRAINT(handle, handle != NULL, "Context handle must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count,
                                                                    const char *const **out_extensions) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, ctx != NULL, "Context handle must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(ctx, count != NULL, "Count pointer must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(ctx, out_extensions != NULL, "out_extensions must not be NULL");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                               LVKW_EventCallback callback, void *userdata) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, callback != NULL, "callback must not be NULL");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms,
                                                               LVKW_EventType event_mask, LVKW_EventCallback callback,
                                                               void *userdata) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, callback != NULL, "callback must not be NULL");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_update(LVKW_Context *ctx, uint32_t field_mask,
                                                           const LVKW_ContextAttributes *attributes) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, attributes != NULL, "attributes must not be NULL");

  return LVKW_SUCCESS;
}

/* --- Monitor Management --- */

static inline LVKW_Status _lvkw_api_constraints_ctx_getMonitors(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors,
                                                                uint32_t *count) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, count != NULL, "count must not be NULL");

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctx_getMonitorModes(LVKW_Context *ctx, LVKW_MonitorId monitor,
                                                                    LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, count != NULL, "count must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(ctx, monitor != LVKW_MONITOR_ID_INVALID, "Monitor ID must not be LVKW_MONITOR_ID_INVALID");

  return LVKW_SUCCESS;
}

/* --- Window Management --- */

static inline LVKW_Status _lvkw_api_constraints_ctx_createWindow(LVKW_Context *ctx,
                                                                 const LVKW_WindowCreateInfo *create_info,
                                                                 LVKW_Window **out_window) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, create_info != NULL, "create_info must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(ctx, out_window != NULL, "out_window must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.logicalSize.x > 0, "Window must have a non-zero size");
  LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.logicalSize.y > 0, "Window must have a non-zero size");

  if (create_info->attributes.minSize.x != 0.0 || create_info->attributes.minSize.y != 0.0) {
    LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.minSize.x >= 0, "minSize.x must be non-negative");
    LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.minSize.y >= 0, "minSize.y must be non-negative");
  }

  if (create_info->attributes.maxSize.x != 0.0 || create_info->attributes.maxSize.y != 0.0) {
    LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.maxSize.x >= 0, "maxSize.x must be non-negative");
    LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.maxSize.y >= 0, "maxSize.y must be non-negative");

    if (create_info->attributes.minSize.x != 0.0 && create_info->attributes.maxSize.x != 0.0) {
      LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.minSize.x <= create_info->attributes.maxSize.x,
                              "minSize.x must be <= maxSize.x");
    }
    if (create_info->attributes.minSize.y != 0.0 && create_info->attributes.maxSize.y != 0.0) {
      LVKW_CTX_ARG_CONSTRAINT(ctx, create_info->attributes.minSize.y <= create_info->attributes.maxSize.y,
                              "minSize.y must be <= maxSize.y");
    }
  }

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_update(LVKW_Window *window, uint32_t field_mask,
                                                           const LVKW_WindowAttributes *attributes) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  LVKW_WND_ARG_CONSTRAINT(window, attributes != NULL, "attributes must not be NULL");

  if (attributes->minSize.x != 0.0 || attributes->minSize.y != 0.0) {
    LVKW_WND_ARG_CONSTRAINT(window, attributes->minSize.x >= 0, "minSize.x must be non-negative");
    LVKW_WND_ARG_CONSTRAINT(window, attributes->minSize.y >= 0, "minSize.y must be non-negative");
  }

  if (attributes->maxSize.x != 0.0 || attributes->maxSize.y != 0.0) {
    LVKW_WND_ARG_CONSTRAINT(window, attributes->maxSize.x >= 0, "maxSize.x must be non-negative");
    LVKW_WND_ARG_CONSTRAINT(window, attributes->maxSize.y >= 0, "maxSize.y must be non-negative");

    if (attributes->minSize.x != 0.0 && attributes->maxSize.x != 0.0) {
      LVKW_WND_ARG_CONSTRAINT(window, attributes->minSize.x <= attributes->maxSize.x, "minSize.x must be <= maxSize.x");
    }
    if (attributes->minSize.y != 0.0 && attributes->maxSize.y != 0.0) {
      LVKW_WND_ARG_CONSTRAINT(window, attributes->minSize.y <= attributes->maxSize.y, "minSize.y must be <= maxSize.y");
    }
  }

  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_destroy(LVKW_Window *handle) {
  // N.B. Window does not need to be healthy to be destroyed, but it needs to exist.
  LVKW_WND_ARG_CONSTRAINT(handle, handle != NULL, "Window handle must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_createVkSurface(LVKW_Window *window, VkInstance instance,
                                                                    VkSurfaceKHR *out_surface) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, instance != NULL, "VkInstance must not be NULL");
  LVKW_WND_ARG_CONSTRAINT(window, out_surface != NULL, "out_surface must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getGeometry(LVKW_Window *window,
                                                                LVKW_WindowGeometry *out_geometry) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, out_geometry != NULL, "out_geometry must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getContext(LVKW_Window *window, LVKW_Context **out_context) {
  // Window does not need to be healty or ready to get its context, but it needs to exist.
  LVKW_WND_ARG_CONSTRAINT(window, window != NULL, "Window handle must not be NULL");
  LVKW_WND_ARG_CONSTRAINT(window, out_context != NULL, "out_context must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_requestFocus(LVKW_Window *window) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text);
LVKW_Status lvkw_wnd_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count);
LVKW_Status lvkw_wnd_getClipboardData(LVKW_Window *window, const char *mime_type, const void **out_data,
                                       size_t *out_size);
LVKW_Status lvkw_wnd_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types, uint32_t *count);

static inline LVKW_Status _lvkw_api_constraints_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, text != NULL, "text must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, out_text != NULL, "out_text must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data,
                                                                     uint32_t count) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, data != NULL || count == 0, "data must not be NULL if count > 0");
  for (uint32_t i = 0; i < count; ++i) {
    LVKW_WND_ARG_CONSTRAINT(window, data[i].mime_type != NULL, "mime_type must not be NULL");
    LVKW_WND_ARG_CONSTRAINT(window, data[i].data != NULL || data[i].size == 0, "data must not be NULL if size > 0");
  }
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardData(LVKW_Window *window, const char *mime_type,
                                                                     const void **out_data, size_t *out_size) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, mime_type != NULL, "mime_type must not be NULL");
  LVKW_WND_ARG_CONSTRAINT(window, out_data != NULL, "out_data must not be NULL");
  LVKW_WND_ARG_CONSTRAINT(window, out_size != NULL, "out_size must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_wnd_getClipboardMimeTypes(LVKW_Window *window,
                                                                         const char ***out_mime_types,
                                                                         uint32_t *count) {
  LVKW_CONSTRAINT_WND_HEALTHY_AND_READY(window);
  LVKW_WND_ARG_CONSTRAINT(window, count != NULL, "count must not be NULL");
  return LVKW_SUCCESS;
}

/* --- Controller Management --- */

#ifdef LVKW_CONTROLLER_ENABLED
static inline LVKW_Status _lvkw_api_constraints_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id,
                                                            LVKW_Controller **out_controller) {
  LVKW_CONSTRAINT_CTX_HEALTHY(ctx);
  LVKW_CTX_ARG_CONSTRAINT(ctx, out_controller != NULL, "out_controller must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctrl_destroy(LVKW_Controller *handle) {
  LVKW_CTX_ARG_CONSTRAINT(&((LVKW_Controller_Base *)handle)->prv.ctx_base->pub, handle != NULL,
                          "Controller handle must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_CTX_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub, controller != NULL,
                          "Controller handle must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub, out_info != NULL,
                          "out_info must not be NULL");
  return LVKW_SUCCESS;
}

static inline LVKW_Status _lvkw_api_constraints_ctrl_setHapticLevels(LVKW_Controller *controller, uint32_t first_haptic,
                                                                     uint32_t count, const LVKW_real_t *intensities) {
  LVKW_CONSTRAINT_CTX_HEALTHY(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub);
  LVKW_CTX_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub, controller != NULL,
                          "Controller handle must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub, intensities != NULL,
                          "intensities array must not be NULL");
  LVKW_CTX_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub,
                          first_haptic + count <= controller->haptic_count, "Haptic channel index out of range");

  for (uint32_t i = 0; i < count; ++i) {
    LVKW_CTX_ARG_CONSTRAINT(&((LVKW_Controller_Base *)controller)->prv.ctx_base->pub,
                            intensities[i] >= 0.0 && intensities[i] <= 1.0, "Intensity must be between 0.0 and 1.0");
  }

  return LVKW_SUCCESS;
}
#endif

#endif  // LVKW_DETAILS_API_CONSTRAINT_H_DEFINED
