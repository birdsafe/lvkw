// N.B. I have no idea why IWYU is complaining about this
#include <errno.h>
#include <poll.h>
#include <stddef.h>

#include "lvkw_api_checks.h"
#include "lvkw_wayland_internal.h"

void _lvkw_wayland_check_error(LVKW_Context_WL *ctx) {
  if (ctx->base.is_lost) return;

  int err = wl_display_get_error(ctx->wl.display);

  if (err != 0) {
    ctx->base.is_lost = true;

#ifdef LVKW_ENABLE_DIAGNOSIS
    if (err == EPROTO) {
      uint32_t code;
      const struct wl_interface *interface;
      uint32_t id = wl_display_get_protocol_error(ctx->wl.display, &interface, &code);

      char msg[512];
      snprintf(msg, sizeof(msg), "Wayland protocol error on interface %s (id %u): error code %u",
               interface ? interface->name : "unknown", id, code);

      LVKW_REPORT_CTX_DIAGNOSIS(ctx, LVKW_DIAGNOSIS_BACKEND_FAILURE, msg);
    }
    else {
      LVKW_REPORT_CTX_DIAGNOSIS(ctx, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE,
                                "Wayland display disconnected or system error");
    }
#endif
  }
}

void _lvkw_wayland_enqueue_event(LVKW_Context_WL *ctx, const LVKW_Event *evt) {
  if (!lvkw_event_queue_push(&ctx->base, &ctx->events.queue, evt)) {
    LVKW_REPORT_WIND_DIAGNOSIS((LVKW_Window_Base *)evt->common.window, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE,
                               "Wayland event queue is full or allocation failed");
  }
}

void _lvkw_wayland_push_event(LVKW_Context_WL *ctx, const LVKW_Event *evt) { _lvkw_wayland_enqueue_event(ctx, evt); }

void _lvkw_wayland_flush_event_pool(LVKW_Context_WL *ctx) {
  if (!ctx->events.dispatch_ctx) return;

  LVKW_EventDispatchContext_WL dispatch = *ctx->events.dispatch_ctx;

  LVKW_Event ev;

  while (lvkw_event_queue_pop(&ctx->events.queue, LVKW_EVENT_TYPE_ALL, &ev)) {
    if (dispatch.evt_mask & ev.type) {
      dispatch.callback(&ev, dispatch.userdata);
    }
  }

  _lvkw_wayland_check_error(ctx);
}

LVKW_ContextResult lvkw_context_pollEvents_WL(LVKW_Context *ctx_handle, LVKW_EventType evt_mask,

                                              LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);

  if (ctx->base.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_EventDispatchContext_WL dispatch = {

      .callback = callback,

      .userdata = userdata,

      .evt_mask = evt_mask,

  };

  ctx->events.dispatch_ctx = &dispatch;

  if (wl_display_prepare_read(ctx->wl.display) == 0) {
    wl_display_flush(ctx->wl.display);

    struct pollfd pfd = {wl_display_get_fd(ctx->wl.display), POLLIN, 0};

    int ret = poll(&pfd, 1, 0);

    if (ret > 0) {
      wl_display_read_events(ctx->wl.display);
    }

    else {
      wl_display_cancel_read(ctx->wl.display);
    }
  }

  wl_display_dispatch_pending(ctx->wl.display);

  _lvkw_wayland_flush_event_pool(ctx);

  ctx->events.dispatch_ctx = NULL;

  _lvkw_wayland_check_error(ctx);

  if (ctx->base.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_OK;
}