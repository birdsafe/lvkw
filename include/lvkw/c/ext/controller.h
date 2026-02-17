// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_EXT_CONTROLLER_H_INCLUDED
#define LVKW_EXT_CONTROLLER_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw/c/input.h"

/**
 * @file controller.h
 * @brief Game controller and gamepad support extension.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LVKW_ENABLE_CONTROLLER

/** @brief Opaque identifier for a physical controller device. */
typedef uint32_t LVKW_CtrlId;
typedef struct LVKW_ControllerRef LVKW_ControllerRef;
typedef struct LVKW_Controller LVKW_Controller;

/** @brief Detailed hardware information for a controller. */
typedef struct LVKW_CtrlInfo {
  const char *name;
  uint16_t vendor_id;
  uint16_t product_id;
  uint16_t version;
  uint8_t guid[16];
  bool is_standardized;
} LVKW_CtrlInfo;

/** @brief Event data for controller hotplugging. */
typedef struct LVKW_CtrlConnectionEvent {
  LVKW_ControllerRef *controller_ref;
  bool connected;
} LVKW_CtrlConnectionEvent;

/** @brief Runtime status flags for a controller. */
typedef enum LVKW_ControllerFlags {
  LVKW_CONTROLLER_STATE_LOST = 1 << 0,
} LVKW_ControllerFlags;

/** @brief Capabilities and properties of an analog input channel. */
typedef struct LVKW_AnalogChannelInfo {
  const char *name;
} LVKW_AnalogChannelInfo;

/** @brief Capabilities and properties of a digital button. */
typedef struct LVKW_ButtonChannelInfo {
  const char *name;
} LVKW_ButtonChannelInfo;

/** @brief Capabilities and properties of a haptic feedback channel. */
typedef struct LVKW_HapticChannelInfo {
  const char *name;
} LVKW_HapticChannelInfo;

/** @brief State and metadata for an active controller connection. */
typedef struct LVKW_Controller {
  LVKW_Context *context;
  void *userdata;
  uint32_t flags;

  const LVKW_AnalogChannelInfo *analog_channels;
  LVKW_TRANSIENT const LVKW_AnalogInputState *analogs;
  uint32_t analog_count;

  const LVKW_ButtonChannelInfo *button_channels;
  LVKW_TRANSIENT const LVKW_ButtonState *buttons;
  uint32_t button_count;

  const LVKW_HapticChannelInfo *haptic_channels;
  uint32_t haptic_count;
} LVKW_Controller;

/** @brief Standardized analog axis indices. */
typedef enum LVKW_CtrlAnalog {
  LVKW_CTRL_ANALOG_LEFT_X = 0,
  LVKW_CTRL_ANALOG_LEFT_Y = 1,
  LVKW_CTRL_ANALOG_RIGHT_X = 2,
  LVKW_CTRL_ANALOG_RIGHT_Y = 3,
  LVKW_CTRL_ANALOG_LEFT_TRIGGER = 4,
  LVKW_CTRL_ANALOG_RIGHT_TRIGGER = 5,
  LVKW_CTRL_ANALOG_STANDARD_COUNT = 6
} LVKW_CtrlAnalog;

/** @brief Standardized button indices. */
typedef enum LVKW_CtrlButton {
  LVKW_CTRL_BUTTON_SOUTH = 0,
  LVKW_CTRL_BUTTON_EAST = 1,
  LVKW_CTRL_BUTTON_WEST = 2,
  LVKW_CTRL_BUTTON_NORTH = 3,
  LVKW_CTRL_BUTTON_LB = 4,
  LVKW_CTRL_BUTTON_RB = 5,
  LVKW_CTRL_BUTTON_BACK = 6,
  LVKW_CTRL_BUTTON_START = 7,
  LVKW_CTRL_BUTTON_GUIDE = 8,
  LVKW_CTRL_BUTTON_L_THUMB = 9,
  LVKW_CTRL_BUTTON_R_THUMB = 10,
  LVKW_CTRL_BUTTON_DPAD_UP = 11,
  LVKW_CTRL_BUTTON_DPAD_RIGHT = 12,
  LVKW_CTRL_BUTTON_DPAD_DOWN = 13,
  LVKW_CTRL_BUTTON_DPAD_LEFT = 14,
  LVKW_CTRL_BUTTON_STANDARD_COUNT = 15
} LVKW_CtrlButton;

/** @brief Standardized haptic channel indices. */
typedef enum LVKW_CtrlHaptic {
  LVKW_CTRL_HAPTIC_LOW_FREQ = 0,
  LVKW_CTRL_HAPTIC_HIGH_FREQ = 1,
  LVKW_CTRL_HAPTIC_LEFT_TRIGGER = 2,
  LVKW_CTRL_HAPTIC_RIGHT_TRIGGER = 3,
  LVKW_CTRL_HAPTIC_STANDARD_COUNT = 4
} LVKW_CtrlHaptic;

LVKW_COLD LVKW_Status lvkw_input_createController(LVKW_ControllerRef *controller_ref,
                                                   LVKW_Controller **out_controller);
LVKW_COLD LVKW_Status lvkw_input_destroyController(LVKW_Controller *controller);

LVKW_COLD LVKW_Status lvkw_input_getControllerInfo(LVKW_Controller *controller,
                                                    LVKW_CtrlInfo *out_info);

LVKW_COLD LVKW_Status lvkw_input_listControllers(LVKW_Context *context,
                                                  LVKW_ControllerRef **out_refs,
                                                  uint32_t *out_count);

LVKW_HOT LVKW_Status lvkw_input_setControllerHapticLevels(LVKW_Controller *controller,
                                                           uint32_t first_haptic, uint32_t count,
                                                           const LVKW_Scalar *intensities);

#endif  // LVKW_ENABLE_CONTROLLER

#ifdef __cplusplus
}
#endif

#endif  // LVKW_EXT_CONTROLLER_H_INCLUDED
