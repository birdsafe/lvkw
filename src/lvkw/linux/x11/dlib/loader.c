// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "dlib/loader.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>

#include "dlib/X11.h"
#include "dlib/Xcursor.h"
#include "dlib/Xi.h"
#include "dlib/Xlib-xcb.h"
#include "dlib/Xss.h"
#include "dlib/linux_loader.h"
#include "lvkw_internal.h"

#ifdef LVKW_ENABLE_DIAGNOSTICS
static void _set_diagnostic(struct LVKW_Context_Base* ctx, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char msg[256];
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);
  LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_DYNAMIC_LIB_FAILURE, msg);
}
#else
#define _set_diagnostic(...) ((void)0)
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

static bool _x11_load_lib_base(struct LVKW_Context_Base* ctx, const char* name,
                               LVKW_External_Lib_Base* tgt) {
  tgt->handle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
  if (!tgt->handle) {
    _set_diagnostic(ctx, "dlopen(%s) failed: %s", name, dlerror());
    tgt->available = false;
    return false;
  }
  tgt->available = true;

  return tgt->available;
}

static void _x11_unload_lib_base(LVKW_External_Lib_Base* tgt) {
  if (tgt->handle) {
    dlclose(tgt->handle);
  }
  tgt->handle = NULL;
  tgt->available = false;
}

static bool x11_load(struct LVKW_Context_Base* ctx, LVKW_Lib_X11* lib) {
  if (lib->base.available) return true;

  if (!_x11_load_lib_base(ctx, "libX11.so.6", &lib->base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                             \
  lib->name = dlsym(lib->base.handle, #sym);               \
  if (!lib->name) {                                        \
    _set_diagnostic(ctx, "dlsym(" #sym ") failed");        \
    functions_ok = false;                                  \
  }
  LVKW_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&lib->base);
  }
  return lib->base.available;
}

static bool x11_xcb_load(struct LVKW_Context_Base* ctx, LVKW_Lib_X11_XCB* lib) {
  if (lib->base.available) return true;

  if (!_x11_load_lib_base(ctx, "libX11-xcb.so.1", &lib->base)) {
    return false;
  }

  lib->GetXCBConnection = dlsym(lib->base.handle, "XGetXCBConnection");
  if (!lib->GetXCBConnection) {
    _set_diagnostic(ctx, "dlsym(XGetXCBConnection) failed");
    _x11_unload_lib_base(&lib->base);
    return false;
  }

  return true;
}

static bool xcursor_load(struct LVKW_Context_Base* ctx, LVKW_Lib_Xcursor* lib) {
  if (lib->base.available) return true;

  if (!_x11_load_lib_base(ctx, "libXcursor.so.1", &lib->base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                           \
  lib->name = (__typeof__(sym)*)dlsym(lib->base.handle, #sym);           \
  if (!lib->name) {                                                      \
    _set_diagnostic(ctx, "dlsym(" #sym ") failed");                      \
    functions_ok = false;                                                \
  }
  LVKW_XCURSOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&lib->base);
  }
  return lib->base.available;
}

static bool xss_load(struct LVKW_Context_Base* ctx, LVKW_Lib_Xss* lib) {
  if (lib->base.available) return true;

  if (!_x11_load_lib_base(ctx, "libXss.so.1", &lib->base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                           \
  lib->name = (__typeof__(sym)*)dlsym(lib->base.handle, #sym);           \
  if (!lib->name) {                                                      \
    _set_diagnostic(ctx, "dlsym(" #sym ") failed");                      \
    functions_ok = false;                                                \
  }
  LVKW_XSS_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&lib->base);
  }
  return lib->base.available;
}

static bool xi_load(struct LVKW_Context_Base* ctx, LVKW_Lib_Xi* lib) {
  if (lib->base.available) return true;

  if (!_x11_load_lib_base(ctx, "libXi.so.6", &lib->base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                           \
  lib->name = (__typeof__(sym)*)dlsym(lib->base.handle, #sym);           \
  if (!lib->name) {                                                      \
    _set_diagnostic(ctx, "dlsym(" #sym ") failed");                      \
    functions_ok = false;                                                \
  }
  LVKW_XI_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&lib->base);
  }
  return lib->base.available;
}

static bool xrandr_load(struct LVKW_Context_Base* ctx, LVKW_Lib_Xrandr* lib) {
  if (lib->base.available) return true;

  if (!_x11_load_lib_base(ctx, "libXrandr.so.2", &lib->base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                           \
  lib->name = (__typeof__(sym)*)dlsym(lib->base.handle, #sym);           \
  if (!lib->name) {                                                      \
    _set_diagnostic(ctx, "dlsym(" #sym ") failed");                      \
    functions_ok = false;                                                \
  }
  LVKW_XRANDR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&lib->base);
  }
  return lib->base.available;
}

#pragma GCC diagnostic pop

bool lvkw_load_x11_symbols(struct LVKW_Context_Base* ctx, LVKW_Lib_X11* x11,
                           LVKW_Lib_X11_XCB* x11_xcb, LVKW_Lib_Xcursor* xcursor,
                           LVKW_Lib_Xrandr* xrandr, LVKW_Lib_Xss* xss, LVKW_Lib_Xi* xi,
                           LVKW_Lib_Xkb* xkb) {
  if (!x11_load(ctx, x11)) return false;

  x11_xcb_load(ctx, x11_xcb);
  xcursor_load(ctx, xcursor);
  xrandr_load(ctx, xrandr);
  xss_load(ctx, xss);
  xi_load(ctx, xi);
  lvkw_linux_xkb_load(ctx, xkb);

  return true;
}

void lvkw_unload_x11_symbols(LVKW_Lib_X11* x11, LVKW_Lib_X11_XCB* x11_xcb, LVKW_Lib_Xcursor* xcursor,
                             LVKW_Lib_Xrandr* xrandr, LVKW_Lib_Xss* xss, LVKW_Lib_Xi* xi,
                             LVKW_Lib_Xkb* xkb) {
  lvkw_linux_xkb_unload(xkb);
  _x11_unload_lib_base(&xi->base);
  _x11_unload_lib_base(&xss->base);
  _x11_unload_lib_base(&xrandr->base);
  _x11_unload_lib_base(&xcursor->base);
  _x11_unload_lib_base(&x11_xcb->base);
  _x11_unload_lib_base(&x11->base);
}
