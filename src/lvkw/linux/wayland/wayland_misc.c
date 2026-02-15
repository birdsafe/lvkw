// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdio.h>
#include <stdlib.h>

#include "lvkw/details/lvkw_config.h"
#include "lvkw_api_constraints.h"
#include "lvkw_wayland_internal.h"

#ifdef LVKW_ENABLE_INTERNAL_CHECKS
void _lvkw_wayland_symbol_poison_trap(void) {
  fprintf(stderr, "[LVKW FATAL] Poisoned Wayland symbol called.\n");
  fprintf(stderr, "This indicates a bug in LVKW: a global Wayland helper was used instead of a "
                  "context-vtable-aware one.\n");
  abort();
}
#endif

/* xdg_activation_v1 */

static void _xdg_activation_token_handle_done(void *data, struct xdg_activation_token_v1 *token,
                                              const char *token_str) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  lvkw_xdg_activation_v1_activate(ctx, ctx->protocols.opt.xdg_activation_v1, token_str, window->wl.surface);
  lvkw_xdg_activation_token_v1_destroy(ctx, token);
}

static const struct xdg_activation_token_v1_listener _xdg_activation_token_listener = {
    .done = _xdg_activation_token_handle_done,
};

LVKW_Status lvkw_wnd_requestFocus_WL(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (!ctx->protocols.opt.xdg_activation_v1) {
    LVKW_REPORT_WIND_DIAGNOSTIC(window_handle, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "xdg_activation_v1 not available");
    return LVKW_ERROR;
  }

  struct xdg_activation_token_v1 *token =
      lvkw_xdg_activation_v1_get_activation_token(ctx, ctx->protocols.opt.xdg_activation_v1);
  lvkw_xdg_activation_token_v1_add_listener(ctx, token, &_xdg_activation_token_listener, window);

  if (ctx->protocols.wl_seat) {
    lvkw_xdg_activation_token_v1_set_serial(ctx, token, ctx->input.pointer_serial, ctx->protocols.wl_seat);
  }
  lvkw_xdg_activation_token_v1_set_surface(ctx, token, window->wl.surface);
  lvkw_xdg_activation_token_v1_commit(ctx, token);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

/* ext_idle_notification_v1 */

static void _idle_handle_idled(void *data, struct ext_idle_notification_v1 *notification) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {0};
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = true;
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_IDLE_NOTIFICATION, NULL,
                        &ev);
}

static void _idle_handle_resumed(void *data, struct ext_idle_notification_v1 *notification) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {0};
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = false;
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_IDLE_NOTIFICATION, NULL,
                        &ev);
}

const struct ext_idle_notification_v1_listener _lvkw_wayland_idle_listener = {
    .idled = _idle_handle_idled,
    .resumed = _idle_handle_resumed,
};

LVKW_Status lvkw_ctx_update_WL(LVKW_Context *ctx_handle, uint32_t field_mask,
                               const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    uint32_t timeout_ms = attributes->idle_timeout_ms;

    // Clear existing tracker
    if (ctx->idle.notification) {
      lvkw_ext_idle_notification_v1_destroy(ctx, ctx->idle.notification);
      ctx->idle.notification = NULL;
      ctx->idle.timeout_ms = 0;
    }

    if (timeout_ms != LVKW_NEVER) {
      if (!ctx->protocols.opt.ext_idle_notifier_v1 || !ctx->protocols.wl_seat) {
        LVKW_REPORT_CTX_DIAGNOSTIC(ctx_handle, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                   "ext_idle_notifier_v1 or seat not available");
        return LVKW_ERROR;
      }

      ctx->idle.timeout_ms = timeout_ms;
      ctx->idle.notification = lvkw_ext_idle_notifier_v1_get_idle_notification(ctx, 
          ctx->protocols.opt.ext_idle_notifier_v1, timeout_ms, ctx->protocols.wl_seat);
      lvkw_ext_idle_notification_v1_add_listener(ctx, ctx->idle.notification, &_lvkw_wayland_idle_listener,
                                            ctx);
    }
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    if (ctx->inhibit_idle != attributes->inhibit_idle) {
      ctx->inhibit_idle = attributes->inhibit_idle;

      // Update all existing windows
      LVKW_Window_Base *curr = ctx->base.prv.window_list;
      while (curr) {
        LVKW_Window_WL *window = (LVKW_Window_WL *)curr;
        if (ctx->inhibit_idle) {
          if (!window->ext.idle_inhibitor && ctx->protocols.opt.zwp_idle_inhibit_manager_v1) {
            window->ext.idle_inhibitor = lvkw_zwp_idle_inhibit_manager_v1_create_inhibitor(ctx, 
                ctx->protocols.opt.zwp_idle_inhibit_manager_v1, window->wl.surface);
          }
        }
        else {
          if (window->ext.idle_inhibitor) {
            lvkw_zwp_idle_inhibitor_v1_destroy(ctx, window->ext.idle_inhibitor);
            window->ext.idle_inhibitor = NULL;
          }
        }
        curr = curr->prv.next;
      }
    }
  }

  _lvkw_update_base_attributes(&ctx->base, field_mask, attributes);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setClipboardText_WL(LVKW_Window *window, const char *text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Wayland");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardText_WL(LVKW_Window *window, const char **out_text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Wayland");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_setClipboardData_WL(LVKW_Window *window, const LVKW_ClipboardData *data,
                                         uint32_t count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Wayland");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardData_WL(LVKW_Window *window, const char *mime_type,
                                         const void **out_data, size_t *out_size) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Wayland");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_WL(LVKW_Window *window, const char ***out_mime_types,
                                              uint32_t *count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Wayland");
  return LVKW_ERROR;
}
