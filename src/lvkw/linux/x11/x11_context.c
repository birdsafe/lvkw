// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dlib/X11.h"
#include "dlib/Xi.h"
#include "dlib/Xlib-xcb.h"
#include "dlib/Xss.h"
#include "dlib/linux_loader.h"
#include "dlib/loader.h"
#include "lvkw/c/core.h"
#include "lvkw/lvkw.h"
#include "api_constraints.h"
#include "x11_internal.h"

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

void _lvkw_x11_update_monitors(LVKW_Context_X11 *ctx) {
  if (!ctx->randr_available) return;

  XRRScreenResources *res = lvkw_XRRGetScreenResourcesCurrent(ctx, ctx->display, DefaultRootWindow(ctx->display));
  if (!res) return;

  RROutput primary = lvkw_XRRGetOutputPrimary(ctx, ctx->display, DefaultRootWindow(ctx->display));

  // Mark all existing monitors as potentially lost
  for (LVKW_Monitor_Base *m = ctx->linux_base.base.prv.monitor_list; m; m = m->prv.next) {
    m->pub.flags |= LVKW_MONITOR_STATE_LOST;
  }

  for (int i = 0; i < res->noutput; i++) {
    XRROutputInfo *output = lvkw_XRRGetOutputInfo(ctx, ctx->display, res, res->outputs[i]);
    if (!output || output->connection != RR_Connected || output->crtc == None) {
      if (output) lvkw_XRRFreeOutputInfo(ctx, output);
      continue;
    }

    XRRCrtcInfo *crtc = lvkw_XRRGetCrtcInfo(ctx, ctx->display, res, output->crtc);
    if (!crtc) {
      lvkw_XRRFreeOutputInfo(ctx, output);
      continue;
    }

    LVKW_Monitor_X11 *monitor = NULL;
    for (LVKW_Monitor_Base *m = ctx->linux_base.base.prv.monitor_list; m; m = m->prv.next) {
      LVKW_Monitor_X11 *mx11 = (LVKW_Monitor_X11 *)m;
      if (mx11->output == res->outputs[i]) {
        monitor = mx11;
        break;
      }
    }

    bool is_new = false;
    if (!monitor) {
      monitor = (LVKW_Monitor_X11 *)lvkw_context_alloc(&ctx->linux_base.base, sizeof(LVKW_Monitor_X11));
      memset(monitor, 0, sizeof(*monitor));
      monitor->base.prv.ctx_base = &ctx->linux_base.base;
      monitor->base.pub.context = &ctx->linux_base.base.pub;
      monitor->output = res->outputs[i];
      monitor->base.pub.name = _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache, &ctx->linux_base.base, output->name);
      
      monitor->base.prv.next = ctx->linux_base.base.prv.monitor_list;
      ctx->linux_base.base.prv.monitor_list = &monitor->base;
      is_new = true;
    }

    LVKW_VideoMode old_mode = monitor->base.pub.current_mode;
    LVKW_Scalar old_scale = monitor->base.pub.scale;
    LVKW_LogicalVec old_logical_size = monitor->base.pub.logical_size;

    monitor->base.pub.flags &= (uint32_t)~LVKW_MONITOR_STATE_LOST;
    monitor->base.pub.is_primary = (res->outputs[i] == primary);
    monitor->base.pub.logical_position.x = crtc->x;
    monitor->base.pub.logical_position.y = crtc->y;
    monitor->base.pub.logical_size.x = (LVKW_Scalar)crtc->width;
    monitor->base.pub.logical_size.y = (LVKW_Scalar)crtc->height;
    monitor->base.pub.physical_size.x = (LVKW_Scalar)output->mm_width;
    monitor->base.pub.physical_size.y = (LVKW_Scalar)output->mm_height;
    // X11 logical size is often same as current mode pixel size unless scaling is applied.
    // LVKW assumes logical size is what we get from CRTC.
    
    // Update modes
    if (monitor->modes) {
        lvkw_context_free(&ctx->linux_base.base, monitor->modes);
        monitor->modes = NULL;
        monitor->mode_count = 0;
    }
    
    monitor->mode_count = (uint32_t)output->nmode;
    monitor->modes = (LVKW_VideoMode *)lvkw_context_alloc(&ctx->linux_base.base, sizeof(LVKW_VideoMode) * monitor->mode_count);
    
    for (uint32_t j = 0; j < monitor->mode_count; j++) {
        for (int k = 0; k < res->nmode; k++) {
            if (res->modes[k].id == output->modes[j]) {
                monitor->modes[j].size.x = (int32_t)res->modes[k].width;
                monitor->modes[j].size.y = (int32_t)res->modes[k].height;
                if (res->modes[k].dotClock && res->modes[k].hTotal && res->modes[k].vTotal) {
                    monitor->modes[j].refresh_rate_mhz = (uint32_t)((double)res->modes[k].dotClock / ((double)res->modes[k].hTotal * (double)res->modes[k].vTotal) * 1000.0);
                } else {
                    monitor->modes[j].refresh_rate_mhz = 60000;
                }
                
                if (output->modes[j] == crtc->mode) {
                    monitor->base.pub.current_mode = monitor->modes[j];
                }
                break;
            }
        }
    }

    monitor->base.pub.scale = ctx->scale; // X11 scale is global in this impl
    
    if (is_new) {
        LVKW_Event evt = {0};
        evt.monitor_connection.monitor_ref = (LVKW_MonitorRef *)&monitor->base.pub;
        evt.monitor_connection.connected = true;
        _lvkw_dispatch_event(&ctx->linux_base.base, LVKW_EVENT_TYPE_MONITOR_CONNECTION, NULL, &evt);
    } else {
        bool mode_changed =
            old_mode.size.x != monitor->base.pub.current_mode.size.x ||
            old_mode.size.y != monitor->base.pub.current_mode.size.y ||
            old_mode.refresh_rate_mhz != monitor->base.pub.current_mode.refresh_rate_mhz ||
            old_scale != monitor->base.pub.scale ||
            old_logical_size.x != monitor->base.pub.logical_size.x ||
            old_logical_size.y != monitor->base.pub.logical_size.y;
        if (mode_changed) {
          LVKW_Event evt = {0};
          evt.monitor_mode.monitor = &monitor->base.pub;
          _lvkw_dispatch_event(&ctx->linux_base.base, LVKW_EVENT_TYPE_MONITOR_MODE, NULL, &evt);
        }
    }

    LVKW_Event sync_evt = {0};
    _lvkw_dispatch_event(&ctx->linux_base.base, LVKW_EVENT_TYPE_SYNC, NULL, &sync_evt);

    lvkw_XRRFreeCrtcInfo(ctx, crtc);
    lvkw_XRRFreeOutputInfo(ctx, output);
  }

  // Handle removed monitors
  LVKW_Monitor_Base **prev = &ctx->linux_base.base.prv.monitor_list;
  while (*prev) {
    LVKW_Monitor_Base *curr = *prev;
    if (curr->pub.flags & LVKW_MONITOR_STATE_LOST) {
        LVKW_Event evt = {0};
        evt.monitor_connection.monitor_ref = (LVKW_MonitorRef *)&curr->pub;
        evt.monitor_connection.connected = false;
        _lvkw_dispatch_event(&ctx->linux_base.base, LVKW_EVENT_TYPE_MONITOR_CONNECTION, NULL, &evt);
        
        LVKW_Event sync_evt = {0};
        _lvkw_dispatch_event(&ctx->linux_base.base, LVKW_EVENT_TYPE_SYNC, NULL, &sync_evt);
    }
    prev = &curr->prv.next;
  }

  lvkw_XRRFreeScreenResources(ctx, res);
}

