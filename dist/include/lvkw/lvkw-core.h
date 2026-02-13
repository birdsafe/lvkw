#ifndef LVKW_LIBRARY_CORE_H_INCLUDED
#define LVKW_LIBRARY_CORE_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "lvkw/details/lvkw_details.h"

/**
 * @file lvkw-core.h
 * @brief Core types, versioning, and memory management.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ----- VERSIONING ----- */

/** @brief Semantic version info. */
typedef struct LVKW_Version {
  uint32_t major;  ///< Incremented for incompatible API changes.
  uint32_t minor;  ///< Incremented for backwards-compatible functionality.
  uint32_t patch;  ///< Incremented for backwards-compatible bug fixes.
} LVKW_Version;

/**
 * @brief Retrieves the linked version of the LVKW library.
 */
LVKW_COLD LVKW_Version lvkw_getVersion(void);

/* ----- MEMORY MANAGEMENT ----- */

/**
 * @brief Function pointer for memory allocation.
 * @param size Number of bytes to allocate.
 * @param userdata User-provided pointer from LVKW_ContextCreateInfo.
 * @return Pointer to the allocated memory, or NULL on failure.
 */
typedef void *(*LVKW_AllocationFunction)(size_t size, void *userdata);

/**
 * @brief Function pointer for memory deallocation.
 * @param ptr Pointer to the memory to free. If NULL, the function does nothing.
 * @param userdata User-provided pointer from LVKW_ContextCreateInfo.
 */
typedef void (*LVKW_FreeFunction)(void *ptr, void *userdata);

/**
 * @brief Custom memory allocator configuration.
 * @note If @p alloc_cb is provided, @p free_cb must also be provided.
 * If both are NULL, the library uses standard malloc/free.
 */
typedef struct LVKW_Allocator {
  LVKW_AllocationFunction alloc_cb;
  LVKW_FreeFunction free_cb;
} LVKW_Allocator;

/* ----- ERROR HANDLING ----- */

/** @brief Common return codes for LVKW functions. */
typedef enum LVKW_Status {
  LVKW_SUCCESS = 0,
  LVKW_ERROR = -1,  ///< The operation did not succeed, but the context (and window if applicable) are still valid.
  LVKW_ERROR_INVALID_USAGE = -2, ///< API misuse (NULL ptr, invalid enum, etc.).
  LVKW_ERROR_WINDOW_LOST = -3,   ///< The window is dead. All operations on it will fail and it should be destroyed.
  LVKW_ERROR_CONTEXT_LOST = -4,  ///< The context is dead. All operations on it will fail and it should be destroyed.
} LVKW_Status;

/* ----- ARITHMETIC TYPES ----- */

#ifdef LVKW_USE_FLOAT
typedef float LVKW_real_t;
#else
typedef double LVKW_real_t;
#endif

/**
 * @brief Represents a simple fraction or aspect ratio.
 * @note Used for aspect ratios.
 */
typedef struct LVKW_Ratio {
  int32_t numer;
  int32_t denom;
} LVKW_Ratio;

/**
 * @brief 2D vector in physical pixel coordinates.
 * @note Used for framebuffer sizes and precise surface areas.
 */
typedef struct LVKW_PixelVec {
  int32_t x;
  int32_t y;
} LVKW_PixelVec;

/**
 * @brief 2D vector in logical coordinates.
 * @note These coordinates are scaled by the OS DPI settings. Used for UI and positioning.
 */
typedef struct LVKW_LogicalVec {
  LVKW_real_t x;
  LVKW_real_t y;
} LVKW_LogicalVec;

/**
 * @brief Represents a rectangular area in logical coordinates.
 */
typedef struct LVKW_LogicalRect {
  LVKW_LogicalVec origin; ///< Top-left corner (x, y).
  LVKW_LogicalVec size;   ///< Dimensions (width, height).
} LVKW_LogicalRect;

/* ----- Main Handles ----- */

/** @brief Forward declaration of the context handle. See lvkw-context.h. */
typedef struct LVKW_Context LVKW_Context;

/** @brief Forward declaration of the window handle. See lvkw-window.h. */
typedef struct LVKW_Window LVKW_Window;

#ifdef __cplusplus
}
#endif

#endif  // LVKW_LIBRARY_CORE_H_INCLUDED