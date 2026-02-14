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

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

static Cursor _lvkw_x11_create_hidden_cursor(Display *display, Window root) {
  Pixmap blank;
  XColor dummy;

  memset(&dummy, 0, sizeof(dummy));
  char data[1] = {0};
  blank = XCreateBitmapFromData(display, root, data, 1, 1);
  Cursor cursor = XCreatePixmapCursor(display, blank, blank, &dummy, &dummy, 0, 0);

  XFreePixmap(display, blank);
  return cursor;
}

#include <threads.h>

static thread_local LVKW_Context_X11 *_lvkw_x11_active_ctx = NULL;

static int _lvkw_x11_diagnostic_handler(Display *display, XErrorEvent *event) {
  if (_lvkw_x11_active_ctx) {
    _lvkw_context_mark_lost(&_lvkw_x11_active_ctx->base);

#ifdef LVKW_ENABLE_DIAGNOSTICS
    char buffer[256];
    XGetErrorText(display, event->error_code, buffer, sizeof(buffer));
    LVKW_REPORT_CTX_DIAGNOSTIC(&_lvkw_x11_active_ctx->base, LVKW_DIAGNOSTIC_BACKEND_FAILURE,
                               buffer);
#endif
  }
  else {
#ifdef LVKW_ENABLE_DIAGNOSTICS
    char buffer[256];
    XGetErrorText(display, event->error_code, buffer, sizeof(buffer));
    fprintf(stderr,
            "LVKW X11 Diagnostic (No Active Context): %s (request code: %d, "
            "minor code: %d)\n",
            buffer, event->request_code, event->minor_code);
#endif
  }

  return 0;
}

void _lvkw_x11_check_error(LVKW_Context_X11 *ctx) {
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return;

  // Most X11 errors are asynchronous. Synchronize to catch any pending
  // diagnostic events.
  _lvkw_x11_active_ctx = ctx;
  XSync(ctx->display, False);
  _lvkw_x11_active_ctx = NULL;
}

static double _lvkw_x11_get_scale(Display *display) {
  double scale = 1.0;
  char *resource_string = XResourceManagerString(display);
  if (resource_string) {
    XrmDatabase db = XrmGetStringDatabase(resource_string);
    if (db) {
      char *type;
      XrmValue value;
      if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value)) {
        if (type && strcmp(type, "String") == 0) {
          double dpi = atof(value.addr);
          scale = dpi / 96.0;
        }
      }
      XrmDestroyDatabase(db);
    }
  }
  return scale;
}

LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info,
                                LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;

  if (!_lvkw_load_x11_symbols()) return LVKW_ERROR;

  if (create_info->flags & LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API) {
    // XInitThreads();
  }

  XrmInitialize();
  XSetErrorHandler(_lvkw_x11_diagnostic_handler);

  LVKW_Allocator alloc = {.alloc_cb = _lvkw_default_alloc, .free_cb = _lvkw_default_free};
  if (create_info->allocator.alloc_cb) alloc = create_info->allocator;

  LVKW_Context_X11 *ctx =
      (LVKW_Context_X11 *)alloc.alloc_cb(sizeof(LVKW_Context_X11), create_info->userdata);
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_OUT_OF_MEMORY,
                                     "Failed to allocate context");
    _lvkw_unload_x11_symbols();
    return LVKW_ERROR;
  }

  _lvkw_context_init_base(&ctx->base, create_info);
#ifdef LVKW_INDIRECT_BACKEND
  ctx->base.prv.backend = &_lvkw_x11_backend;
