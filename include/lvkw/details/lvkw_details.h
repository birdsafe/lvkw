// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_DETAILS_H_INCLUDED
#define LVKW_DETAILS_H_INCLUDED

// Vulkan forward declarations, so that include order doesn't matter for users of the library.
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;
typedef struct VkInstance_T *VkInstance;

/* --- Optimization and Documentation Hints --- */

// TODO: play around with __attribute__((hot|cold)) and see what kind of impact it has.
/** Marker for functions in the performance-critical "hot path". */
#define LVKW_HOT

/** Marker for initialization or infrequent "cold path" functions. */
#define LVKW_COLD

#endif  // LVKW_DETAILS_H_INCLUDED
