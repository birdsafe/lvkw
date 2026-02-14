// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_LINUX_DLIB_LOADER_H_INCLUDED
#define LVKW_LINUX_DLIB_LOADER_H_INCLUDED

#include <stdbool.h>

#include "dlib/xkbcommon.h"

struct LVKW_Context_Base;

bool lvkw_linux_xkb_load(struct LVKW_Context_Base *ctx, LVKW_Lib_Xkb *out_lib);
void lvkw_linux_xkb_unload(LVKW_Lib_Xkb *lib);

#endif