#ifdef LVKW_ENABLE_CONTROLLER
static void _ctrl_push_event_bridge(LVKW_EventType type, LVKW_Window *window, const LVKW_Event *evt,
                                    void *userdata) {
  _lvkw_dispatch_event((LVKW_Context_Base *)userdata, type, window, evt);
}
#endif

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
  ctx->wake_pipe_read = -1;
  ctx->wake_pipe_write = -1;
  ctx->idle_poll_interval_ms =
      (create_info->tuning && create_info->tuning->x11.idle_poll_interval_ms > 0)
          ? create_info->tuning->x11.idle_poll_interval_ms
          : 250;

  if (_lvkw_context_init_base(&ctx->linux_base.base, create_info) != LVKW_SUCCESS) {
    _ctx_free(ctx, ctx);
    return LVKW_ERROR;
  }
#ifdef LVKW_INDIRECT_BACKEND
  ctx->linux_base.base.prv.backend = &_lvkw_x11_backend;
#endif

  if (!lvkw_load_x11_symbols(&ctx->linux_base.base, &ctx->dlib.x11, &ctx->dlib.x11_xcb, &ctx->dlib.xcursor,
                             &ctx->dlib.xrandr, &ctx->dlib.xss, &ctx->dlib.xi, &ctx->dlib.xkb)) {
    _ctx_free(ctx, ctx);
    return LVKW_ERROR;
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
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "XOpenDisplay failed");
    goto cleanup_display;
  }

  {
    int pipefd[2];
    if (pipe(pipefd) == 0) {
      int flags = fcntl(pipefd[0], F_GETFL, 0);
      if (flags >= 0) {
        (void)fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
      }
      flags = fcntl(pipefd[1], F_GETFL, 0);
      if (flags >= 0) {
        (void)fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK);
      }
      ctx->wake_pipe_read = pipefd[0];
      ctx->wake_pipe_write = pipefd[1];
    }
  }

  // Initialize Xrandr
  if (ctx->dlib.xrandr.base.available) {
    if (lvkw_XRRQueryExtension(ctx, ctx->display, &ctx->randr_event_base, &ctx->randr_error_base)) {
      int major, minor;
      if (lvkw_XRRQueryVersion(ctx, ctx->display, &major, &minor)) {
        ctx->randr_available = true;
        lvkw_XRRSelectInput(ctx, ctx->display, DefaultRootWindow(ctx->display),
                            RROutputChangeNotifyMask | RRScreenChangeNotifyMask);
      }
    }
  }

  // Initialize XScreenSaver extension support (server-side extension, not just libXss presence).
  if (ctx->dlib.xss.base.available) {
    int xss_event_base = 0;
    int xss_error_base = 0;
    ctx->xss_available =
        lvkw_XScreenSaverQueryExtension(ctx, ctx->display, &xss_event_base, &xss_error_base) ? true : false;
  }

  // Initialize XKB
  if (ctx->dlib.xkb.base.available) {
    ctx->linux_base.xkb.ctx = lvkw_xkb_context_new(ctx, XKB_CONTEXT_NO_FLAGS);
    if (ctx->linux_base.xkb.ctx && ctx->dlib.x11_xcb.base.available && ctx->dlib.xkb.x11_base.available) {
      xcb_connection_t *conn = lvkw_XGetXCBConnection(ctx, ctx->display);
      if (conn) {
        uint16_t major, minor;
        uint8_t base_evt, base_err;
        if (lvkw_xkb_x11_setup_xkb_extension(
                ctx, conn, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION,
                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS, &major, &minor, &base_evt, &base_err)) {
          int32_t device_id = lvkw_xkb_x11_get_core_keyboard_device_id(ctx, conn);
          if (device_id != -1) {
            ctx->linux_base.xkb.keymap = lvkw_xkb_x11_keymap_new_from_device(
                ctx, ctx->linux_base.xkb.ctx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
            if (ctx->linux_base.xkb.keymap) {
              ctx->linux_base.xkb.state =
                  lvkw_xkb_x11_state_new_from_device(ctx, ctx->linux_base.xkb.keymap, conn, device_id);
            }
          }
        }
      }
    }
  }

  ctx->scale = (LVKW_Scalar)_lvkw_x11_get_scale(ctx);

  ctx->wm_protocols = lvkw_XInternAtom(ctx, ctx->display, "WM_PROTOCOLS", False);
  ctx->wm_delete_window = lvkw_XInternAtom(ctx, ctx->display, "WM_DELETE_WINDOW", False);
  ctx->window_context = lvkw_XUniqueContext(ctx);
  ctx->hidden_cursor =
      _lvkw_x11_create_hidden_cursor(ctx, ctx->display, DefaultRootWindow(ctx->display));
  ctx->net_wm_state = lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_STATE", False);
  ctx->net_wm_state_fullscreen = lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_STATE_FULLSCREEN", False);
  ctx->net_wm_state_maximized_vert =
      lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
  ctx->net_wm_state_maximized_horz =
      lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
  ctx->net_active_window = lvkw_XInternAtom(ctx, ctx->display, "_NET_ACTIVE_WINDOW", False);
  ctx->net_wm_ping = lvkw_XInternAtom(ctx, ctx->display, "_NET_WM_PING", False);
  ctx->wm_take_focus = lvkw_XInternAtom(ctx, ctx->display, "WM_TAKE_FOCUS", False);
  ctx->motif_wm_hints = lvkw_XInternAtom(ctx, ctx->display, "_MOTIF_WM_HINTS", False);
  ctx->clipboard = lvkw_XInternAtom(ctx, ctx->display, "CLIPBOARD", False);
  ctx->targets = lvkw_XInternAtom(ctx, ctx->display, "TARGETS", False);
  ctx->utf8_string = lvkw_XInternAtom(ctx, ctx->display, "UTF8_STRING", False);
  ctx->text_atom = lvkw_XInternAtom(ctx, ctx->display, "TEXT", False);
  ctx->clipboard_property = lvkw_XInternAtom(ctx, ctx->display, "LVKW_CLIPBOARD_DATA", False);
  ctx->xdnd_aware = lvkw_XInternAtom(ctx, ctx->display, "XdndAware", False);

  for (int i = 1; i <= 12; i++) {
    ctx->linux_base.base.prv.standard_cursors[i].pub.flags = LVKW_CURSOR_FLAG_SYSTEM;
    ctx->linux_base.base.prv.standard_cursors[i].prv.ctx_base = &ctx->linux_base.base;
#ifdef LVKW_INDIRECT_BACKEND
    ctx->linux_base.base.prv.standard_cursors[i].prv.backend = ctx->linux_base.base.prv.backend;
#endif
    ctx->linux_base.base.prv.standard_cursors[i].prv.shape = (LVKW_CursorShape)i;
  }

  ctx->xi_opcode = -1;
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
    }
    else {
      ctx->xi_opcode = -1;
    }
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_init_context_Linux(&ctx->linux_base.base, &ctx->linux_base.controller, _ctrl_push_event_bridge, &ctx->linux_base.base);
#endif

  _lvkw_x11_update_monitors(ctx);

  // Apply initial attributes
  _lvkw_update_base_attributes(&ctx->linux_base.base, LVKW_CONTEXT_ATTR_ALL, &create_info->attributes);

  return LVKW_SUCCESS;

