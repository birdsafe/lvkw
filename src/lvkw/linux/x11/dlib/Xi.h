// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_DLIB_XI_H_INCLUDED
#define LVKW_X11_DLIB_XI_H_INCLUDED

#include <X11/extensions/XInput2.h>

#include "lvkw_internal.h"

#define LVKW_XI_FUNCTIONS_TABLE             \
  LVKW_LIB_FN(QueryVersion, XIQueryVersion) \
  LVKW_LIB_FN(SelectEvents, XISelectEvents) \
  LVKW_LIB_FN(QueryDevice, XIQueryDevice)   \
  LVKW_LIB_FN(FreeDeviceInfo, XIFreeDeviceInfo)

typedef struct LVKW_Lib_Xi {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) __typeof__(sym)* name;
  LVKW_XI_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_Xi;

#endif