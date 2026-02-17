// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_LINUX_INTERNAL_H_INCLUDED
#define LVKW_LINUX_INTERNAL_H_INCLUDED

#include <poll.h>
#include "dlib/xkbcommon.h"
#include "lvkw/lvkw.h"
#include "types_internal.h"

LVKW_Key lvkw_linux_translate_keysym(xkb_keysym_t keysym);
LVKW_Key lvkw_linux_translate_keycode(uint32_t keycode);

#ifdef LVKW_ENABLE_CONTROLLER
struct LVKW_CtrlDevice_Linux {
  LVKW_CtrlId id;
  int fd;
  int rumble_effect_id;
  char *name;
  char *path;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t version;

  LVKW_AnalogInputState analogs[LVKW_CTRL_ANALOG_STANDARD_COUNT];
  LVKW_ButtonState buttons[LVKW_CTRL_BUTTON_STANDARD_COUNT];
  struct LVKW_Controller_Base *controller;

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

int _lvkw_ctrl_get_poll_fds_Linux(LVKW_ControllerContext_Linux *ctrl_ctx, struct pollfd *pfds,
                                  int max_count);

/* Public-facing but Linux-internal controller implementation */
LVKW_Status lvkw_ctrl_getInfo_Linux(LVKW_Controller *controller, LVKW_CtrlInfo *out_info);
LVKW_Status lvkw_ctrl_list_Linux(LVKW_Context *ctx, LVKW_ControllerRef **out_refs, uint32_t *out_count);
LVKW_Status lvkw_ctrl_setHapticLevels_Linux(LVKW_Controller *controller, uint32_t first_haptic,
                                            uint32_t count, const LVKW_Scalar *intensities);

#endif

typedef struct LVKW_Context_Linux {
  LVKW_Context_Base base;

  LVKW_Scalar scale;
  bool inhibit_idle;

  struct {
    struct xkb_context *ctx;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
    struct {
      xkb_mod_index_t shift;
      xkb_mod_index_t ctrl;
      xkb_mod_index_t alt;
      xkb_mod_index_t super;
      xkb_mod_index_t caps;
      xkb_mod_index_t num;
    } mod_indices;
  } xkb;

#ifdef LVKW_ENABLE_CONTROLLER
  LVKW_ControllerContext_Linux controller;
#endif
} LVKW_Context_Linux;

#endif
