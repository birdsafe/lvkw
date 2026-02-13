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

/** @brief Unique identifier for a physical monitor. Persistent for the lifetime of the context. */
typedef uint32_t LVKW_MonitorId;

/** @brief Invalid monitor identifier. */
#define LVKW_MONITOR_ID_INVALID 0

/** @brief A video mode supported by a monitor. */
typedef struct LVKW_VideoMode {
  LVKW_PixelVec size;         ///< Resolution in physical pixels.
  uint32_t refresh_rate_mhz;  ///< Refresh rate in millihertz (e.g., 60000 for 60Hz).
} LVKW_VideoMode;

/** @brief Snapshot of a monitor's current state and capabilities. */
typedef struct LVKW_MonitorInfo {
  LVKW_MonitorId id;                 ///< Handle for querying modes or targeting windows.
  const char *name;                  ///< Human-readable name. Valid until context destruction.
  LVKW_LogicalVec physical_size;     ///< Dimensions of the display area in millimeters.
  LVKW_VideoMode current_mode;       ///< The mode currently active on the OS desktop.
  LVKW_LogicalVec logical_position;  ///< Top-left corner in global logical coordinates.
  LVKW_LogicalVec logical_size;      ///< Size in logical coordinates (affected by @p scale).
  bool is_primary;                   ///< True if this is the system's primary/default monitor.
  LVKW_real_t scale;                      ///< Content scaling factor (DPI).
} LVKW_MonitorInfo;

/* ----- Monitor Querying ----- */

/**
 * @brief Enumerates available monitors.
 * @note Follows the two-call pattern: call with @p out_monitors as NULL to retrieve @p count,
 * then call again with a sufficiently sized buffer.
 * @param ctx_handle Active context.
 * @param[out] out_monitors Array to fill with monitor info snapshots. Can be NULL.
 * @param[in,out] count Pointer to capacity (in) and actual number of monitors (out).
 * @return LVKW_SUCCESS or LVKW_ERROR.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx_handle, LVKW_MonitorInfo *out_monitors, uint32_t *count);

/**
 * @brief Enumerates supported video modes for a specific monitor.
 * @note Follows the two-call pattern.
 * @note Some OSs only report the current active mode, in which case this function will return a single mode matching
 * the current_mode field of the monitor info.
 * @param ctx_handle Active context.
 * @param monitor ID of the monitor to query.
 * @param[out] out_modes Array to fill with video modes. Can be NULL.
 * @param[in,out] count Pointer to capacity (in) and actual number of modes (out).
 * @return LVKW_SUCCESS or LVKW_ERROR.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx_handle, LVKW_MonitorId monitor,
                                               LVKW_VideoMode *out_modes, uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_MONITOR_H_INCLUDED
