// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_DETAILS_H_INCLUDED
#define LVKW_DETAILS_H_INCLUDED

// Vulkan forward declarations, so that include order doesn't matter for users
// of the library.
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;
typedef struct VkInstance_T *VkInstance;

/* --- Optimization and Documentation Hints --- */

// Realistically, the compiler flags don't do much for us here and they are included in a bit of a
// "might as well" fashion. The real role of these macros is to communicate to the user how we are
// expecting the API to be used.
#if defined(__GNUC__) || defined(__clang__)
#define LVKW_HOT __attribute__((hot))
#define LVKW_COLD __attribute__((cold))
#else
#define LVKW_HOT
#define LVKW_COLD
#endif

// Tag for struct fields that are backed by the double-buffered transient pools.
// Such pointers are only valid until the next call to lvkw_events_commit().
#define LVKW_TRANSIENT

#endif  // LVKW_DETAILS_H_INCLUDED
