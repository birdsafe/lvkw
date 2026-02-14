// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

// N.B. I have no idea why IWYU is complaining about this
#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>

#include <sys/eventfd.h>
#include <unistd.h>

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
  if (!((uint32_t)ctx->base.prv.event_mask & (uint32_t)type)) return;
  _lvkw_wayland_enqueue_event(ctx, type, window, evt);
}

LVKW_Status lvkw_ctx_syncEvents_WL(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  // OS pump
  if (lvkw_wl_display_prepare_read(ctx, ctx->wl.display) == 0) {
    lvkw_wl_display_flush(ctx, ctx->wl.display);

    struct pollfd pfds[32];
    pfds[0].fd = lvkw_wl_display_get_fd(ctx, ctx->wl.display);
    pfds[0].events = POLLIN;
    int count = 1;

    int wake_fd_idx = -1;
    if (ctx->wake_fd >= 0) {
      wake_fd_idx = count;
      pfds[count].fd = ctx->wake_fd;
      pfds[count].events = POLLIN;
      count++;
    }

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

    int poll_timeout = (timeout_ms == LVKW_NEVER) ? -1 : (int)timeout_ms;
    int ret = poll(pfds, (nfds_t)count, poll_timeout);

    if (ret > 0) {
      if (pfds[0].revents & POLLIN) {
        lvkw_wl_display_read_events(ctx, ctx->wl.display);
      }
      else {
        lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
      }

      if (wake_fd_idx != -1 && (pfds[wake_fd_idx].revents & POLLIN)) {
        uint64_t val;
        (void)read(ctx->wake_fd, &val, sizeof(val));
      }
    }
    else {
      lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
    }
  }

  lvkw_wl_display_dispatch_pending(ctx, ctx->wl.display);

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_poll_Linux(&ctx->base, &ctx->controller);
#endif

  // Promote pending to stable
  lvkw_event_queue_begin_gather(&ctx->events.queue);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_WL(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                  const LVKW_Event *evt) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  if (!lvkw_event_queue_push_external(&ctx->events.queue, type, window, evt)) {
    return LVKW_ERROR;
  }

  if (ctx->wake_fd >= 0) {
    uint64_t val = 1;
    (void)write(ctx->wake_fd, &val, sizeof(val));
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_WL(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                   LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  lvkw_event_queue_scan(&ctx->events.queue, event_mask, callback, userdata);

  return LVKW_SUCCESS;
}
