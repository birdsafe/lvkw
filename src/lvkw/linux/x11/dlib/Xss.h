// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_DLIB_XSS_H_INCLUDED
#define LVKW_X11_DLIB_XSS_H_INCLUDED

#include <X11/extensions/scrnsaver.h>

#include "lvkw_internal.h"

#define LVKW_XSS_FUNCTIONS_TABLE                          \
  LVKW_LIB_FN(QueryExtension, XScreenSaverQueryExtension) \
  LVKW_LIB_FN(QueryInfo, XScreenSaverQueryInfo)           \
  LVKW_LIB_FN(AllocInfo, XScreenSaverAllocInfo)           \
  LVKW_LIB_FN(Suspend, XScreenSaverSuspend)

typedef struct LVKW_Lib_Xss {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) __typeof__(sym) *name;
  LVKW_XSS_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_Xss;

#endif