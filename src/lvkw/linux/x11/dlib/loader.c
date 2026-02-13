// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "dlib/loader.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stddef.h>

#include "dlib/X11.h"
#include "dlib/Xcursor.h"
#include "dlib/Xi.h"
#include "dlib/Xlib-xcb.h"
#include "dlib/Xss.h"

_LVKW_Lib_X11 _lvkw_lib_x11;
_LVKW_Lib_X11_XCB _lvkw_lib_x11_xcb;
_LVKW_Lib_Xcursor _lvkw_lib_xcursor;
_LVKW_Lib_Xss _lvkw_lib_xss;
_LVKW_Lib_Xi _lvkw_lib_xi;

static int x11_refcount = 0;
static pthread_mutex_t x11_loader_mutex = PTHREAD_MUTEX_INITIALIZER;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

static bool _x11_load_lib_base(const char *name, LVKW_External_Lib_Base *tgt) {
  tgt->handle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
  tgt->available = tgt->handle != NULL;

  return tgt->available;
}

static void _x11_unload_lib_base(LVKW_External_Lib_Base *tgt) {
  if (tgt->handle) {
    dlclose(tgt->handle);
  }
  tgt->handle = NULL;
  tgt->available = false;
}

static bool x11_load(void) {
  if (_lvkw_lib_x11.base.available) return true;

  if (!_x11_load_lib_base("libX11.so.6", &_lvkw_lib_x11.base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                 \
  _lvkw_lib_x11.name = dlsym(_lvkw_lib_x11.base.handle, #sym); \
  if (!_lvkw_lib_x11.name) functions_ok = false;
  LVKW_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&_lvkw_lib_x11.base);
  }
  return _lvkw_lib_x11.base.available;
}

static bool x11_xcb_load(void) {
  if (_lvkw_lib_x11_xcb.base.available) return true;

  if (!_x11_load_lib_base("libX11-xcb.so.1", &_lvkw_lib_x11_xcb.base)) {
    return false;
  }

  _lvkw_lib_x11_xcb.GetXCBConnection = dlsym(_lvkw_lib_x11_xcb.base.handle, "XGetXCBConnection");
  if (!_lvkw_lib_x11_xcb.GetXCBConnection) {
    _x11_unload_lib_base(&_lvkw_lib_x11_xcb.base);
    return false;
  }

  return true;
}

static bool xcursor_load(void) {
  if (_lvkw_lib_xcursor.base.available) return true;

  if (!_x11_load_lib_base("libXcursor.so.1", &_lvkw_lib_xcursor.base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                                        \
  _lvkw_lib_xcursor.name = (typeof(sym) *)dlsym(_lvkw_lib_xcursor.base.handle, #sym); \
  if (!_lvkw_lib_xcursor.name) functions_ok = false;
  LVKW_XCURSOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&_lvkw_lib_xcursor.base);
  }
  return _lvkw_lib_xcursor.base.available;
}

static bool xss_load(void) {
  if (_lvkw_lib_xss.base.available) return true;

  if (!_x11_load_lib_base("libXss.so.1", &_lvkw_lib_xss.base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                                \
  _lvkw_lib_xss.name = (typeof(sym) *)dlsym(_lvkw_lib_xss.base.handle, #sym); \
  if (!_lvkw_lib_xss.name) functions_ok = false;
  LVKW_XSS_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&_lvkw_lib_xss.base);
  }
  return _lvkw_lib_xss.base.available;
}

static bool xi_load(void) {
  if (_lvkw_lib_xi.base.available) return true;

  if (!_x11_load_lib_base("libXi.so.6", &_lvkw_lib_xi.base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name, sym)                                              \
  _lvkw_lib_xi.name = (typeof(sym) *)dlsym(_lvkw_lib_xi.base.handle, #sym); \
  if (!_lvkw_lib_xi.name) functions_ok = false;
  LVKW_XI_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _x11_unload_lib_base(&_lvkw_lib_xi.base);
  }
  return _lvkw_lib_xi.base.available;
}

#pragma GCC diagnostic pop

bool _lvkw_load_x11_symbols(void) {
  pthread_mutex_lock(&x11_loader_mutex);

  if (x11_refcount > 0) {
    x11_refcount++;
    pthread_mutex_unlock(&x11_loader_mutex);
    return true;
  }

  if (!x11_load()) {
    pthread_mutex_unlock(&x11_loader_mutex);
    return false;
  }

  x11_xcb_load();  // Optional but recommended for XKB
  xcursor_load();  // Optional
  xss_load();      // Optional
  xi_load();       // Optional

  x11_refcount++;
  pthread_mutex_unlock(&x11_loader_mutex);
  return true;
}

void _lvkw_unload_x11_symbols(void) {
  pthread_mutex_lock(&x11_loader_mutex);

  if (x11_refcount > 0) {
    x11_refcount--;
    if (x11_refcount == 0) {
      _x11_unload_lib_base(&_lvkw_lib_xi.base);
      _x11_unload_lib_base(&_lvkw_lib_xss.base);
      _x11_unload_lib_base(&_lvkw_lib_xcursor.base);
      _x11_unload_lib_base(&_lvkw_lib_x11_xcb.base);
      _x11_unload_lib_base(&_lvkw_lib_x11.base);
    }
  }

  pthread_mutex_unlock(&x11_loader_mutex);
}
