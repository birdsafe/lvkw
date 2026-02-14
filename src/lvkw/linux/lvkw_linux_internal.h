// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_LINUX_INTERNAL_H_INCLUDED
#define LVKW_LINUX_INTERNAL_H_INCLUDED

#include "dlib/xkbcommon.h"
#include "lvkw/lvkw.h"
#include "lvkw_types_internal.h"

LVKW_Key lvkw_linux_translate_keysym(xkb_keysym_t keysym);

#ifdef LVKW_ENABLE_CONTROLLER

struct LVKW_CtrlDevice_Linux {
  LVKW_CtrlId id;
  int fd;
  char *name;
  char *path;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t version;

  LVKW_AnalogInputState analogs[LVKW_CTRL_ANALOG_STANDARD_COUNT];
  LVKW_ButtonState buttons[LVKW_CTRL_BUTTON_STANDARD_COUNT];

  struct LVKW_CtrlDevice_Linux *next;
};

typedef struct LVKW_ControllerContext_Linux {
  int inotify_fd;
  struct LVKW_CtrlDevice_Linux *devices;
  uint32_t next_id;
  void (*push_event)(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                     const LVKW_Event *evt);
} LVKW_ControllerContext_Linux;

/* Internal Linux-specific controller functions */
void _lvkw_ctrl_init_context_Linux(LVKW_Context_Base *ctx_base,
                                   LVKW_ControllerContext_Linux *ctrl_ctx,
                                   void (*push_event)(LVKW_Context_Base *ctx, LVKW_EventType type,
                                                      LVKW_Window *window, const LVKW_Event *evt));
void _lvkw_ctrl_cleanup_context_Linux(LVKW_Context_Base *ctx_base,
                                      LVKW_ControllerContext_Linux *ctrl_ctx);
void _lvkw_ctrl_poll_Linux(LVKW_Context_Base *ctx_base, LVKW_ControllerContext_Linux *ctrl_ctx);

/* Public-facing but Linux-internal controller implementation */
LVKW_Status lvkw_ctrl_create_Linux(LVKW_Context *ctx, LVKW_CtrlId id,
                                   LVKW_Controller **out_controller);
LVKW_Status lvkw_ctrl_destroy_Linux(LVKW_Controller *controller);
LVKW_Status lvkw_ctrl_getInfo_Linux(LVKW_Controller *controller, LVKW_CtrlInfo *out_info);
LVKW_Status lvkw_ctrl_setHapticLevels_Linux(LVKW_Controller *controller, uint32_t first_haptic,
                                            uint32_t count, const LVKW_real_t *intensities);

#endif

#endif
