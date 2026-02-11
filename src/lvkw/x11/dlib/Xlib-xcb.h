#ifndef LVKW_X11_DLIB_XLIB_XCB_H_INCLUDED
#define LVKW_X11_DLIB_XLIB_XCB_H_INCLUDED

#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

#include "lvkw_internal.h"

#define LVKW_XLIB_XCB_FUNCTIONS_TABLE \
  LVKW_LIB_FN(GetXCBConnection, XGetXCBConnection)

typedef struct _LVKW_Lib_X11_XCB {
  LVKW_External_Lib_Base base;
  xcb_connection_t *(*GetXCBConnection)(Display *dpy);
} _LVKW_Lib_X11_XCB;

extern _LVKW_Lib_X11_XCB _lvkw_lib_x11_xcb;

#define XGetXCBConnection(...) _lvkw_lib_x11_xcb.GetXCBConnection(__VA_ARGS__)

#endif
