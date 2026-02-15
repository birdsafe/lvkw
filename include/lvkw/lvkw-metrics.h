// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_METRICS_H_INCLUDED
#define LVKW_METRICS_H_INCLUDED

#include <stdint.h>

#include "lvkw-core.h"

/**
 * @file lvkw-metrics.h
 * @brief Performance and system metrics monitoring.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Categorization of metrics data groups.
 */
typedef enum LVKW_MetricsCategory {
  LVKW_METRICS_CATEGORY_NONE = 0,
  LVKW_METRICS_CATEGORY_EVENTS = 1,  ///< Returns LVKW_EventMetrics snapshot.
} LVKW_MetricsCategory;

/**
 * @brief Metrics snapshot for the internal event queue.
 */
typedef struct LVKW_EventMetrics {
  uint32_t peak_count;        ///< High watermark of events in queue since last reset.
  uint32_t current_capacity;  ///< Current number of allocated slots in the ring
                              ///< buffer.
  uint32_t drop_count;        ///< Total events dropped due to max_capacity since last
                              ///< reset.
} LVKW_EventMetrics;

/**
 * @brief Retrieves a specific category of metrics data from the context.
 *
 * @param ctx The context to query.
 * @param category The category of metrics to retrieve.
 * @param out_data Pointer to a structure matching the category (e.g.,
 * LVKW_EventMetrics).
 * @param reset If true, and the category supports it, counters/watermarks will
 * be reset after retrieval.
 * @note Threading: callable from any thread with external synchronization
 * against @ref lvkw_ctx_syncEvents and @ref lvkw_ctx_destroy.
 */
LVKW_COLD LVKW_Status lvkw_ctx_getMetrics(LVKW_Context *ctx, LVKW_MetricsCategory category,
                                          void *out_data, bool reset);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_METRICS_H_INCLUDED
