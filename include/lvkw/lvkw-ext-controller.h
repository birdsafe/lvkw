#ifndef LVKW_CONTROLLER_H
#define LVKW_CONTROLLER_H

#include <stdint.h>

#include "lvkw-input.h"

/**
 * @file lvkw-ext-controller.h
 * @brief Game controller and gamepad support extension.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LVKW_CONTROLLER_ENABLED

/** @brief Opaque identifier for a physical controller device. */
typedef uint32_t LVKW_CtrlId;

/** @brief Detailed hardware information for a controller. */
typedef struct LVKW_CtrlInfo {
  const char *name;      ///< Human-readable name. Valid until context destruction.
  uint16_t vendor_id;    ///< USB/Bluetooth vendor ID.
  uint16_t product_id;   ///< USB/Bluetooth product ID.
  uint16_t version;      ///< Hardware version or revision.
  uint8_t guid[16];      ///< Platform-specific unique identifier (often compatible with SDL mappings).
  bool is_standardized;  ///< True if the controller follows the standard layout defined by LVKW_CtrlButton.
} LVKW_CtrlInfo;

/** @brief Event data for controller hotplugging. */
typedef struct LVKW_CtrlConnectionEvent {
  LVKW_CtrlId id;  ///< Identifier for the controller.
  bool connected;  ///< True if the controller was plugged in, false if removed.
} LVKW_CtrlConnectionEvent;

/**
 * @brief State and metadata for an active controller connection.
 */
typedef struct LVKW_Controller {
  void *userdata;  ///< User-controlled pointer.
  uint32_t flags;  ///< READ ONLY: Bitmask of status flags. (e.g. LVKW_CTX_STATE_LOST).

  const LVKW_AnalogInputState *analogs;  ///< READ ONLY: Array of analog axes. Indices 0-5 follow LVKW_CtrlAnalog.
  uint32_t analog_count;                 ///< READ ONLY: Total number of analog axes available.
  const LVKW_ButtonState *buttons;       ///< READ ONLY: Array of digital buttons. Indices 0-14 follow LVKW_CtrlButton.
  uint32_t button_count;                 ///< READ ONLY: Total number of buttons available.

  uint32_t motor_count;  ///< READ ONLY: Total number of haptic motors available.
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

/** @brief Standardized button indices (following a common gamepad layout). */
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

/** @brief Standardized haptic motor indices. */
typedef enum LVKW_CtrlMotor {
  LVKW_CTRL_MOTOR_LOW_FREQ = 0,       ///< Large vibration motor (typically left).
  LVKW_CTRL_MOTOR_HIGH_FREQ = 1,      ///< Small vibration motor (typically right).
  LVKW_CTRL_MOTOR_LEFT_TRIGGER = 2,   ///< Independent left trigger motor.
  LVKW_CTRL_MOTOR_RIGHT_TRIGGER = 3,  ///< Independent right trigger motor.
  LVKW_CTRL_MOTOR_STANDARD_COUNT = 4
} LVKW_CtrlMotor;

/**
 * @brief Opens a controller for use.
 * @note **Thread Affinity:** Must be called on the context's main thread.
 * @param ctx Active context.
 * @param id Controller identifier from a connection event.
 * @param[out] out_controller Receives the handle to the opened controller.
 */
LVKW_COLD LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller);

/**
 * @brief Closes a controller and frees associated resources.
 * @note **Thread Affinity:** Must be called on the context's main thread.
 * @param controller Controller handle to destroy.
 */
LVKW_COLD LVKW_Status lvkw_ctrl_destroy(LVKW_Controller *controller);

/**
 * @brief Retrieves hardware identification for the controller.
 * @param controller Active controller handle.
 * @param[out] out_info Receives the hardware information.
 */
LVKW_COLD LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info);

/**
 * @brief Sets the vibration intensities for a range of motors.
 *
 * Motors are treated as output channels. Indices 0-3 follow @ref LVKW_CtrlMotor
 * for standardized controllers.
 *
 * @note **Thread Affinity:** Must be called on the context's main thread.
 * @param controller Active controller handle.
 * @param first_motor Index of the first motor to update.
 * @param count Number of motor levels provided in the intensities array.
 * @param intensities Array of normalized values [0.0, 1.0].
 */
LVKW_COLD LVKW_Status lvkw_ctrl_setMotorLevels(LVKW_Controller *controller, uint32_t first_motor, uint32_t count,
                                               const LVKW_real_t *intensities);

#endif /* LVKW_CONTROLLER_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* LVKW_CONTROLLER_H */