#endif
  ctx->base.prv.alloc_cb = alloc;

  ctx->display = XOpenDisplay(NULL);
  if (!ctx->display) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "XOpenDisplay failed");
    _ctx_free(ctx, ctx);
    _lvkw_unload_x11_symbols();
    return LVKW_ERROR;
  }

  // Initialize XKB
  if (lvkw_linux_xkb_load()) {
    ctx->xkb.ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (ctx->xkb.ctx && _lvkw_lib_x11_xcb.base.available && lvkw_lib_xkb.x11_base.available) {
      xcb_connection_t *conn = XGetXCBConnection(ctx->display);
      if (conn) {
        uint16_t major, minor;
        uint8_t base_evt, base_err;
        if (xkb_x11_setup_xkb_extension(
                conn, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION,
                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS, &major, &minor, &base_evt, &base_err)) {
          int32_t device_id = xkb_x11_get_core_keyboard_device_id(conn);
          if (device_id != -1) {
            ctx->xkb.keymap = xkb_x11_keymap_new_from_device(ctx->xkb.ctx, conn, device_id,
                                                             XKB_KEYMAP_COMPILE_NO_FLAGS);
            if (ctx->xkb.keymap) {
              ctx->xkb.state = xkb_x11_state_new_from_device(ctx->xkb.keymap, conn, device_id);
            }
          }
        }
      }
    }
  }

  ctx->scale = _lvkw_x11_get_scale(ctx->display);

  ctx->wm_protocols = XInternAtom(ctx->display, "WM_PROTOCOLS", False);
  ctx->wm_delete_window = XInternAtom(ctx->display, "WM_DELETE_WINDOW", False);
  ctx->window_context = XUniqueContext();
  ctx->hidden_cursor =
      _lvkw_x11_create_hidden_cursor(ctx->display, DefaultRootWindow(ctx->display));
  ctx->net_wm_state = XInternAtom(ctx->display, "_NET_WM_STATE", False);
  ctx->net_wm_state_fullscreen = XInternAtom(ctx->display, "_NET_WM_STATE_FULLSCREEN", False);
  ctx->net_active_window = XInternAtom(ctx->display, "_NET_ACTIVE_WINDOW", False);
  ctx->net_wm_ping = XInternAtom(ctx->display, "_NET_WM_PING", False);
  ctx->wm_take_focus = XInternAtom(ctx->display, "WM_TAKE_FOCUS", False);

  if (_lvkw_lib_xi.base.available) {
    int ev, err;
    if (XQueryExtension(ctx->display, "XInputExtension", &ctx->xi_opcode, &ev, &err)) {
      int major = 2, minor = 2;
      if (XIQueryVersion(ctx->display, &major, &minor) == Success) {
        XIEventMask mask;
        mask.deviceid = XIAllMasterDevices;
        mask.mask_len = XIMaskLen(XI_LASTEVENT);
        mask.mask = (unsigned char *)calloc(1, (size_t)mask.mask_len);
        if (mask.mask) {
          XISetMask(mask.mask, XI_RawMotion);
          XISelectEvents(ctx->display, DefaultRootWindow(ctx->display), &mask, 1);
          free(mask.mask);
        }
      }
      else {
        ctx->xi_opcode = -1;
      }
    }
    else {
      ctx->xi_opcode = -1;
    }
  }
  else {
    ctx->xi_opcode = -1;
  }

  LVKW_EventTuning tuning = create_info->tuning->events;
  if (lvkw_event_queue_init(&ctx->base, &ctx->event_queue, tuning) != LVKW_SUCCESS) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_OUT_OF_MEMORY,
                               "Failed to initialize event queue");
    if (ctx->xkb.state) xkb_state_unref(ctx->xkb.state);
    if (ctx->xkb.keymap) xkb_keymap_unref(ctx->xkb.keymap);
    if (ctx->xkb.ctx) xkb_context_unref(ctx->xkb.ctx);
    lvkw_linux_xkb_unload();
    XFreeCursor(ctx->display, ctx->hidden_cursor);
    XCloseDisplay(ctx->display);
    _ctx_free(ctx, ctx);
    _lvkw_unload_x11_symbols();
    return LVKW_ERROR;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_init_context_Linux(&ctx->base, &ctx->controller,
                                (void (*)(LVKW_Context_Base *, LVKW_EventType, LVKW_Window *,
                                          const LVKW_Event *))_lvkw_x11_push_event);
#endif

  // Apply initial attributes
  lvkw_ctx_update_X11((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_destroy_X11(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  XSetErrorHandler(NULL);

  while (ctx->base.prv.window_list) {
    lvkw_wnd_destroy_X11((LVKW_Window *)ctx->base.prv.window_list);
  }

  if (ctx->xkb.state) xkb_state_unref(ctx->xkb.state);
  if (ctx->xkb.keymap) xkb_keymap_unref(ctx->xkb.keymap);
  if (ctx->xkb.ctx) xkb_context_unref(ctx->xkb.ctx);
  lvkw_linux_xkb_unload();

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_cleanup_context_Linux(&ctx->base, &ctx->controller);
#endif

  lvkw_event_queue_cleanup(&ctx->base, &ctx->event_queue);
  _lvkw_context_cleanup_base(&ctx->base);
  XFreeCursor(ctx->display, ctx->hidden_cursor);
  XCloseDisplay(ctx->display);
  _ctx_free(ctx, ctx);

  _lvkw_unload_x11_symbols();
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
      lvkw_event_queue_get_telemetry(&x11_ctx->event_queue, (LVKW_EventTelemetry *)out_data, reset);
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

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    if (!_lvkw_lib_xss.base.available) {
      LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                 "XScreenSaver extension not available");
      return LVKW_ERROR;
    }

    ctx->idle_timeout_ms = attributes->idle_timeout_ms;
    ctx->is_idle = false;
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    if (ctx->inhibit_idle != attributes->inhibit_idle) {
      if (_lvkw_lib_xss.base.available) {
        XScreenSaverSuspend(ctx->display, attributes->inhibit_idle ? True : False);
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