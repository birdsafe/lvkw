// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_LOADER_H_INCLUDED
#define LVKW_X11_LOADER_H_INCLUDED
#include <stdbool.h>

#include "dlib/X11.h"
#include "dlib/Xcursor.h"
#include "dlib/Xi.h"
#include "dlib/Xlib-xcb.h"
#include "dlib/Xrandr.h"
#include "dlib/Xss.h"
#include "dlib/xkbcommon.h"

struct LVKW_Context_Base;

bool lvkw_load_x11_symbols(struct LVKW_Context_Base *ctx, LVKW_Lib_X11 *x11,
                           LVKW_Lib_X11_XCB *x11_xcb, LVKW_Lib_Xcursor *xcursor,
                           LVKW_Lib_Xrandr *xrandr, LVKW_Lib_Xss *xss, LVKW_Lib_Xi *xi,
                           LVKW_Lib_Xkb *xkb);

void lvkw_unload_x11_symbols(LVKW_Lib_X11 *x11, LVKW_Lib_X11_XCB *x11_xcb, LVKW_Lib_Xcursor *xcursor,
                             LVKW_Lib_Xrandr *xrandr, LVKW_Lib_Xss *xss, LVKW_Lib_Xi *xi,
                             LVKW_Lib_Xkb *xkb);

#endif
