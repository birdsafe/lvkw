// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "lvkw/lvkw.h"
#include "linux_internal.h"
#include "x11_internal.h"

LVKW_MouseButton _lvkw_x11_translate_button(unsigned int button) {
  switch (button) {
    case 1:
      return LVKW_MOUSE_BUTTON_LEFT;
    case 2:
      return LVKW_MOUSE_BUTTON_MIDDLE;
    case 3:
      return LVKW_MOUSE_BUTTON_RIGHT;
    case 8:
      return LVKW_MOUSE_BUTTON_4;
    case 9:
      return LVKW_MOUSE_BUTTON_5;
    default:
      return (LVKW_MouseButton)0xFFFFFFFF;
  }
}
