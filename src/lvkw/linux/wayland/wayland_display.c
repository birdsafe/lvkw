// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <stdio.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "lvkw_api_constraints.h"
#include "lvkw_wayland_internal.h"
#include "lvkw_diagnostic_internal.h"

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

bool _lvkw_wayland_connect_display(LVKW_Context_WL *ctx) {
  ctx->wl.display = lvkw_wl_display_connect(ctx, NULL);
  if (!ctx->wl.display) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Failed to connect to wayland display");
    return false;
  }

  ctx->wake_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (ctx->wake_fd < 0) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Failed to create wake-up eventfd");
    lvkw_wl_display_disconnect(ctx, ctx->wl.display);
    ctx->wl.display = NULL;
    return false;
  }

  return true;
}

void _lvkw_wayland_disconnect_display(LVKW_Context_WL *ctx) {
  if (ctx->wake_fd >= 0) {
    close(ctx->wake_fd);
    ctx->wake_fd = -1;
  }

  if (ctx->wl.display) {
    lvkw_wl_display_flush(ctx, ctx->wl.display);
    lvkw_wl_display_disconnect(ctx, ctx->wl.display);
    ctx->wl.display = NULL;
  }
}
