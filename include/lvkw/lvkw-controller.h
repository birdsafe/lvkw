#ifndef LVKW_CONTROLLER_H
#define LVKW_CONTROLLER_H

#include <stdint.h>

#include "lvkw-core.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LVKW_CONTROLLER_ENABLED
typedef enum LVKW_CtrlAnalog {
  LVKW_CTRL_ANALOG_LEFT_X = 0,
  LVKW_CTRL_ANALOG_LEFT_Y = 1,
  LVKW_CTRL_ANALOG_RIGHT_X = 2,
  LVKW_CTRL_ANALOG_RIGHT_Y = 3,
  LVKW_CTRL_ANALOG_LEFT_TRIGGER = 4,
  LVKW_CTRL_ANALOG_RIGHT_TRIGGER = 5,
  LVKW_CTRL_ANALOG_STANDARD_COUNT = 6
} LVKW_CtrlAnalog;

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

typedef struct LVKW_Controller LVKW_Controller;
typedef uint32_t LVKW_CtrlId;

typedef struct LVKW_AnalogInputState {
  float value; /**< Normalized: [-1.0, 1.0] for sticks, [0.0, 1.0] for triggers. */
} LVKW_AnalogInputState;

/** @brief Detailed information about a controller hardware device. */
typedef struct LVKW_CtrlInfo {
  const char *name;     /**< Human-readable name. Valid until context destruction. */
  uint16_t vendor_id;   /**< USB/Bluetooth Vendor ID. */
  uint16_t product_id;  /**< USB/Bluetooth Product ID. */
  uint16_t version;     /**< Device version. */
  uint8_t guid[16];     /**< Platform-specific persistent identifier for mapping databases. */
  bool is_standardized; /**< True if the controller follows the Standardized Mapping (Gamepad). */
} LVKW_CtrlInfo;

/** @brief Event data for controller connection changes. */
typedef struct LVKW_CtrlConnectionEvent {
  LVKW_CtrlId id; /**< Temporary ID for creating the controller handle. */
  bool connected; /**< True if plugged in, false if removed. */
} LVKW_CtrlConnectionEvent;

struct LVKW_Controller {
  void *userdata;
  const LVKW_AnalogInputState *analogs; /**< Size: analog_count. Indices 0-5 standardized. */
  uint32_t analog_count;
  const LVKW_ButtonState *buttons; /**< Size: button_count. Indices 0-14 standardized. */
  uint32_t button_count;
  uint32_t flags; /**< LVKW_CTRL_STATE_LOST if unplugged. */
};

LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller);
void lvkw_ctrl_destroy(LVKW_Controller *controller);
LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info);

#endif /* LVKW_CONTROLLER_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* LVKW_CONTROLLER_H */
