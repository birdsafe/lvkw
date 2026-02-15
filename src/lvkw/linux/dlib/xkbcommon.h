// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_XKBCOMMON_DLIB_H_INCLUDED
#define LVKW_XKBCOMMON_DLIB_H_INCLUDED

#include <stdint.h>
#include <xcb/xcb.h>
#include <xkbcommon/xkbcommon.h>

#include "lvkw_internal.h"

#define LVKW_XKB_FUNCTIONS_TABLE        \
  LVKW_LIB_FN(context_new)              \
  LVKW_LIB_FN(context_unref)            \
  LVKW_LIB_FN(keymap_new_from_string)   \
  LVKW_LIB_FN(keymap_unref)             \
  LVKW_LIB_FN(state_new)                \
  LVKW_LIB_FN(state_unref)              \
  LVKW_LIB_FN(state_update_mask)        \
  LVKW_LIB_FN(state_key_get_one_sym)    \
  LVKW_LIB_FN(state_key_get_utf8)       \
  LVKW_LIB_FN(state_mod_name_is_active) \
  LVKW_LIB_FN(keymap_mod_get_index)     \
  LVKW_LIB_FN(state_serialize_mods)

// xkbcommon-x11 types and functions (defined manually since headers might be missing)
enum xkb_x11_setup_xkb_extension_flags {
  XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS = 0,
};

#define XKB_X11_MIN_MAJOR_XKB_VERSION 1
#define XKB_X11_MIN_MINOR_XKB_VERSION 0

#define LVKW_XKB_X11_FUNCTIONS_TABLE                                                               \
  LVKW_LIB_FN_X11(                                                                                 \
      setup_xkb_extension, int,                                                                    \
      (xcb_connection_t * connection, uint16_t major_xkb_version, uint16_t minor_xkb_version,      \
       enum xkb_x11_setup_xkb_extension_flags flags, uint16_t *major_xkb_version_out,              \
       uint16_t *minor_xkb_version_out, uint8_t *base_event_out, uint8_t *base_error_out))         \
  LVKW_LIB_FN_X11(get_core_keyboard_device_id, int32_t, (xcb_connection_t * connection))           \
  LVKW_LIB_FN_X11(keymap_new_from_device, struct xkb_keymap *,                                     \
                  (struct xkb_context * context, xcb_connection_t * connection, int32_t device_id, \
                   enum xkb_keymap_compile_flags flags))                                           \
  LVKW_LIB_FN_X11(state_new_from_device, struct xkb_state *,                                       \
                  (struct xkb_keymap * keymap, xcb_connection_t * connection, int32_t device_id))

typedef struct LVKW_Lib_Xkb {
  LVKW_External_Lib_Base base;
  LVKW_External_Lib_Base x11_base;

#define LVKW_LIB_FN(name) __typeof__(xkb_##name) *name;
  LVKW_XKB_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

#define LVKW_LIB_FN_X11(name, ret, args) ret(*x11_##name) args;
  LVKW_XKB_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN_X11
} LVKW_Lib_Xkb;

#endif
