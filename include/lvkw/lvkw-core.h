#ifndef LVKW_LIBRARY_CORE_H_INCLUDED
#define LVKW_LIBRARY_CORE_H_INCLUDED

#include <stdint.h>

#include "lvkw/details/lvkw_details.h"
#include "lvkw/details/lvkw_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief An opaque handle to the library's main context. */
typedef struct LVKW_Context LVKW_Context;
/** @brief An opaque handle to a window instance. */
typedef struct LVKW_Window LVKW_Window;

/** @brief Tracks whether a button is currently pressed or released. */
typedef enum LVKW_ButtonState {
  LVKW_BUTTON_STATE_RELEASED = 0,
  LVKW_BUTTON_STATE_PRESSED = 1,
} LVKW_ButtonState;

/** @brief Status codes returned by the API. */
typedef enum LVKW_Status {
  /** @brief The operation succeeded. */
  LVKW_SUCCESS = 0,
  /** @brief The operation failed, but your handles are still safe. */
  LVKW_ERROR = 1,
  /** @brief The operation failed and the window handle is now dead. */
  LVKW_ERROR_WINDOW_LOST = 2,
  /** @brief The operation failed and the entire context is dead. */
  LVKW_ERROR_CONTEXT_LOST = 3,
} LVKW_Status;

typedef uint32_t LVKW_MonitorId;
#define LVKW_MONITOR_ID_INVALID 0

#ifdef __cplusplus
}
#endif

#endif