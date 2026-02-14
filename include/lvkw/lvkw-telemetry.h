// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_TELEMETRY_H_INCLUDED
#define LVKW_TELEMETRY_H_INCLUDED

#include <stdint.h>

#include "lvkw-core.h"

/**
 * @file lvkw-telemetry.h
 * @brief Telemetry and performance monitoring.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Categorization of telemetry data groups.
 */
typedef enum LVKW_TelemetryCategory {
  LVKW_TELEMETRY_CATEGORY_NONE = 0,
  LVKW_TELEMETRY_CATEGORY_EVENTS = 1,  ///< Returns LVKW_EventTelemetry snapshot.
} LVKW_TelemetryCategory;

/**
 * @brief Telemetry snapshot for the internal event queue.
 */
typedef struct LVKW_EventTelemetry {
  uint32_t peak_count;        ///< High watermark of events in queue since last reset.
  uint32_t current_capacity;  ///< Current number of allocated slots in the ring
                              ///< buffer.
  uint32_t drop_count;        ///< Total events dropped due to max_capacity since last
                              ///< reset.
} LVKW_EventTelemetry;

/**
 * @brief Retrieves a specific category of telemetry data from the context.
 *
 * @param ctx The context to query.
 * @param category The category of telemetry to retrieve.
 * @param out_data Pointer to a structure matching the category (e.g.,
 * LVKW_EventTelemetry).
 * @param reset If true, and the category supports it, counters/watermarks will
 * be reset after retrieval.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getTelemetry(LVKW_Context *ctx, LVKW_TelemetryCategory category,
                                            void *out_data, bool reset);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_TELEMETRY_H_INCLUDED
