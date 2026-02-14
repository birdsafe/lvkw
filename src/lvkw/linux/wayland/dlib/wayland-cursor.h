// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_WAYLAND_CURSOR_DLIB_H_INCLUDED
#define LVKW_WAYLAND_CURSOR_DLIB_H_INCLUDED

#include "lvkw_internal.h"
#include "vendor/wayland-cursor.h"

#define LVKW_WL_CURSOR_FUNCTIONS_TABLE \
  LVKW_LIB_FN(theme_load)              \
  LVKW_LIB_FN(theme_destroy)           \
  LVKW_LIB_FN(theme_get_cursor)        \
  LVKW_LIB_FN(image_get_buffer)        \
  // end of table

typedef struct LVKW_Lib_WaylandCursor {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name) __typeof__(wl_cursor_##name) *name;
  LVKW_WL_CURSOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_WaylandCursor;

extern LVKW_Lib_WaylandCursor lvkw_lib_wlc;

#define wl_cursor_theme_load(...) lvkw_lib_wlc.theme_load(__VA_ARGS__)
#define wl_cursor_theme_destroy(...) lvkw_lib_wlc.theme_destroy(__VA_ARGS__)
#define wl_cursor_theme_get_cursor(...) lvkw_lib_wlc.theme_get_cursor(__VA_ARGS__)
#define wl_cursor_image_get_buffer(...) lvkw_lib_wlc.image_get_buffer(__VA_ARGS__)

#endif
