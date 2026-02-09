#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlib/X11.h"
#include "dlib/Xi.h"
#include "dlib/Xlib-xcb.h"
#include "dlib/Xss.h"
#include "dlib/linux_loader.h"
#include "dlib/loader.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_x11_backend;
#endif

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

static int _lvkw_x11_diagnosis_handler(Display *display, XErrorEvent *event) {
  if (_lvkw_x11_active_ctx) {
    _lvkw_x11_active_ctx->base.is_lost = true;
#ifdef LVKW_ENABLE_DIAGNOSIS
    char buffer[256];
    XGetErrorText(display, event->error_code, buffer, sizeof(buffer));
    LVKW_REPORT_CTX_DIAGNOSIS(&_lvkw_x11_active_ctx->base, LVKW_DIAGNOSIS_BACKEND_FAILURE, buffer);
#endif
  }
  else {
#ifdef LVKW_ENABLE_DIAGNOSIS
    char buffer[256];
    XGetErrorText(display, event->error_code, buffer, sizeof(buffer));
    fprintf(stderr,
            "LVKW X11 Diagnosis (No Active Context): %s (request code: %d, "
            "minor code: %d)\n",
            buffer, event->request_code, event->minor_code);
#endif
  }

  return 0;
}

void _lvkw_x11_check_error(LVKW_Context_X11 *ctx) {
  if (ctx->base.is_lost) return;

  // Most X11 errors are asynchronous. Synchronize to catch any pending
  // diagnosis events.
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

LVKW_Status _lvkw_context_create_X11(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;

  if (!_lvkw_load_x11_symbols()) return LVKW_ERROR_NOOP;

  XrmInitialize();
  XSetErrorHandler(_lvkw_x11_diagnosis_handler);

  LVKW_Allocator alloc = {.alloc = _lvkw_default_alloc, .free = _lvkw_default_free};
  if (create_info->allocator.alloc) alloc = create_info->allocator;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)alloc.alloc(sizeof(LVKW_Context_X11), create_info->user_data);
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_OUT_OF_MEMORY, "Failed to allocate context");
    _lvkw_unload_x11_symbols();
    return LVKW_ERROR_NOOP;
  }
  memset(ctx, 0, sizeof(*ctx));
#ifdef LVKW_INDIRECT_BACKEND
  ctx->base.backend = &_lvkw_x11_backend;
#endif
  ctx->base.alloc_cb = alloc;
  ctx->base.diagnosis_cb = create_info->diagnosis_callback;
  ctx->base.diagnosis_user_data = create_info->diagnosis_user_data;
  ctx->base.user_data = create_info->user_data;

  ctx->display = XOpenDisplay(NULL);
  if (!ctx->display) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_BACKEND_FAILURE, "XOpenDisplay failed");
    _ctx_free(ctx, ctx);
    _lvkw_unload_x11_symbols();
    return LVKW_ERROR_NOOP;
  }

  // Initialize XKB
  if (lvkw_linux_xkb_load()) {
    ctx->xkb.ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (ctx->xkb.ctx && _lvkw_lib_x11_xcb.base.available && lvkw_lib_xkb.x11_base.available) {
      xcb_connection_t *conn = XGetXCBConnection(ctx->display);
      if (conn) {
        uint16_t major, minor;
        uint8_t base_evt, base_err;
        if (xkb_x11_setup_xkb_extension(conn, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION,
                                        XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS, &major, &minor, &base_evt, &base_err)) {
          int32_t device_id = xkb_x11_get_core_keyboard_device_id(conn);
          if (device_id != -1) {
            ctx->xkb.keymap =
                xkb_x11_keymap_new_from_device(ctx->xkb.ctx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
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
  ctx->hidden_cursor = _lvkw_x11_create_hidden_cursor(ctx->display, DefaultRootWindow(ctx->display));
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

  if (lvkw_event_queue_init(&ctx->base, &ctx->event_queue, 64, 4096) != LVKW_OK) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_OUT_OF_MEMORY, "Failed to initialize event queue");
    if (ctx->xkb.state) xkb_state_unref(ctx->xkb.state);
    if (ctx->xkb.keymap) xkb_keymap_unref(ctx->xkb.keymap);
    if (ctx->xkb.ctx) xkb_context_unref(ctx->xkb.ctx);
    lvkw_linux_xkb_unload();
    XFreeCursor(ctx->display, ctx->hidden_cursor);
    XCloseDisplay(ctx->display);
    _ctx_free(ctx, ctx);
    _lvkw_unload_x11_symbols();
    return LVKW_ERROR_NOOP;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;
  return LVKW_OK;
}

void lvkw_context_destroy_X11(LVKW_Context *ctx_handle) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  if (!ctx) return;

  XSetErrorHandler(NULL);

  while (ctx->window_list) {
    lvkw_window_destroy_X11((LVKW_Window *)ctx->window_list);
  }

  if (ctx->xkb.state) xkb_state_unref(ctx->xkb.state);
  if (ctx->xkb.keymap) xkb_keymap_unref(ctx->xkb.keymap);
  if (ctx->xkb.ctx) xkb_context_unref(ctx->xkb.ctx);
  lvkw_linux_xkb_unload();

  lvkw_event_queue_cleanup(&ctx->base, &ctx->event_queue);
  XFreeCursor(ctx->display, ctx->hidden_cursor);
  XCloseDisplay(ctx->display);
  _ctx_free(ctx, ctx);

  _lvkw_unload_x11_symbols();
}

void lvkw_context_getVulkanInstanceExtensions_X11(const LVKW_Context *ctx_handle, uint32_t *count,
                                                  const char **out_extensions) {
  static const char *extensions[] = {"VK_KHR_surface", "VK_KHR_xlib_surface"};
  uint32_t extension_count = 2;

  if (out_extensions == NULL) {
    *count = extension_count;
    return;
  }

  uint32_t to_copy = (*count < extension_count) ? *count : extension_count;
  for (uint32_t i = 0; i < to_copy; ++i) {
    out_extensions[i] = extensions[i];
  }
  *count = to_copy;
}

void *lvkw_context_getUserData_X11(const LVKW_Context *ctx_handle) {
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  return ctx_base->user_data;
}

LVKW_Status lvkw_context_setIdleTimeout_X11(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  _lvkw_x11_check_error(ctx);
  if (ctx->base.is_lost) return LVKW_ERROR_NOOP;

  if (!_lvkw_lib_xss.base.available) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_FEATURE_UNSUPPORTED, "XScreenSaver extension not available");
    return LVKW_ERROR_NOOP;
  }

  ctx->idle_timeout_ms = timeout_ms;
  ctx->is_idle = false;

  return LVKW_OK;
}