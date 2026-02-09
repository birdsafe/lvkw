#ifndef LVKW_LINUX_INTERNAL_H_INCLUDED
#define LVKW_LINUX_INTERNAL_H_INCLUDED

#include "dlib/xkbcommon.h"
#include "lvkw/lvkw.h"

LVKW_Key lvkw_linux_translate_keysym(xkb_keysym_t keysym);

#endif
