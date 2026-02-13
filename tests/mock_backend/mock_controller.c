// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "lvkw_mock_internal.h"
#include "lvkw_mem_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER

LVKW_Status lvkw_ctrl_create_Mock(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx;
  LVKW_Controller_Mock *ctrl = lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, sizeof(LVKW_Controller_Mock));
  if (!ctrl) return LVKW_ERROR;

  memset(ctrl, 0, sizeof(*ctrl));
  ctrl->base.pub.analog_count = LVKW_CTRL_ANALOG_STANDARD_COUNT;
  ctrl->base.pub.button_count = LVKW_CTRL_BUTTON_STANDARD_COUNT;
  ctrl->base.pub.haptic_count = LVKW_CTRL_HAPTIC_STANDARD_COUNT;

  ctrl->base.prv.analog_channels_backing =
      lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata,
                 sizeof(LVKW_AnalogChannelInfo) * LVKW_CTRL_ANALOG_STANDARD_COUNT);
  if (ctrl->base.prv.analog_channels_backing) {
    ctrl->base.prv.analog_channels_backing[LVKW_CTRL_ANALOG_LEFT_X].name = "Mock Left Stick X";
    ctrl->base.prv.analog_channels_backing[LVKW_CTRL_ANALOG_LEFT_Y].name = "Mock Left Stick Y";
    ctrl->base.prv.analog_channels_backing[LVKW_CTRL_ANALOG_RIGHT_X].name = "Mock Right Stick X";
    ctrl->base.prv.analog_channels_backing[LVKW_CTRL_ANALOG_RIGHT_Y].name = "Mock Right Stick Y";
    ctrl->base.prv.analog_channels_backing[LVKW_CTRL_ANALOG_LEFT_TRIGGER].name = "Mock Left Trigger";
    ctrl->base.prv.analog_channels_backing[LVKW_CTRL_ANALOG_RIGHT_TRIGGER].name = "Mock Right Trigger";
    ctrl->base.pub.analog_channels = ctrl->base.prv.analog_channels_backing;
  }

  ctrl->base.prv.button_channels_backing =
      lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata,
                 sizeof(LVKW_ButtonChannelInfo) * LVKW_CTRL_BUTTON_STANDARD_COUNT);
  if (ctrl->base.prv.button_channels_backing) {
    const char *btn_names[] = {"South", "East",  "West",  "North", "LB",       "RB",         "Back",      "Start",
                               "Guide", "ThumbL", "ThumbR", "Up",    "Right", "Down", "Left"};
    for (int i = 0; i < LVKW_CTRL_BUTTON_STANDARD_COUNT; ++i) {
      ctrl->base.prv.button_channels_backing[i].name = btn_names[i];
    }
    ctrl->base.pub.button_channels = ctrl->base.prv.button_channels_backing;
  }

  ctrl->base.prv.haptic_channels_backing =
      lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata,
                 sizeof(LVKW_HapticChannelInfo) * LVKW_CTRL_HAPTIC_STANDARD_COUNT);

  if (ctrl->base.prv.haptic_channels_backing) {
    ctrl->base.prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_LOW_FREQ].name = "Mock Low Frequency";
    ctrl->base.prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_HIGH_FREQ].name = "Mock High Frequency";
    ctrl->base.prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_LEFT_TRIGGER].name = "Mock Left Trigger";
    ctrl->base.prv.haptic_channels_backing[LVKW_CTRL_HAPTIC_RIGHT_TRIGGER].name = "Mock Right Trigger";
    ctrl->base.pub.haptic_channels = ctrl->base.prv.haptic_channels_backing;
  }

  ctrl->base.prv.ctx_base = ctx_base;
  ctrl->base.prv.id = id;

  *out_controller = (LVKW_Controller *)ctrl;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_destroy_Mock(LVKW_Controller *controller) {
  LVKW_Controller_Mock *ctrl = (LVKW_Controller_Mock *)controller;
  if (ctrl->base.prv.analog_channels_backing) {
    lvkw_free(&ctrl->base.prv.ctx_base->prv.alloc_cb, ctrl->base.prv.ctx_base->pub.userdata,
              ctrl->base.prv.analog_channels_backing);
  }
  if (ctrl->base.prv.button_channels_backing) {
    lvkw_free(&ctrl->base.prv.ctx_base->prv.alloc_cb, ctrl->base.prv.ctx_base->pub.userdata,
              ctrl->base.prv.button_channels_backing);
  }
  if (ctrl->base.prv.haptic_channels_backing) {
    lvkw_free(&ctrl->base.prv.ctx_base->prv.alloc_cb, ctrl->base.prv.ctx_base->pub.userdata,
              ctrl->base.prv.haptic_channels_backing);
  }
  lvkw_free(&ctrl->base.prv.ctx_base->prv.alloc_cb, ctrl->base.prv.ctx_base->pub.userdata, ctrl);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_getInfo_Mock(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  out_info->name = "Mock Controller";
  out_info->vendor_id = 0x1234;
  out_info->product_id = 0x5678;
  out_info->version = 1;
  out_info->is_standardized = true;
  memset(out_info->guid, 0, 16);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_setHapticLevels_Mock(LVKW_Controller *controller, uint32_t first_haptic, uint32_t count,
                                           const LVKW_real_t *intensities) {
  LVKW_Controller_Mock *ctrl = (LVKW_Controller_Mock *)controller;
  for (uint32_t i = 0; i < count; ++i) {
    ctrl->haptic_levels[first_haptic + i] = intensities[i];
  }
  return LVKW_SUCCESS;
}

#endif
