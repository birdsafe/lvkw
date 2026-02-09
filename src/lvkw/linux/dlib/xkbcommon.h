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
  LVKW_LIB_FN(state_mod_name_is_active) \
  LVKW_LIB_FN(keymap_mod_get_index)     \
  LVKW_LIB_FN(state_serialize_mods)

// xkbcommon-x11 types and functions (defined manually since headers might be missing)
enum xkb_x11_setup_xkb_extension_flags {
  XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS = 0,
};

#define XKB_X11_MIN_MAJOR_XKB_VERSION 1
#define XKB_X11_MIN_MINOR_XKB_VERSION 0

#define LVKW_XKB_X11_FUNCTIONS_TABLE                                                                      \
  LVKW_LIB_FN_X11(setup_xkb_extension, int,                                                               \
                  (xcb_connection_t * connection, uint16_t major_xkb_version, uint16_t minor_xkb_version, \
                   enum xkb_x11_setup_xkb_extension_flags flags, uint16_t *major_xkb_version_out,         \
                   uint16_t *minor_xkb_version_out, uint8_t *base_event_out, uint8_t *base_error_out))    \
  LVKW_LIB_FN_X11(get_core_keyboard_device_id, int32_t, (xcb_connection_t * connection))                  \
  LVKW_LIB_FN_X11(keymap_new_from_device, struct xkb_keymap *,                                            \
                  (struct xkb_context * context, xcb_connection_t * connection, int32_t device_id,        \
                   enum xkb_keymap_compile_flags flags))                                                  \
  LVKW_LIB_FN_X11(state_new_from_device, struct xkb_state *,                                              \
                  (struct xkb_keymap * keymap, xcb_connection_t * connection, int32_t device_id))

typedef struct LVKW_Lib_Xkb {
  LVKW_External_Lib_Base base;
  LVKW_External_Lib_Base x11_base;

#define LVKW_LIB_FN(name) typeof(xkb_##name) *name;
  LVKW_XKB_FUNCTIONS_TABLE
#undef LVKW_LIB_FN

#define LVKW_LIB_FN_X11(name, ret, args) ret(*x11_##name) args;
  LVKW_XKB_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN_X11
} LVKW_Lib_Xkb;

extern LVKW_Lib_Xkb lvkw_lib_xkb;

#define xkb_context_new(...) lvkw_lib_xkb.context_new(__VA_ARGS__)
#define xkb_context_unref(...) lvkw_lib_xkb.context_unref(__VA_ARGS__)
#define xkb_keymap_new_from_string(...) lvkw_lib_xkb.keymap_new_from_string(__VA_ARGS__)
#define xkb_keymap_unref(...) lvkw_lib_xkb.keymap_unref(__VA_ARGS__)
#define xkb_state_new(...) lvkw_lib_xkb.state_new(__VA_ARGS__)
#define xkb_state_unref(...) lvkw_lib_xkb.state_unref(__VA_ARGS__)
#define xkb_state_update_mask(...) lvkw_lib_xkb.state_update_mask(__VA_ARGS__)
#define xkb_state_key_get_one_sym(...) lvkw_lib_xkb.state_key_get_one_sym(__VA_ARGS__)
#define xkb_state_mod_name_is_active(...) lvkw_lib_xkb.state_mod_name_is_active(__VA_ARGS__)
#define xkb_keymap_mod_get_index(...) lvkw_lib_xkb.keymap_mod_get_index(__VA_ARGS__)
#define xkb_state_serialize_mods(...) lvkw_lib_xkb.state_serialize_mods(__VA_ARGS__)

#define xkb_x11_setup_xkb_extension(...) lvkw_lib_xkb.x11_setup_xkb_extension(__VA_ARGS__)
#define xkb_x11_get_core_keyboard_device_id(...) lvkw_lib_xkb.x11_get_core_keyboard_device_id(__VA_ARGS__)
#define xkb_x11_keymap_new_from_device(...) lvkw_lib_xkb.x11_keymap_new_from_device(__VA_ARGS__)
#define xkb_x11_state_new_from_device(...) lvkw_lib_xkb.x11_state_new_from_device(__VA_ARGS__)

#endif