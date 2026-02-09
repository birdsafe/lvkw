#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dlib/xkbcommon.h"
#include "lvkw_internal.h"

LVKW_Lib_Xkb lvkw_lib_xkb;

static bool _lib_load_base(const char* name, LVKW_External_Lib_Base* tgt) {
  tgt->handle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
  if (!tgt->handle) {
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

bool lvkw_linux_xkb_load(void) {
  if (lvkw_lib_xkb.base.available) return true;

  if (!_lib_load_base("libxkbcommon.so.0", &lvkw_lib_xkb.base)) {
    return false;
  }

  bool functions_ok = true;
#define LVKW_LIB_FN(name)                                            \
  lvkw_lib_xkb.name = dlsym(lvkw_lib_xkb.base.handle, "xkb_" #name); \
  if (!lvkw_lib_xkb.name) {                                          \
    functions_ok = false;                                            \
  }
  LVKW_XKB_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

  if (!functions_ok) {
    _lib_unload_base(&lvkw_lib_xkb.base);
    return false;
  }

  // Try to load x11 support (optional)
  if (_lib_load_base("libxkbcommon-x11.so.0", &lvkw_lib_xkb.x11_base)) {
#define LVKW_LIB_FN_X11(name, ret, args)                                           \
  lvkw_lib_xkb.x11_##name = dlsym(lvkw_lib_xkb.x11_base.handle, "xkb_x11_" #name); \
  if (!lvkw_lib_xkb.x11_##name) {                                                  \
    functions_ok = false;                                                          \
  }
    LVKW_XKB_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN_X11

    if (!functions_ok) {
      _lib_unload_base(&lvkw_lib_xkb.x11_base);
    }
  }

  return lvkw_lib_xkb.base.available;
}

void lvkw_linux_xkb_unload(void) {
  _lib_unload_base(&lvkw_lib_xkb.x11_base);
  _lib_unload_base(&lvkw_lib_xkb.base);
}