// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "api_constraints.h"
#include "wayland_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "controller/controller_internal.h"
#endif

void _lvkw_wayland_push_event_cb(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                                 const LVKW_Event *evt) {
  LVKW_Context_WL *ctx_wl = (LVKW_Context_WL *)ctx;

  if (type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
      type == LVKW_EVENT_TYPE_WINDOW_RESIZED) {
    lvkw_event_queue_push_compressible_with_mask(
        &ctx_wl->linux_base.base, &ctx_wl->linux_base.base.prv.event_queue,
        ctx_wl->linux_base.base.prv.pump_event_mask, type, window, evt);
  } else {
    lvkw_event_queue_push_with_mask(&ctx_wl->linux_base.base,
                                    &ctx_wl->linux_base.base.prv.event_queue,
                                    ctx_wl->linux_base.base.prv.pump_event_mask, type, window,
                                    evt);
  }
}

LVKW_Status lvkw_ctx_pumpEvents_WL(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_pumpEvents, ctx_handle, timeout_ms);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  ctx->linux_base.base.prv.pump_event_mask =
      LVKW_ATOMIC_LOAD_RELAXED(&ctx->linux_base.base.prv.event_mask);

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
      count += _lvkw_ctrl_get_poll_fds_Linux(&ctx->linux_base.controller, &pfds[count], 32 - count);
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
          if (read(ctx->wake_fd, &val, sizeof(val)) == -1) {
            if (errno != EAGAIN && errno != EINTR) {
#ifdef LVKW_ENABLE_DIAGNOSTICS
              char msg[256];
              snprintf(msg, sizeof(msg), "Failed to read from wake_fd: %s", strerror(errno));
              LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_BACKEND_FAILURE, msg);
#endif
            }
          }
        }
      } else {
        lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
      }
    }

    lvkw_wl_display_dispatch_pending(ctx, ctx->wl.display);

#ifdef LVKW_ENABLE_CONTROLLER
    _lvkw_ctrl_poll_Linux(&ctx->linux_base.base, &ctx->linux_base.controller);
#endif

    // Check exit conditions
    if (ctx->linux_base.base.prv.event_queue.active->count > 0) {
      break;
    }

    if (poll_timeout == 0) {
      break;
    }
  }

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_commitEvents_WL(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_commitEvents, ctx_handle);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  lvkw_event_queue_begin_gather(&ctx->linux_base.base.prv.event_queue);

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  lvkw_event_queue_note_commit_success(&ctx->linux_base.base.prv.event_queue);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_WL(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                  const LVKW_Event *evt) {
  LVKW_API_VALIDATE(ctx_postEvent, ctx_handle, type, window, evt);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  if (!lvkw_event_queue_push_external(&ctx->linux_base.base.prv.event_queue, type, window, evt)) {
    return LVKW_ERROR;
  }

  if (ctx->wake_fd >= 0) {
    uint64_t val = 1;
    if (write(ctx->wake_fd, &val, sizeof(val)) == -1) {
      if (errno != EAGAIN && errno != EINTR) {
#ifdef LVKW_ENABLE_DIAGNOSTICS
        char msg[256];
        snprintf(msg, sizeof(msg), "Failed to write to wake_fd: %s", strerror(errno));
        LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_BACKEND_FAILURE, msg);
#endif
      }
    }
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_WL(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                   LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_scanEvents, ctx_handle, event_mask, callback, userdata);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  lvkw_event_queue_scan(&ctx->linux_base.base.prv.event_queue, event_mask, callback, userdata);

  return LVKW_SUCCESS;
}
