// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_MONITOR_H_INCLUDED
#define LVKW_MONITOR_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw-core.h"

/**
 * @file lvkw-monitor.h
 * @brief Monitor enumeration and display mode querying.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Runtime status flags for a monitor. */
typedef enum LVKW_MonitorFlags {
  LVKW_MONITOR_STATE_LOST = 1 << 0,  ///< Monitor was disconnected. Handle is
                                     ///< still valid but data is stale.
} LVKW_MonitorFlags;

/** @brief A video mode supported by a monitor. */
typedef struct LVKW_VideoMode {
  LVKW_PixelVec size;         ///< Resolution in physical pixels.
  uint32_t refresh_rate_mhz;  ///< Refresh rate in millihertz (e.g., 60000 for 60Hz).
} LVKW_VideoMode;

/**
 * @brief Persistent handle representing a physical monitor.
 * @note **Lifetime:** Managed by the context. Handles remain valid until
 * context destruction.
 *
 * ### Threading At A Glance
 * - Monitor query APIs are callable from any thread.
 * - Caller must externally synchronize against context event pumping
 *   (@ref lvkw_ctx_syncEvents) and context destruction.
 */
typedef struct LVKW_Monitor {
  void *userdata;  ///< User-controlled pointer.
  uint32_t flags;  ///< READ ONLY: Bitmask of @ref LVKW_MonitorFlags.

  const char *name;                  ///< READ ONLY: Human-readable name.
  LVKW_LogicalVec physical_size;     ///< READ ONLY: Dimensions in millimeters.
  LVKW_VideoMode current_mode;       ///< READ ONLY: Currently active mode.
  LVKW_LogicalVec logical_position;  ///< READ ONLY: Top-left corner in global
                                     ///< logical coordinates.
  LVKW_LogicalVec logical_size;      ///< READ ONLY: Size in logical coordinates
                                     ///< (affected by @p scale).
  bool is_primary;                   ///< READ ONLY: True if this is the system's primary/default
                                     ///< monitor.
  LVKW_real_t scale;                 ///< READ ONLY: Content scaling factor (DPI).
} LVKW_Monitor;

/* ----- Monitor Querying ----- */

/**
 * @brief Enumerates available (connected) monitors.
 * @note Follows the two-call pattern: call with @p out_monitors as NULL to
 * retrieve @p count, then call again with a sufficiently sized buffer.
 * @param ctx_handle Active context.
 * @param[out] out_monitors Array to fill with monitor handles. Can be NULL.
 * @param[in,out] count Pointer to capacity (in) and actual number of active
 * monitors (out).
 * @note Threading: callable from any thread with external synchronization
 * against @ref lvkw_ctx_syncEvents and @ref lvkw_ctx_destroy.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx_handle, LVKW_Monitor **out_monitors,
                                           uint32_t *count);

/**
 * @brief Enumerates supported video modes for a specific monitor.
 * @note Follows the two-call pattern.
 * @param ctx_handle Active context.
 * @param monitor Handle of the monitor to query.
 * @param[out] out_modes Array to fill with video modes. Can be NULL.
 * @param[in,out] count Pointer to capacity (in) and actual number of modes
 * (out).
 * @note Threading: callable from any thread with external synchronization
 * against @ref lvkw_ctx_syncEvents and @ref lvkw_ctx_destroy.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx_handle,
                                               const LVKW_Monitor *monitor,
                                               LVKW_VideoMode *out_modes, uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_MONITOR_H_INCLUDED
