// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlib/X11.h"
#include "dlib/Xi.h"
#include "dlib/Xlib-xcb.h"
#include "dlib/Xss.h"
#include "dlib/linux_loader.h"
#include "dlib/loader.h"
#include "lvkw/lvkw-core.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_x11_backend;
#endif

LVKW_Status lvkw_ctx_update_X11(LVKW_Context *ctx_handle, uint32_t field_mask,
                                const LVKW_ContextAttributes *attributes);

static Cursor _lvkw_x11_create_hidden_cursor(LVKW_Context_X11 *ctx, Display *display, Window root) {
  Pixmap blank;
  XColor dummy;

  memset(&dummy, 0, sizeof(dummy));
  char data[1] = {0};
  blank = lvkw_XCreateBitmapFromData(ctx, display, root, data, 1, 1);
  Cursor cursor = lvkw_XCreatePixmapCursor(ctx, display, blank, blank, &dummy, &dummy, 0, 0);

  lvkw_XFreePixmap(ctx, display, blank);
  return cursor;
}

#ifdef LVKW_ENABLE_INTERNAL_CHECKS
static int _lvkw_x11_diagnostic_handler(Display *display, XErrorEvent *event) {
  (void)display;
#ifdef LVKW_ENABLE_DIAGNOSTICS
  fprintf(stderr,
          "LVKW X11 Diagnostic (Process-Global): Request code %d, Minor code %d, Error code %d\n",
          event->request_code, event->minor_code, event->error_code);
#endif
  return 0;
}
#endif

static double _lvkw_x11_get_scale(LVKW_Context_X11 *ctx) {
  double scale = 1.0;
  char *resource_string = lvkw_XResourceManagerString(ctx, ctx->display);
  if (resource_string) {
    XrmDatabase db = lvkw_XrmGetStringDatabase(ctx, resource_string);
    if (db) {
      char *type;
      XrmValue value;
      if (lvkw_XrmGetResource(ctx, db, "Xft.dpi", "Xft.Dpi", &type, &value)) {
        if (type && strcmp(type, "String") == 0) {
          double dpi = atof(value.addr);
          scale = dpi / 96.0;
        }
      }
      lvkw_XrmDestroyDatabase(ctx, db);
    }
  }
  return scale;
}

LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info,
                                LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;

  LVKW_Context_X11 *ctx =
      (LVKW_Context_X11 *)lvkw_context_alloc_bootstrap(create_info, sizeof(LVKW_Context_X11));
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_OUT_OF_MEMORY,
                                     "Failed to allocate context");
    return LVKW_ERROR;
  }

  memset(ctx, 0, sizeof(*ctx));

  _lvkw_context_init_base(&ctx->base, create_info);
#ifdef LVKW_INDIRECT_BACKEND
  ctx->base.prv.backend = &_lvkw_x11_backend;
