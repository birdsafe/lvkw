// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "dlib/loader.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "dlib/libdecor.h"
#include "dlib/linux_loader.h"
#include "dlib/wayland-client.h"
#include "dlib/wayland-cursor.h"
#include "dlib/xkbcommon.h"
#include "lvkw_internal.h"

LVKW_Lib_WaylandClient lvkw_lib_wl;
LVKW_Lib_WaylandCursor lvkw_lib_wlc;
LVKW_Lib_Decor lvkw_lib_decor;

static int wayland_refcount = 0;
static pthread_mutex_t loader_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef LVKW_ENABLE_DIAGNOSTICS
static thread_local char _loader_diagnostic[256] = {0};

const char* lvkw_wayland_loader_get_diagnostic(void) { return _loader_diagnostic; }

static void _set_diagnostic(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(_loader_diagnostic, sizeof(_loader_diagnostic), fmt, args);
  va_end(args);
}
#else
#define _set_diagnostic(...) ((void)0)
#endif

// dlsym causes unavoidable cast warnings...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

static bool _wl_load_lib_base(const char* name, LVKW_External_Lib_Base* tgt) {
  tgt->handle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
  if (!tgt->handle) {
    _set_diagnostic("dlopen(%s) failed: %s", name, dlerror());
    tgt->available = false;
    return false;
  }
  tgt->available = true;

  return tgt->available;
}

static void _wl_unload_lib_base(LVKW_External_Lib_Base* tgt) {
  if (tgt->handle) {
    dlclose(tgt->handle);
  }
  tgt->handle = NULL;
  tgt->available = false;
}

static bool wl_load(void) {
  if (lvkw_lib_wl.base.available) return true;

  if (!_wl_load_lib_base("libwayland-client.so.0", &lvkw_lib_wl.base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name)                                         \
  lvkw_lib_wl.name = dlsym(lvkw_lib_wl.base.handle, "wl_" #name); \
  if (!lvkw_lib_wl.name) {                                        \
    _set_diagnostic("dlsym(wl_" #name ") failed");                 \
    functions_ok = false;                                         \
  }
  LVKW_WL_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _wl_unload_lib_base(&lvkw_lib_wl.base);
  }
  return lvkw_lib_wl.base.available;
}

static bool wlc_load(void) {
  if (lvkw_lib_wlc.base.available) return true;

  if (!_wl_load_lib_base("libwayland-cursor.so.0", &lvkw_lib_wlc.base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name)                                                  \
  lvkw_lib_wlc.name = dlsym(lvkw_lib_wlc.base.handle, "wl_cursor_" #name); \
  if (!lvkw_lib_wlc.name) {                                                \
    _set_diagnostic("dlsym(wl_cursor_" #name ") failed");                   \
    functions_ok = false;                                                  \
  }
  LVKW_WL_CURSOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _wl_unload_lib_base(&lvkw_lib_wlc.base);
  }
  return lvkw_lib_wlc.base.available;
}

static bool decor_load(void) {
  if (lvkw_lib_decor.base.available) return true;

  if (!_wl_load_lib_base("libdecor-0.so.0", &lvkw_lib_decor.base)) {
    return false;
  }
  bool functions_ok = true;

#define LVKW_LIB_FN(name)                                                     \
  lvkw_lib_decor.name = dlsym(lvkw_lib_decor.base.handle, "libdecor_" #name); \
  if (!lvkw_lib_decor.name) {                                                 \
    _set_diagnostic("dlsym(libdecor_" #name ") failed");                       \
    functions_ok = false;                                                     \
  }
#define LVKW_LIB_OPT_FN(name) \
  lvkw_lib_decor.opt.name = dlsym(lvkw_lib_decor.base.handle, "libdecor_" #name);
  LVKW_LIBDECOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
#undef LVKW_LIB_OPT_FN

  if (!functions_ok) {
    _wl_unload_lib_base(&lvkw_lib_decor.base);
  }
  return lvkw_lib_decor.base.available;
}

#pragma GCC diagnostic pop

bool lvkw_load_wayland_symbols(void) {
  pthread_mutex_lock(&loader_mutex);
  if (wayland_refcount > 0) {
    wayland_refcount++;
    pthread_mutex_unlock(&loader_mutex);
    return true;
  }

  if (!wl_load()) goto failure_wl;
  if (!wlc_load()) goto failure_wlc;
  if (!lvkw_linux_xkb_load()) goto failure_xkb;

  decor_load();  // Optional

  wayland_refcount++;
  pthread_mutex_unlock(&loader_mutex);
  return true;

failure_xkb:
  _wl_unload_lib_base(&lvkw_lib_wlc.base);
failure_wlc:
  _wl_unload_lib_base(&lvkw_lib_wl.base);
failure_wl:
  pthread_mutex_unlock(&loader_mutex);
  return false;
}

void lvkw_unload_wayland_symbols(void) {
  pthread_mutex_lock(&loader_mutex);
  if (wayland_refcount > 0) {
    wayland_refcount--;
    if (wayland_refcount == 0) {
      _wl_unload_lib_base(&lvkw_lib_decor.base);
      lvkw_linux_xkb_unload();
      _wl_unload_lib_base(&lvkw_lib_wlc.base);
      _wl_unload_lib_base(&lvkw_lib_wl.base);
    }
  }
  pthread_mutex_unlock(&loader_mutex);
}