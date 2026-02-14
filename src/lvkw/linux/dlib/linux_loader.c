// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dlib/xkbcommon.h"
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

static bool _lib_load_base(struct LVKW_Context_Base* ctx, const char* name, LVKW_External_Lib_Base* tgt) {
  tgt->handle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
  if (!tgt->handle) {
    _set_diagnostic(ctx, "dlopen(%s) failed: %s", name, dlerror());
    tgt->available = false;
    return false;
  }
  tgt->available = true;
  return true;
}

static void _lib_unload_base(LVKW_External_Lib_Base* tgt) {
  if (tgt->handle) {
    dlclose(tgt->handle);
  }
  tgt->handle = NULL;
  tgt->available = false;
}

bool lvkw_linux_xkb_load(struct LVKW_Context_Base *ctx, LVKW_Lib_Xkb *out_lib) {
  if (out_lib->base.available) return true;

  if (!_lib_load_base(ctx, "libxkbcommon.so.0", &out_lib->base)) {
    return false;
  }

  bool functions_ok = true;
#define LVKW_LIB_FN(name)                                            \
  out_lib->name = dlsym(out_lib->base.handle, "xkb_" #name); \
  if (!out_lib->name) {                                          \
    _set_diagnostic(ctx, "dlsym(xkb_" #name ") failed");         \
    functions_ok = false;                                            \
  }
  LVKW_XKB_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _lib_unload_base(&out_lib->base);
    return false;
  }

  // Try to load x11 support (optional)
  if (_lib_load_base(ctx, "libxkbcommon-x11.so.0", &out_lib->x11_base)) {
#define LVKW_LIB_FN_X11(name, ret, args)                                           \
  out_lib->x11_##name = dlsym(out_lib->x11_base.handle, "xkb_x11_" #name); \
  if (!out_lib->x11_##name) {                                                  \
    _set_diagnostic(ctx, "dlsym(xkb_x11_" #name ") failed");                   \
    functions_ok = false;                                                          \
  }
    LVKW_XKB_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN_X11

    if (!functions_ok) {
      _lib_unload_base(&out_lib->x11_base);
    }
  }

  return out_lib->base.available;
}

void lvkw_linux_xkb_unload(LVKW_Lib_Xkb *lib) {
  _lib_unload_base(&lib->x11_base);
  _lib_unload_base(&lib->base);
}
