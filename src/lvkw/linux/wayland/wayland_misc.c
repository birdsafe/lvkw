// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_api_constraints.h"
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
      xdg_activation_v1_get_activation_token(ctx->protocols.opt.xdg_activation_v1);
  xdg_activation_token_v1_add_listener(token, &_xdg_activation_token_listener, window);

  if (ctx->protocols.wl_seat) {
    xdg_activation_token_v1_set_serial(token, ctx->input.pointer_serial, ctx->protocols.wl_seat);
  }
  xdg_activation_token_v1_set_surface(token, window->wl.surface);
  xdg_activation_token_v1_commit(token);

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
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_IDLE_NOTIFICATION, NULL, &ev);
}

static void _idle_handle_resumed(void *data, struct ext_idle_notification_v1 *notification) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {0};
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = false;
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_IDLE_NOTIFICATION, NULL, &ev);
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
      ext_idle_notification_v1_destroy(ctx->idle.notification);
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
      ctx->idle.notification = ext_idle_notifier_v1_get_idle_notification(
          ctx->protocols.opt.ext_idle_notifier_v1, timeout_ms, ctx->protocols.wl_seat);
      ext_idle_notification_v1_add_listener(ctx->idle.notification, &_lvkw_wayland_idle_listener,
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
            window->ext.idle_inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(
                ctx->protocols.opt.zwp_idle_inhibit_manager_v1, window->wl.surface);
          }
        }
        else {
          if (window->ext.idle_inhibitor) {
            zwp_idle_inhibitor_v1_destroy(window->ext.idle_inhibitor);
            window->ext.idle_inhibitor = NULL;
          }
        }
        curr = curr->prv.next;
      }
    }
  }

  if (field_mask & LVKW_CTX_ATTR_DIAGNOSTICS) {
    ctx->base.prv.diagnostic_cb = attributes->diagnostic_cb;
    ctx->base.prv.diagnostic_userdata = attributes->diagnostic_userdata;
  }

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
