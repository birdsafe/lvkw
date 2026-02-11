#ifndef LVKW_X11_DLIB_XI_H_INCLUDED
#define LVKW_X11_DLIB_XI_H_INCLUDED

#include <X11/extensions/XInput2.h>

#include "lvkw_internal.h"

#define LVKW_XI_FUNCTIONS_TABLE             \
  LVKW_LIB_FN(QueryVersion, XIQueryVersion) \
  LVKW_LIB_FN(SelectEvents, XISelectEvents) \
  LVKW_LIB_FN(QueryDevice, XIQueryDevice)   \
  LVKW_LIB_FN(FreeDeviceInfo, XIFreeDeviceInfo)

typedef struct _LVKW_Lib_Xi {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) typeof(sym) *name;
  LVKW_XI_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} _LVKW_Lib_Xi;

extern _LVKW_Lib_Xi _lvkw_lib_xi;

#define XIQueryVersion(...) _lvkw_lib_xi.QueryVersion(__VA_ARGS__)
#define XISelectEvents(...) _lvkw_lib_xi.SelectEvents(__VA_ARGS__)
#define XIQueryDevice(...) _lvkw_lib_xi.QueryDevice(__VA_ARGS__)
#define XIFreeDeviceInfo(...) _lvkw_lib_xi.FreeDeviceInfo(__VA_ARGS__)

#endif