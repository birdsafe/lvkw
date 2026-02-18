// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CORE_H_INCLUDED
#define LVKW_CORE_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "lvkw/details/lvkw_config.h"
#include "lvkw/details/lvkw_details.h"

/**
 * @file core.h
 * @brief Core types, versioning, and memory management.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Semantic version info. */
typedef struct LVKW_Version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
} LVKW_Version;

/** @brief Retrieves the linked version of the LVKW library. */
LVKW_COLD LVKW_Version lvkw_core_getVersion(void);

/** @brief Function pointer for memory allocation. */
typedef void *(*LVKW_AllocationFunction)(size_t size, void *userdata);

/** @brief Function pointer for resizing a memory allocation. */
typedef void *(*LVKW_ReallocationFunction)(void *ptr, size_t new_size, void *userdata);

/** @brief Function pointer for memory deallocation. */
typedef void (*LVKW_FreeFunction)(void *ptr, void *userdata);

/** @brief Custom memory allocator configuration. */
typedef struct LVKW_Allocator {
  LVKW_AllocationFunction alloc_cb;
  LVKW_ReallocationFunction realloc_cb;
  LVKW_FreeFunction free_cb;
  void *userdata;
} LVKW_Allocator;

/** @brief Common return codes for LVKW functions. */
typedef enum LVKW_Status {
  LVKW_SUCCESS = 0,
  LVKW_ERROR = -1,
  LVKW_ERROR_INVALID_USAGE = -2,
  LVKW_ERROR_WINDOW_LOST = -3,
  LVKW_ERROR_CONTEXT_LOST = -4,
} LVKW_Status;

/** @brief Bitmask for filtering events during polling or waiting. */
typedef enum LVKW_EventType {
  LVKW_EVENT_TYPE_ALL = (int)0xFFFFFFFF,

  LVKW_EVENT_TYPE_CLOSE_REQUESTED = 1 << 0,
  LVKW_EVENT_TYPE_WINDOW_RESIZED = 1 << 1,
  LVKW_EVENT_TYPE_KEY = 1 << 2,
  LVKW_EVENT_TYPE_WINDOW_READY = 1 << 3,
  LVKW_EVENT_TYPE_MOUSE_MOTION = 1 << 4,
  LVKW_EVENT_TYPE_MOUSE_BUTTON = 1 << 5,
  LVKW_EVENT_TYPE_MOUSE_SCROLL = 1 << 6,
  LVKW_EVENT_TYPE_IDLE_STATE_CHANGED = 1 << 7,
  LVKW_EVENT_TYPE_MONITOR_CONNECTION = 1 << 8,
  LVKW_EVENT_TYPE_MONITOR_MODE = 1 << 9,
  LVKW_EVENT_TYPE_TEXT_INPUT = 1 << 11,
  LVKW_EVENT_TYPE_FOCUS = 1 << 12,
  LVKW_EVENT_TYPE_WINDOW_MAXIMIZED = 1 << 13,
  LVKW_EVENT_TYPE_DND_HOVER = 1 << 14,
  LVKW_EVENT_TYPE_DND_LEAVE = 1 << 15,
  LVKW_EVENT_TYPE_DND_DROP = 1 << 16,
  LVKW_EVENT_TYPE_TEXT_COMPOSITION = 1 << 17,
  LVKW_EVENT_TYPE_DATA_READY = 1 << 18,

  LVKW_EVENT_TYPE_CONTROLLER_CONNECTION = 1 << 27,

  LVKW_EVENT_TYPE_USER_0 = 1 << 28,
  LVKW_EVENT_TYPE_USER_1 = 1 << 29,
  LVKW_EVENT_TYPE_USER_2 = 1 << 30,
  LVKW_EVENT_TYPE_USER_3 = (int)(1u << 31),
} LVKW_EventType;

#ifdef LVKW_USE_FLOAT
typedef float LVKW_Scalar;
#else
typedef double LVKW_Scalar;
#endif

/** @brief Represents a simple fraction or aspect ratio. */
typedef struct LVKW_Fraction {
  int32_t numerator;
  int32_t denominator;
} LVKW_Fraction;

/** @brief 2D vector in physical pixel coordinates. */
typedef struct LVKW_PixelVec {
  int32_t x;
  int32_t y;
} LVKW_PixelVec;

/** @brief 2D vector in logical coordinates. */
typedef struct LVKW_LogicalVec {
  LVKW_Scalar x;
  LVKW_Scalar y;
} LVKW_LogicalVec;

/** @brief Represents a rectangular area in logical coordinates. */
typedef struct LVKW_LogicalRect {
  LVKW_LogicalVec origin;
  LVKW_LogicalVec size;
} LVKW_LogicalRect;

/** @brief Forward declaration of the context handle. */
typedef struct LVKW_Context LVKW_Context;

/** @brief Forward declaration of the window handle. */
typedef struct LVKW_Window LVKW_Window;

#ifdef __cplusplus
}
#endif

#endif  // LVKW_CORE_H_INCLUDED
