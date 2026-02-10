#ifndef LVKW_X11_DLIB_XSS_H_INCLUDED
#define LVKW_X11_DLIB_XSS_H_INCLUDED

#include <X11/extensions/scrnsaver.h>

#include "lvkw_internal.h"

#define LVKW_XSS_FUNCTIONS_TABLE                          \
  LVKW_LIB_FN(QueryExtension, XScreenSaverQueryExtension) \
  LVKW_LIB_FN(QueryInfo, XScreenSaverQueryInfo)           \
  LVKW_LIB_FN(AllocInfo, XScreenSaverAllocInfo)           \
  LVKW_LIB_FN(Suspend, XScreenSaverSuspend)

typedef struct _LVKW_Lib_Xss {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) typeof(sym) *name;
  LVKW_XSS_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} _LVKW_Lib_Xss;

extern _LVKW_Lib_Xss _lvkw_lib_xss;

#define XScreenSaverQueryExtension(...) _lvkw_lib_xss.QueryExtension(__VA_ARGS__)
#define XScreenSaverQueryInfo(...) _lvkw_lib_xss.QueryInfo(__VA_ARGS__)
#define XScreenSaverAllocInfo(...) _lvkw_lib_xss.AllocInfo(__VA_ARGS__)
#define XScreenSaverSuspend(...) _lvkw_lib_xss.Suspend(__VA_ARGS__)

#endif