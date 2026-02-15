// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_TUNING_H_INCLUDED
#define LVKW_TUNING_H_INCLUDED

#include <stdint.h>

#include "lvkw-core.h"
/**
 * @file lvkw-tuning.h
 * @brief Low-level engine and backend tuning parameters.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Window decoration strategies for Wayland backends. */
typedef enum LVKW_WaylandDecorationMode {
  LVKW_WAYLAND_DECORATION_MODE_AUTO = 0,  ///< Try to use SSD, falls back to CSD
  LVKW_WAYLAND_DECORATION_MODE_SSD = 1,   ///< Force Server-Side Decorations
  LVKW_WAYLAND_DECORATION_MODE_CSD = 2,   ///< Force Client-Side Decorations via libdecor.
  LVKW_WAYLAND_DECORATION_MODE_NONE = 3,  ///< Disable all decorations.
} LVKW_WaylandDecorationMode;

/** @brief Generic function pointer for Vulkan return types. */
typedef void (*LVKW_VulkanVoidFunction)(void);

/** @brief Function pointer signature for vkGetInstanceProcAddr. */
typedef LVKW_VulkanVoidFunction (*LVKW_VkGetInstanceProcAddrFunc)(VkInstance instance,
                                                                  const char *pName);

/** @brief Parameters for the internal event queue. */
typedef struct LVKW_EventTuning {
  uint32_t initial_capacity;   ///< Initial number of event slots.
  uint32_t max_capacity;       ///< Hard limit for the primary queue.
  uint32_t external_capacity;  ///< Size of the thread-safe secondary buffer.
  LVKW_Scalar growth_factor;   ///< Multiplier for dynamic resizing.
} LVKW_EventTuning;

/** @brief Backend-specific and internal library tuning.
 *  @note Most applications should use LVKW_CONTEXT_TUNING_DEFAULT. */
typedef struct LVKW_ContextTuning {
  LVKW_EventTuning events;

  struct {
    LVKW_WaylandDecorationMode decoration_mode;
  } wayland;

  /**
   * @brief Optional override for the Vulkan loader entry point.
   *
   * If NULL (default), LVKW will use the symbol `vkGetInstanceProcAddr` from
   * the linked Vulkan library. If provided, LVKW will use this function pointer
   * instead. This is useful for custom loaders or when the Vulkan library is
   * not directly linked.
   */
  LVKW_VkGetInstanceProcAddrFunc vk_loader;
} LVKW_ContextTuning;

/**
 * @brief Default tuning parameters optimized for general-purpose applications.
 */
#define LVKW_CONTEXT_TUNING_DEFAULT                                                      \
  {.events =                                                                             \
       {.initial_capacity = 64, .max_capacity = 4096, .external_capacity = 64, .growth_factor = 2.0}, \
   .wayland = {.decoration_mode = LVKW_WAYLAND_DECORATION_MODE_AUTO},                    \
   .vk_loader = NULL}

#ifdef __cplusplus
}
#endif

#endif  // LVKW_TUNING_H_INCLUDED