cleanup_display:
  if (ctx->wake_pipe_read >= 0) close(ctx->wake_pipe_read);
  if (ctx->wake_pipe_write >= 0) close(ctx->wake_pipe_write);
  if (ctx->linux_base.xkb.state) lvkw_xkb_state_unref(ctx, ctx->linux_base.xkb.state);
  if (ctx->linux_base.xkb.keymap) lvkw_xkb_keymap_unref(ctx, ctx->linux_base.xkb.keymap);
  if (ctx->linux_base.xkb.ctx) lvkw_xkb_context_unref(ctx, ctx->linux_base.xkb.ctx);

  if (ctx->clipboard_owned_mimes) {
    for (uint32_t i = 0; i < ctx->clipboard_owned_mime_count; ++i) {
      lvkw_context_free(&ctx->linux_base.base, ctx->clipboard_owned_mimes[i].bytes);
    }
    lvkw_context_free(&ctx->linux_base.base, ctx->clipboard_owned_mimes);
  }
  if (ctx->clipboard_read_cache) {
    lvkw_context_free(&ctx->linux_base.base, ctx->clipboard_read_cache);
  }
  if (ctx->clipboard_mime_query_ptr) {
    lvkw_context_free(&ctx->linux_base.base, (void *)ctx->clipboard_mime_query_ptr);
  }

  if (ctx->hidden_cursor) lvkw_XFreeCursor(ctx, ctx->display, ctx->hidden_cursor);
  if (ctx->display) lvkw_XCloseDisplay(ctx, ctx->display);
  _lvkw_context_cleanup_base(&ctx->linux_base.base);
  lvkw_unload_x11_symbols(&ctx->dlib.x11, &ctx->dlib.x11_xcb, &ctx->dlib.xcursor, &ctx->dlib.xrandr,
                          &ctx->dlib.xss, &ctx->dlib.xi, &ctx->dlib.xkb);
  _ctx_free(ctx, ctx);
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_destroy_X11(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  #ifdef LVKW_ENABLE_INTERNAL_CHECKS
  #ifdef LVKW_ENABLE_DIAGNOSTICS
 
  lvkw_XSetErrorHandler(ctx, NULL);
  
  #endif 
  #endif
  while (ctx->linux_base.base.prv.window_list) {
    lvkw_wnd_destroy_X11((LVKW_Window *)ctx->linux_base.base.prv.window_list);
  }

  LVKW_Monitor_Base *m = ctx->linux_base.base.prv.monitor_list;
  while (m) {
    LVKW_Monitor_Base *next = m->prv.next;
    LVKW_Monitor_X11 *mx11 = (LVKW_Monitor_X11 *)m;
    if (mx11->modes) {
      lvkw_context_free(&ctx->linux_base.base, mx11->modes);
    }
    // LVKW_Monitor_Base itself is freed in _lvkw_context_cleanup_base
    m = next;
  }

  if (ctx->linux_base.xkb.state) lvkw_xkb_state_unref(ctx, ctx->linux_base.xkb.state);
  if (ctx->linux_base.xkb.keymap) lvkw_xkb_keymap_unref(ctx, ctx->linux_base.xkb.keymap);
  if (ctx->linux_base.xkb.ctx) lvkw_xkb_context_unref(ctx, ctx->linux_base.xkb.ctx);

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_cleanup_context_Linux(&ctx->linux_base.base, &ctx->linux_base.controller);
#endif

  for (int i = 1; i <= 12; i++) {
    Cursor cursor = (Cursor)ctx->linux_base.base.prv.standard_cursors[i].prv.backend_data[0];
    if (cursor != None) {
      lvkw_XFreeCursor(ctx, ctx->display, cursor);
      ctx->linux_base.base.prv.standard_cursors[i].prv.backend_data[0] = (uintptr_t)None;
    }
  }

  _lvkw_context_cleanup_base(&ctx->linux_base.base);
  lvkw_XFreeCursor(ctx, ctx->display, ctx->hidden_cursor);
  lvkw_XCloseDisplay(ctx, ctx->display);

    lvkw_unload_x11_symbols(&ctx->dlib.x11, &ctx->dlib.x11_xcb, &ctx->dlib.xcursor, &ctx->dlib.xrandr,
                             &ctx->dlib.xss, &ctx->dlib.xi, &ctx->dlib.xkb);
  _ctx_free(ctx, ctx);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitors_X11(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs,
                                     uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_refs, count);
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  if (!out_refs) {
    uint32_t monitor_count = 0;
    for (LVKW_Monitor_Base *m = ctx->linux_base.base.prv.monitor_list; m != NULL; m = m->prv.next) {
      if (!(m->pub.flags & LVKW_MONITOR_STATE_LOST)) {
        monitor_count++;
      }
    }
    *count = monitor_count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *count;
  uint32_t filled = 0;
  for (LVKW_Monitor_Base *m = ctx->linux_base.base.prv.monitor_list; m != NULL; m = m->prv.next) {
    if (m->pub.flags & LVKW_MONITOR_STATE_LOST) continue;

    if (filled < room) {
      out_refs[filled++] = (LVKW_MonitorRef *)&m->pub;
    }
    else {
      break;
    }
  }
  *count = filled;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitorModes_X11(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                         LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  (void)ctx_handle;

  LVKW_Monitor_X11 *target_monitor = (LVKW_Monitor_X11 *)monitor;

  if (!out_modes) {
    *count = target_monitor->mode_count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *count;
  uint32_t filled = 0;
  for (uint32_t i = 0; i < target_monitor->mode_count && filled < room; i++) {
    out_modes[filled++] = target_monitor->modes[i];
  }
  *count = filled;
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

LVKW_Status lvkw_ctx_getMetrics_X11(LVKW_Context *ctx, LVKW_MetricsCategory category,
                                      void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx, category, out_data, reset);
  (void)ctx;
  (void)category;
  (void)out_data;
  (void)reset;
  return LVKW_ERROR;
}
