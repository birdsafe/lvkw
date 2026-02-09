#ifndef LVKW_LIBDECOR_DLIB_H_INCLUDED
#define LVKW_LIBDECOR_DLIB_H_INCLUDED

// Necessary because libdecor uses these functions, so they need to be rebound
// before including <libdecor.h>
#include "dlib/wayland-client.h"  // IWYU pragma: keep

// ...
#include <libdecor.h>

#include "lvkw_internal.h"

#define LVKW_LIBDECOR_FUNCTIONS_TABLE         \
  LVKW_LIB_FN(new)                            \
  LVKW_LIB_FN(unref)                          \
  LVKW_LIB_FN(decorate)                       \
  LVKW_LIB_FN(frame_unref)                    \
  LVKW_LIB_FN(frame_set_title)                \
  LVKW_LIB_FN(frame_set_app_id)               \
  LVKW_LIB_FN(frame_map)                      \
  LVKW_LIB_FN(frame_set_visibility)           \
  LVKW_LIB_FN(frame_commit)                   \
  LVKW_LIB_FN(frame_get_xdg_toplevel)         \
  LVKW_LIB_FN(frame_get_xdg_surface)          \
  LVKW_LIB_FN(frame_set_min_content_size)     \
  LVKW_LIB_FN(frame_set_max_content_size)     \
  LVKW_LIB_FN(frame_set_fullscreen)           \
  LVKW_LIB_FN(frame_unset_fullscreen)         \
  LVKW_LIB_FN(state_new)                      \
  LVKW_LIB_FN(state_free)                     \
  LVKW_LIB_FN(configuration_get_content_size) \
  LVKW_LIB_FN(configuration_get_window_state) \
  // end of table

typedef struct LVKW_Lib_Decor {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name) typeof(libdecor_##name) *name;
  LVKW_LIBDECOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_Decor;

extern LVKW_Lib_Decor lvkw_lib_decor;

#define libdecor_new(...) lvkw_lib_decor.new(__VA_ARGS__)
#define libdecor_unref(...) lvkw_lib_decor.unref(__VA_ARGS__)
#define libdecor_decorate(...) lvkw_lib_decor.decorate(__VA_ARGS__)
#define libdecor_frame_unref(...) lvkw_lib_decor.frame_unref(__VA_ARGS__)
#define libdecor_frame_set_title(...) lvkw_lib_decor.frame_set_title(__VA_ARGS__)
#define libdecor_frame_set_app_id(...) lvkw_lib_decor.frame_set_app_id(__VA_ARGS__)
#define libdecor_frame_map(...) lvkw_lib_decor.frame_map(__VA_ARGS__)
#define libdecor_frame_set_visibility(...) lvkw_lib_decor.frame_set_visibility(__VA_ARGS__)
#define libdecor_frame_commit(...) lvkw_lib_decor.frame_commit(__VA_ARGS__)
#define libdecor_frame_get_xdg_toplevel(...) lvkw_lib_decor.frame_get_xdg_toplevel(__VA_ARGS__)
#define libdecor_frame_get_xdg_surface(...) lvkw_lib_decor.frame_get_xdg_surface(__VA_ARGS__)
#define libdecor_frame_set_min_content_size(...) lvkw_lib_decor.frame_set_min_content_size(__VA_ARGS__)
#define libdecor_frame_set_max_content_size(...) lvkw_lib_decor.frame_set_max_content_size(__VA_ARGS__)
#define libdecor_frame_set_fullscreen(...) lvkw_lib_decor.frame_set_fullscreen(__VA_ARGS__)
#define libdecor_frame_unset_fullscreen(...) lvkw_lib_decor.frame_unset_fullscreen(__VA_ARGS__)
#define libdecor_state_new(...) lvkw_lib_decor.state_new(__VA_ARGS__)
#define libdecor_state_free(...) lvkw_lib_decor.state_free(__VA_ARGS__)
#define libdecor_configuration_get_content_size(...) lvkw_lib_decor.configuration_get_content_size(__VA_ARGS__)
#define libdecor_configuration_get_window_state(...) lvkw_lib_decor.configuration_get_window_state(__VA_ARGS__)

#endif
