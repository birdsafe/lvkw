// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_WAYLAND_CURSOR_DLIB_H_INCLUDED
#define LVKW_WAYLAND_CURSOR_DLIB_H_INCLUDED

#include "internal.h"
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

#endif
