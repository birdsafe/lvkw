// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_DLIB_XRANDR_H_INCLUDED
#define LVKW_X11_DLIB_XRANDR_H_INCLUDED

#include <X11/extensions/Xrandr.h>

#include "lvkw_internal.h"

#define LVKW_XRANDR_FUNCTIONS_TABLE                               \
  LVKW_LIB_FN(QueryExtension, XRRQueryExtension)                  \
  LVKW_LIB_FN(QueryVersion, XRRQueryVersion)                      \
  LVKW_LIB_FN(GetScreenResourcesCurrent, XRRGetScreenResourcesCurrent) \
  LVKW_LIB_FN(GetOutputInfo, XRRGetOutputInfo)                    \
  LVKW_LIB_FN(GetCrtcInfo, XRRGetCrtcInfo)                        \
  LVKW_LIB_FN(FreeScreenResources, XRRFreeScreenResources)        \
  LVKW_LIB_FN(FreeOutputInfo, XRRFreeOutputInfo)                  \
  LVKW_LIB_FN(FreeCrtcInfo, XRRFreeCrtcInfo)                      \
  LVKW_LIB_FN(SelectInput, XRRSelectInput)                        \
  LVKW_LIB_FN(GetOutputPrimary, XRRGetOutputPrimary)              \
  /* end of table */

typedef struct LVKW_Lib_Xrandr {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) __typeof__(sym)* name;
  LVKW_XRANDR_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_Xrandr;

#endif
