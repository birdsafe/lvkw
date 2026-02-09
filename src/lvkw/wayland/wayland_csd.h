#ifndef LVKW_WAYLAND_CSD_H_INCLUDED
#define LVKW_WAYLAND_CSD_H_INCLUDED

#include "lvkw/lvkw.h"

typedef struct LVKW_Context_WL LVKW_Context_WL;
typedef struct LVKW_Window_WL LVKW_Window_WL;

typedef enum LVKW_DecorationMode {
  LVKW_DECORATION_MODE_AUTO,
  LVKW_DECORATION_MODE_SSD,
  LVKW_DECORATION_MODE_CSD,
  LVKW_DECORATION_MODE_NONE
} LVKW_DecorationMode;

bool _lvkw_wayland_create_csd_frame(LVKW_Context_WL *ctx, LVKW_Window_WL *window,
                                    const LVKW_WindowCreateInfo *create_info);

#endif
