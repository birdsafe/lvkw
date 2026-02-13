// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_DLIB_XCURSOR_H_INCLUDED
#define LVKW_X11_DLIB_XCURSOR_H_INCLUDED

#include <X11/Xcursor/Xcursor.h>

#include "lvkw_internal.h"

#define LVKW_XCURSOR_FUNCTIONS_TABLE                       \
  LVKW_LIB_FN(LibraryLoadCursor, XcursorLibraryLoadCursor) \
  // end of table

typedef struct _LVKW_Lib_Xcursor {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) typeof(sym) *name;
  LVKW_XCURSOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} _LVKW_Lib_Xcursor;

extern _LVKW_Lib_Xcursor _lvkw_lib_xcursor;

#define XcursorLibraryLoadCursor(...) _lvkw_lib_xcursor.LibraryLoadCursor(__VA_ARGS__)

#endif