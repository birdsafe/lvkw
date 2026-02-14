// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_WAYLAND_LOADER_H_INCLUDED
#define LVKW_WAYLAND_LOADER_H_INCLUDED

#include <stdbool.h>

#include "wayland-client.h"
#include "wayland-cursor.h"
#include "libdecor.h"
#include "dlib/xkbcommon.h"

struct LVKW_Context_Base;

bool lvkw_load_wayland_symbols(struct LVKW_Context_Base *ctx, LVKW_Lib_WaylandClient *wl, LVKW_Lib_WaylandCursor *wlc, LVKW_Lib_Xkb *xkb, LVKW_Lib_Decor *decor);
void lvkw_unload_wayland_symbols(LVKW_Lib_WaylandClient *wl, LVKW_Lib_WaylandCursor *wlc, LVKW_Lib_Xkb *xkb, LVKW_Lib_Decor *decor);

#endif
