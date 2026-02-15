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

uint64_t _lvkw_get_timestamp_ms(void);

void _lvkw_wayland_push_event_cb(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                                 const LVKW_Event *evt) {
  LVKW_Context_WL *ctx_wl = (LVKW_Context_WL *)ctx;

  if (type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
      type == LVKW_EVENT_TYPE_WINDOW_RESIZED) {
    lvkw_event_queue_push_compressible(&ctx_wl->base, &ctx_wl->base.prv.event_queue, type, window, evt);
  } else {
    lvkw_event_queue_push(&ctx_wl->base, &ctx_wl->base.prv.event_queue, type, window, evt);
  }
}

LVKW_Status lvkw_ctx_syncEvents_WL(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_syncEvents, ctx_handle, timeout_ms);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  uint64_t start_time = (timeout_ms != LVKW_NEVER && timeout_ms > 0) ? _lvkw_get_timestamp_ms() : 0;

  for (;;) {
    int poll_timeout = -1;
    if (timeout_ms != LVKW_NEVER) {
      if (timeout_ms == 0) {
        poll_timeout = 0;
      } else {
        uint64_t now = _lvkw_get_timestamp_ms();
        uint64_t elapsed = now - start_time;
        if (elapsed >= timeout_ms) {
          poll_timeout = 0;
        } else {
          poll_timeout = (int)(timeout_ms - elapsed);
        }
      }
    }

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

      int ret = poll(pfds, (nfds_t)count, poll_timeout);

      if (ret > 0) {
        if (pfds[0].revents & POLLIN) {
          lvkw_wl_display_read_events(ctx, ctx->wl.display);
        } else {
          lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
        }

        if (wake_fd_idx != -1 && (pfds[wake_fd_idx].revents & POLLIN)) {
          uint64_t val;
          (void)read(ctx->wake_fd, &val, sizeof(val));
        }
      } else {
        lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
      }
    }

    lvkw_wl_display_dispatch_pending(ctx, ctx->wl.display);

#ifdef LVKW_ENABLE_CONTROLLER
    _lvkw_ctrl_poll_Linux(&ctx->base, &ctx->controller);
#endif

    // Check exit conditions
    if (ctx->base.prv.event_queue.active->count > 0) {
      break;
    }

    if (poll_timeout == 0) {
      break;
    }
  }

  // Promote pending to stable
  lvkw_event_queue_begin_gather(&ctx->base.prv.event_queue);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_WL(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                  const LVKW_Event *evt) {
  LVKW_API_VALIDATE(ctx_postEvent, ctx_handle, type, window, evt);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  if (!lvkw_event_queue_push_external(&ctx->base.prv.event_queue, type, window, evt)) {
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
  LVKW_API_VALIDATE(ctx_scanEvents, ctx_handle, event_mask, callback, userdata);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  lvkw_event_queue_scan(&ctx->base.prv.event_queue, event_mask, callback, userdata);

  return LVKW_SUCCESS;
}