#endif

  if (!lvkw_load_x11_symbols(&ctx->base, &ctx->dlib.x11, &ctx->dlib.x11_xcb, &ctx->dlib.xcursor,
                             &ctx->dlib.xss, &ctx->dlib.xi, &ctx->dlib.xkb)) {
    _ctx_free(ctx, ctx);
    return LVKW_ERROR;
  }

  if (create_info->flags & LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API) {
    // XInitThreads();
  }

  lvkw_XrmInitialize(ctx);

  // In an ideal world, we'd only require LVKW_ENABLE_DIAGNOSTICS. But some users
  // will want to keep diagnoistics on in prod and/or get info while maintaining the
  // strict guarantees. LVKW_ENABLE_INTERNAL_CHECKS is a decent compromise. No one would
  // ever expect true performance with that on.
  #ifdef LVKW_ENABLE_INTERNAL_CHECKS
  #ifdef LVKW_ENABLE_DIAGNOSTICS
  lvkw_XSetErrorHandler(ctx, _lvkw_x11_diagnostic_handler);
  #endif
  #endif

  ctx->display = lvkw_XOpenDisplay(ctx, NULL);
  if (!ctx->display) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "XOpenDisplay failed");
    goto cleanup_symbols;
  }

  // Initialize XKB
  if (ctx->dlib.xkb.base.available) {
    ctx->xkb.ctx = lvkw_xkb_context_new(ctx, XKB_CONTEXT_NO_FLAGS);
    if (ctx->xkb.ctx && ctx->dlib.x11_xcb.base.available && ctx->dlib.xkb.x11_base.available) {
      xcb_connection_t *conn = lvkw_XGetXCBConnection(ctx, ctx->display);
      if (conn) {
        uint16_t major, minor;
        uint8_t base_evt, base_err;
        if (lvkw_xkb_x11_setup_xkb_extension(
                ctx, conn, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION,
                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS, &major, &minor, &base_evt, &base_err)) {
          int32_t device_id = lvkw_xkb_x11_get_core_keyboard_device_id(ctx, conn);
          if (device_id != -1) {
            ctx->xkb.keymap = lvkw_xkb_x11_keymap_new_from_device(
                ctx, ctx->xkb.ctx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
            if (ctx->xkb.keymap) {
              ctx->xkb.state =
                  lvkw_xkb_x11_state_new_from_device(ctx, ctx->xkb.keymap, conn, device_id);
            }
          }
        }
      }
    }
  }

  ctx->scale = _lvkw_x11_get_scale(ctx);

  ctx->wm_protocols = lvkw_XInternAtom(ctx, ctx->display, "WM_PROTOCOLS", False);
  ctx->wm_delete_window = lvkw_XInternAtom(ctx, ctx->display, "WM_DELETE_WINDOW", False);
  ctx->window_context = lvkw_XUniqueContext(ctx);
  ctx->hidden_cursor =
      _lvkw_x11_create_hidden_cursor(ctx, ctx->display, DefaultRootWindow(ctx->display));
  ctx->net_wm_state = lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_STATE", False);
  ctx->net_wm_state_fullscreen = lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_STATE_FULLSCREEN", False);
  ctx->net_active_window = lvkw_XInternAtom(ctx, ctx->display, "_NET_ACTIVE_WINDOW", False);
  ctx->net_wm_ping = lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_PING", False);
  ctx->wm_take_focus = lvkw_XInternAtom(ctx, ctx->display, "WM_TAKE_FOCUS", False);

  if (ctx->dlib.xi.base.available) {
    int ev, err;
    if (lvkw_XQueryExtension(ctx, ctx->display, "XInputExtension", &ctx->xi_opcode, &ev, &err)) {
      int major = 2, minor = 2;
      if (lvkw_XIQueryVersion(ctx, ctx->display, &major, &minor) == Success) {
        XIEventMask mask;
        mask.deviceid = XIAllMasterDevices;
        mask.mask_len = XIMaskLen(XI_LASTEVENT);
        mask.mask = (unsigned char *)calloc(1, (size_t)mask.mask_len);
        if (mask.mask) {
          XISetMask(mask.mask, XI_RawMotion);
          lvkw_XISelectEvents(ctx, ctx->display, DefaultRootWindow(ctx->display), &mask, 1);
          free(mask.mask);
        }
      }
      else {
        ctx->xi_opcode = -1;
        goto cleanup_display;
      }
    }
    else {
      ctx->xi_opcode = -1;
      goto cleanup_display;
    }
  }
  else {
    ctx->xi_opcode = -1;
    goto cleanup_display;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_init_context_Linux(&ctx->base, &ctx->controller, _lvkw_x11_push_event_cb);
#endif

  // Apply initial attributes
  lvkw_ctx_update_X11((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);

  return LVKW_SUCCESS;

cleanup_display:
  if (ctx->xkb.state) lvkw_xkb_state_unref(ctx, ctx->xkb.state);
  if (ctx->xkb.keymap) lvkw_xkb_keymap_unref(ctx, ctx->xkb.keymap);
  if (ctx->xkb.ctx) lvkw_xkb_context_unref(ctx, ctx->xkb.ctx);
  if (ctx->hidden_cursor) lvkw_XFreeCursor(ctx, ctx->display, ctx->hidden_cursor);
  lvkw_XCloseDisplay(ctx, ctx->display);
cleanup_symbols:
  lvkw_unload_x11_symbols(&ctx->dlib.x11, &ctx->dlib.x11_xcb, &ctx->dlib.xcursor, &ctx->dlib.xss,
                          &ctx->dlib.xi, &ctx->dlib.xkb);
  _ctx_free(ctx, ctx);
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_destroy_X11(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  lvkw_XSetErrorHandler(ctx, NULL);

  while (ctx->base.prv.window_list) {
    lvkw_wnd_destroy_X11((LVKW_Window *)ctx->base.prv.window_list);
  }

  if (ctx->xkb.state) lvkw_xkb_state_unref(ctx, ctx->xkb.state);
  if (ctx->xkb.keymap) lvkw_xkb_keymap_unref(ctx, ctx->xkb.keymap);
  if (ctx->xkb.ctx) lvkw_xkb_context_unref(ctx, ctx->xkb.ctx);

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_cleanup_context_Linux(&ctx->base, &ctx->controller);
#endif

  _lvkw_context_cleanup_base(&ctx->base);
  lvkw_XFreeCursor(ctx, ctx->display, ctx->hidden_cursor);
  lvkw_XCloseDisplay(ctx, ctx->display);

  lvkw_unload_x11_symbols(&ctx->dlib.x11, &ctx->dlib.x11_xcb, &ctx->dlib.xcursor, &ctx->dlib.xss,
                          &ctx->dlib.xi, &ctx->dlib.xkb);

  _ctx_free(ctx, ctx);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitors_X11(LVKW_Context *ctx, LVKW_Monitor **out_monitors,
                                     uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx, out_monitors, count);
  (void)ctx;
  (void)out_monitors;
  // TODO: Implement X11/XRandR monitor enumeration
  *count = 0;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitorModes_X11(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                         LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx, monitor, out_modes, count);
  (void)ctx;
  (void)monitor;
  (void)out_modes;
  // TODO: Implement X11/XRandR monitor mode enumeration
  *count = 0;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getVkExtensions_X11(LVKW_Context *ctx_handle, uint32_t *count,
                                         const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  (void)ctx_handle;
  static const char *extensions[] = {
      "VK_KHR_surface",
      "VK_KHR_xlib_surface",
      NULL,
  };

  *count = 2;
  *out_extensions = extensions;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getTelemetry_X11(LVKW_Context *ctx, LVKW_TelemetryCategory category,
                                      void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getTelemetry, ctx, category, out_data, reset);

  LVKW_Context_X11 *x11_ctx = (LVKW_Context_X11 *)ctx;

  switch (category) {
    case LVKW_TELEMETRY_CATEGORY_EVENTS:
      lvkw_event_queue_get_telemetry(&x11_ctx->base.prv.event_queue, (LVKW_EventTelemetry *)out_data, reset);
      return LVKW_SUCCESS;
    default:
      LVKW_REPORT_CTX_DIAGNOSTIC(&x11_ctx->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                 "Unknown telemetry category");
      return LVKW_ERROR;
  }
}

LVKW_Status lvkw_ctx_update_X11(LVKW_Context *ctx_handle, uint32_t field_mask,
                                const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    if (!ctx->dlib.xss.base.available) {
      LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                 "XScreenSaver extension not available");
      return LVKW_ERROR;
    }

    ctx->idle_timeout_ms = attributes->idle_timeout_ms;
    ctx->is_idle = false;
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    if (ctx->inhibit_idle != attributes->inhibit_idle) {
      if (ctx->dlib.xss.base.available) {
        lvkw_XScreenSaverSuspend(ctx, ctx->display, attributes->inhibit_idle ? True : False);
      }
      ctx->inhibit_idle = attributes->inhibit_idle;
    }
  }

  if (field_mask & LVKW_CTX_ATTR_DIAGNOSTICS) {
    ctx->base.prv.diagnostic_cb = attributes->diagnostic_cb;
    ctx->base.prv.diagnostic_userdata = attributes->diagnostic_userdata;
  }

  return LVKW_SUCCESS;
}