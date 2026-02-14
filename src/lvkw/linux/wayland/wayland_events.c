// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

// N.B. I have no idea why IWYU is complaining about this
#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>

#include "lvkw_api_constraints.h"
#include "lvkw_wayland_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "controller/lvkw_controller_internal.h"
#endif

LVKW_Status lvkw_ctx_waitEvents_WL(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                   LVKW_EventType evt_mask, LVKW_EventCallback callback,
                                   void *userdata);

void _lvkw_wayland_check_error(LVKW_Context_WL *ctx) {
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return;

  int err = lvkw_wl_display_get_error(ctx, ctx->wl.display);

  if (err != 0) {
    _lvkw_context_mark_lost(&ctx->base);

#ifdef LVKW_ENABLE_DIAGNOSTICS
    if (err == EPROTO) {
      uint32_t code;
      const struct wl_interface *interface;
      uint32_t id = lvkw_wl_display_get_protocol_error(ctx, ctx->wl.display, &interface, &code);

      char msg[512];
      snprintf(msg, sizeof(msg), "Wayland protocol error on interface %s (id %u): error code %u",
               interface ? interface->name : "unknown", id, code);

      LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_BACKEND_FAILURE, msg);
    }
    else {
      LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                 "Wayland display disconnected or system error");
    }
#endif
  }
}

void _lvkw_wayland_enqueue_event(LVKW_Context_WL *ctx, LVKW_EventType type, LVKW_Window_WL *window,
                                 const LVKW_Event *evt) {
  if (!lvkw_event_queue_push(&ctx->base, &ctx->events.queue, type, (LVKW_Window *)window, evt)) {
    LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                "Wayland event queue is full or allocation failed");
  }
}

void _lvkw_wayland_push_event(LVKW_Context_WL *ctx, LVKW_EventType type, LVKW_Window_WL *window,
                              const LVKW_Event *evt) {
  if (!(ctx->base.pub.flags & LVKW_CTX_STATE_READY)) return;
  _lvkw_wayland_enqueue_event(ctx, type, window, evt);
}

void _lvkw_wayland_flush_event_pool(LVKW_Context_WL *ctx) {
  if (!ctx->events.dispatch_ctx) return;

  LVKW_EventDispatchContext_WL dispatch = *ctx->events.dispatch_ctx;

  LVKW_Event ev;
  LVKW_EventType type;
  LVKW_Window *window;
  while (lvkw_event_queue_pop(&ctx->events.queue, LVKW_EVENT_TYPE_ALL, &type, &window, &ev)) {
    if (dispatch.evt_mask & type) {
      dispatch.callback(type, window, &ev, dispatch.userdata);
    }
  }

  _lvkw_wayland_check_error(ctx);
}

LVKW_Status lvkw_ctx_pollEvents_WL(LVKW_Context *ctx_handle, LVKW_EventType evt_mask,

                                   LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_pollEvents, ctx_handle, evt_mask, callback, userdata);
  return lvkw_ctx_waitEvents_WL(ctx_handle, 0, evt_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents_WL(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                   LVKW_EventType evt_mask,

                                   LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_waitEvents, ctx_handle, timeout_ms, evt_mask, callback, userdata);

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_EventDispatchContext_WL dispatch = {
      .callback = callback,
      .userdata = userdata,
      .evt_mask = evt_mask,
  };

  ctx->events.dispatch_ctx = &dispatch;

  uint64_t start_time = _lvkw_get_timestamp_ms();
  bool matched = false;
  bool wake_on_any = (evt_mask & LVKW_EVENT_FLAG_WAKE_ON_ANY) != 0;
  LVKW_EventType filter_mask = evt_mask & ~LVKW_EVENT_FLAG_WAKE_ON_ANY;

  // We loop until we find a match, or timeout.
  while (!matched) {
    if (lvkw_wl_display_prepare_read(ctx, ctx->wl.display) == 0) {
      lvkw_wl_display_flush(ctx, ctx->wl.display);

      struct pollfd pfds[32];

      pfds[0].fd = lvkw_wl_display_get_fd(ctx, ctx->wl.display);
      pfds[0].events = POLLIN;
      int count = 1;

#ifdef LVKW_ENABLE_CONTROLLER

      if (ctx->controller.inotify_fd >= 0) {
        pfds[count].fd = ctx->controller.inotify_fd;
        pfds[count].events = POLLIN;
        count++;
      }

      struct LVKW_CtrlDevice_Linux *dev = ctx->controller.devices;

      while (dev && count < 32) {
        pfds[count].fd = dev->fd;
        pfds[count].events = POLLIN;
        count++;
        dev = dev->next;
      }

#endif

      int current_timeout = -1;

      if (timeout_ms != LVKW_NEVER) {
        uint64_t elapsed = _lvkw_get_timestamp_ms() - start_time;

        if (timeout_ms > 0 && elapsed >= timeout_ms) {
          lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
          break;
        }

        current_timeout = (int)(timeout_ms - elapsed);
        if (current_timeout < 0) current_timeout = 0;
      }

      int ret = poll(pfds, (nfds_t)count, current_timeout);

      if (ret > 0 && (pfds[0].revents & POLLIN)) {
        lvkw_wl_display_read_events(ctx, ctx->wl.display);
      }
      else {
        lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
      }

      if (ret == 0 && timeout_ms != LVKW_NEVER && timeout_ms != 0) break;  // Timeout
      if (ret < 0 && errno != EINTR) break;                               // Error
      if (ret > 0 && wake_on_any) matched = true;
    }

    lvkw_wl_display_dispatch_pending(ctx, ctx->wl.display);

#ifdef LVKW_ENABLE_CONTROLLER

    _lvkw_ctrl_poll_Linux(&ctx->base, &ctx->controller);

#endif

    // Check if any event in the queue matches our mask.

    if (!matched) {
      matched = lvkw_event_queue_peek(&ctx->events.queue, filter_mask);
    }

    _lvkw_wayland_flush_event_pool(ctx);
    _lvkw_wayland_check_error(ctx);

    if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) break;
    if (timeout_ms == LVKW_NEVER) break;  // pollEvents mode
  }

  ctx->events.dispatch_ctx = NULL;

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}
