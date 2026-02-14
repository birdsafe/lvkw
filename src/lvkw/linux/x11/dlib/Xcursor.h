// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_DLIB_XCURSOR_H_INCLUDED
#define LVKW_X11_DLIB_XCURSOR_H_INCLUDED

#include <X11/Xcursor/Xcursor.h>

#include "lvkw_internal.h"

#define LVKW_XCURSOR_FUNCTIONS_TABLE                       \
  LVKW_LIB_FN(LibraryLoadCursor, XcursorLibraryLoadCursor) \
  // end of table

typedef struct LVKW_Lib_Xcursor {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) __typeof__(sym)* name;
  LVKW_XCURSOR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_Xcursor;

#endif