#include "lvkw_api_checks.h"
#include "lvkw_wayland_internal.h"

/* xdg_activation_v1 */

static void _xdg_activation_token_handle_done(void *data, struct xdg_activation_token_v1 *token,
                                              const char *token_str) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  xdg_activation_v1_activate(ctx->protocols.opt.xdg_activation_v1, token_str, window->wl.surface);
  xdg_activation_token_v1_destroy(token);
}

static const struct xdg_activation_token_v1_listener _xdg_activation_token_listener = {
    .done = _xdg_activation_token_handle_done,
};

LVKW_Status lvkw_window_requestFocus_WL(LVKW_Window *window_handle) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (!ctx->protocols.opt.xdg_activation_v1) {
    LVKW_REPORT_WIND_DIAGNOSIS(window_handle, LVKW_DIAGNOSIS_FEATURE_UNSUPPORTED, "xdg_activation_v1 not available");
    return LVKW_ERROR_NOOP;
  }

  struct xdg_activation_token_v1 *token = xdg_activation_v1_get_activation_token(ctx->protocols.opt.xdg_activation_v1);
  xdg_activation_token_v1_add_listener(token, &_xdg_activation_token_listener, window);

  if (ctx->protocols.wl_seat) {
    xdg_activation_token_v1_set_serial(token, ctx->input.pointer_serial, ctx->protocols.wl_seat);
  }
  xdg_activation_token_v1_set_surface(token, window->wl.surface);
  xdg_activation_token_v1_commit(token);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_OK;
}

/* ext_idle_notification_v1 */

static void _idle_handle_idled(void *data, struct ext_idle_notification_v1 *notification) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {.type = LVKW_EVENT_TYPE_IDLE_NOTIFICATION};
  ev.idle.window = NULL;
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = true;
  _lvkw_wayland_push_event(ctx, &ev);
}

static void _idle_handle_resumed(void *data, struct ext_idle_notification_v1 *notification) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {.type = LVKW_EVENT_TYPE_IDLE_NOTIFICATION};
  ev.idle.window = NULL;
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = false;
  _lvkw_wayland_push_event(ctx, &ev);
}

const struct ext_idle_notification_v1_listener _lvkw_wayland_idle_listener = {
    .idled = _idle_handle_idled,
    .resumed = _idle_handle_resumed,
};

LVKW_Status lvkw_context_setIdleTimeout_WL(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  // Clear existing tracker
  if (ctx->idle.notification) {
    ext_idle_notification_v1_destroy(ctx->idle.notification);
    ctx->idle.notification = NULL;
    ctx->idle.timeout_ms = 0;
  }

  if (timeout_ms == LVKW_IDLE_NEVER) {
    return LVKW_OK;
  }

  if (!ctx->protocols.opt.ext_idle_notifier_v1 || !ctx->protocols.wl_seat) {
    LVKW_REPORT_CTX_DIAGNOSIS(ctx_handle, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE,
                              "ext_idle_notifier_v1 or seat not available");
    return LVKW_ERROR_NOOP;
  }

  ctx->idle.timeout_ms = timeout_ms;
  ctx->idle.notification = ext_idle_notifier_v1_get_idle_notification(ctx->protocols.opt.ext_idle_notifier_v1,
                                                                      timeout_ms, ctx->protocols.wl_seat);
  ext_idle_notification_v1_add_listener(ctx->idle.notification, &_lvkw_wayland_idle_listener, ctx);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_OK;
}