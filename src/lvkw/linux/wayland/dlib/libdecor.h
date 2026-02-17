// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_LIBDECOR_DLIB_H_INCLUDED
#define LVKW_LIBDECOR_DLIB_H_INCLUDED

#include "dlib/wayland-client.h"
#include "dlib/vendor/libdecor.h"
#include "internal.h"

struct libdecor;
void libdecor_set_userdata(struct libdecor *context, void *userdata);
void *libdecor_get_userdata(struct libdecor *context);

#define LVKW_LIBDECOR_FUNCTIONS_TABLE         \
  LVKW_LIB_FN(new)                            \
  LVKW_LIB_FN(unref)                          \
  LVKW_LIB_FN(decorate)                       \
  LVKW_LIB_FN(frame_unref)                    \
  LVKW_LIB_FN(frame_set_title)                \
  LVKW_LIB_FN(frame_set_app_id)               \
  LVKW_LIB_FN(frame_set_capabilities)         \
  LVKW_LIB_FN(frame_map)                      \
  LVKW_LIB_FN(frame_set_visibility)           \
  LVKW_LIB_FN(frame_commit)                   \
  LVKW_LIB_FN(frame_get_xdg_toplevel)         \
  LVKW_LIB_FN(frame_get_xdg_surface)          \
  LVKW_LIB_FN(frame_set_min_content_size)     \
  LVKW_LIB_FN(frame_set_max_content_size)     \
  LVKW_LIB_FN(frame_get_min_content_size)     \
  LVKW_LIB_FN(frame_get_max_content_size)     \
  LVKW_LIB_FN(frame_set_fullscreen)           \
  LVKW_LIB_FN(frame_unset_fullscreen)         \
  LVKW_LIB_FN(frame_set_maximized)            \
  LVKW_LIB_FN(frame_unset_maximized)          \
  LVKW_LIB_FN(frame_translate_coordinate)     \
  LVKW_LIB_FN(state_new)                      \
  LVKW_LIB_FN(state_free)                     \
  LVKW_LIB_FN(configuration_get_content_size) \
  LVKW_LIB_FN(configuration_get_window_state) \
  LVKW_LIB_FN(dispatch)                       \
  LVKW_LIB_OPT_FN(set_userdata)               \
  LVKW_LIB_OPT_FN(get_userdata)               \
  // end of table

typedef struct LVKW_Lib_Decor {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name) __typeof__(libdecor_##name) *name;
#define LVKW_LIB_OPT_FN(name)
  LVKW_LIBDECOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
#undef LVKW_LIB_OPT_FN

  struct {
#define LVKW_LIB_FN(name)
#define LVKW_LIB_OPT_FN(name) __typeof__(libdecor_##name) *name;
    LVKW_LIBDECOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
#undef LVKW_LIB_OPT_FN
  } opt;
} LVKW_Lib_Decor;

#endif
