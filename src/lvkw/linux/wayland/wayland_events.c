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

void _lvkw_wayland_push_event(LVKW_Context_WL *ctx, LVKW_EventType type, LVKW_Window_WL *window,
                              const LVKW_Event *evt) {
  if (ctx->input.pending_frame.count >= 16) {
    // Should not happen with standard Wayland protocols, but let's be safe
    _lvkw_wayland_dispatch_pending_frame(ctx);
  }

  uint32_t idx = ctx->input.pending_frame.count++;
  ctx->input.pending_frame.types[idx] = type;
  ctx->input.pending_frame.windows[idx] = window;
  if (evt)
    ctx->input.pending_frame.events[idx] = *evt;
  else
    memset(&ctx->input.pending_frame.events[idx], 0, sizeof(LVKW_Event));
}

void _lvkw_wayland_dispatch_pending_frame(LVKW_Context_WL *ctx) {
  if (ctx->input.pending_frame.count == 0) return;

  for (uint32_t i = 0; i < ctx->input.pending_frame.count; ++i) {
    _lvkw_dispatch_event(&ctx->linux_base.base, ctx->input.pending_frame.types[i],
                         (LVKW_Window *)ctx->input.pending_frame.windows[i],
                         &ctx->input.pending_frame.events[i]);
  }

  ctx->input.pending_frame.count = 0;

  LVKW_Event sync_evt = {0};
  _lvkw_dispatch_event(&ctx->linux_base.base, LVKW_EVENT_TYPE_SYNC, NULL, &sync_evt);
}

LVKW_Status lvkw_ctx_pumpEvents_WL(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_pumpEvents, ctx_handle, timeout_ms);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  
  // Drain cross-thread notifications first
  _lvkw_notification_ring_dispatch_all(&ctx->linux_base.base);

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

      struct pollfd pfds[128];
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

      int dnd_fd_idx = -1;
      int dnd_fd = _lvkw_wayland_dnd_get_async_fd(ctx);
      if (dnd_fd >= 0) {
        dnd_fd_idx = count;
        pfds[count].fd = dnd_fd;
        pfds[count].events = POLLIN;
        count++;
      }

      int transfer_pfds_idx = count;
      int transfer_count = _lvkw_wayland_get_transfer_poll_fds(ctx, &pfds[count], 128 - count);
      count += transfer_count;

#ifdef LVKW_ENABLE_CONTROLLER
      count += _lvkw_ctrl_get_poll_fds_Linux(&ctx->linux_base.controller, &pfds[count], 128 - count);
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
            // ignore
          }
        }

        if (transfer_count > 0) {
          _lvkw_wayland_process_transfers(ctx, pfds, transfer_pfds_idx);
        }
      } else {
        lvkw_wl_display_cancel_read(ctx, ctx->wl.display);
      }

      bool dnd_fd_ready = false;
      if (ret > 0 && dnd_fd_idx != -1) {
        short revents = pfds[dnd_fd_idx].revents;
        dnd_fd_ready = (revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)) != 0;
      }
      _lvkw_wayland_dnd_process_async(ctx, dnd_fd_ready, _lvkw_get_timestamp_ms());
    }

    lvkw_wl_display_dispatch_pending(ctx, ctx->wl.display);
    _lvkw_wayland_dnd_process_async(ctx, false, _lvkw_get_timestamp_ms());

#ifdef LVKW_ENABLE_CONTROLLER
    _lvkw_ctrl_poll_Linux(&ctx->linux_base.base, &ctx->linux_base.controller);
#endif

    // Post-poll notifications
    _lvkw_notification_ring_dispatch_all(&ctx->linux_base.base);

    // In a stateless model, we might want to exit after some events were dispatched, 
    // but without a queue count, we rely on the timeout or internal logic.
    // For now, if we polled and dispatched something, we can consider returning if 0 timeout.
    if (poll_timeout == 0) {
      break;
    }
    
    // If we were waiting for a specific event or timeout, we'd continue.
    // For now, let's keep the wait-until-timeout logic.
    if (timeout_ms == 0) break;
    if (timeout_ms != LVKW_NEVER) {
        if (_lvkw_get_timestamp_ms() - start_time >= timeout_ms) break;
    }
  }

  // End of pump sync event
  LVKW_Event sync_evt = {0};
  _lvkw_dispatch_event(&ctx->linux_base.base, LVKW_EVENT_TYPE_SYNC, NULL, &sync_evt);

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_WL(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                  const LVKW_Event *evt) {
  LVKW_API_VALIDATE(ctx_postEvent, ctx_handle, type, window, evt);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  if (!_lvkw_notification_ring_push(&ctx->linux_base.base.prv.external_notifications, type, window, evt)) {
    return LVKW_ERROR;
  }

  if (ctx->wake_fd >= 0) {
    uint64_t val = 1;
    if (write(ctx->wake_fd, &val, sizeof(val)) == -1) {
      // ignore
    }
  }

  return LVKW_SUCCESS;
}
