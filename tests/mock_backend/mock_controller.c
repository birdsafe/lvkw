#include <stdlib.h>
#include <string.h>

#include "lvkw_mock_internal.h"
#include "lvkw_mem_internal.h"

#ifdef LVKW_CONTROLLER_ENABLED

LVKW_Status lvkw_ctrl_create_Mock(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx;
  LVKW_Controller_Mock *ctrl = lvkw_alloc(&ctx_base->prv.alloc_cb, ctx_base->pub.userdata, sizeof(LVKW_Controller_Mock));
  if (!ctrl) return LVKW_ERROR;

  memset(ctrl, 0, sizeof(*ctrl));
  ctrl->base.pub.motor_count = LVKW_CTRL_MOTOR_STANDARD_COUNT;
  ctrl->base.prv.ctx_base = ctx_base;
  ctrl->base.prv.id = id;

  *out_controller = (LVKW_Controller *)ctrl;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_destroy_Mock(LVKW_Controller *controller) {
  LVKW_Controller_Mock *ctrl = (LVKW_Controller_Mock *)controller;
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

LVKW_Status lvkw_ctrl_setMotorLevels_Mock(LVKW_Controller *controller, uint32_t first_motor, uint32_t count,
                                          const LVKW_real_t *intensities) {
  LVKW_Controller_Mock *ctrl = (LVKW_Controller_Mock *)controller;
  for (uint32_t i = 0; i < count; ++i) {
    ctrl->motor_levels[first_motor + i] = intensities[i];
  }
  return LVKW_SUCCESS;
}

#endif
