// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_INSTRUMENTATION_H_INCLUDED
#define LVKW_INSTRUMENTATION_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw/c/core.h"

/**
 * @file instrumentation.h
 * @brief Error reporting and runtime metrics APIs.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Categorization of library-side errors and warnings.
 */
typedef enum LVKW_Diagnostic {
  LVKW_DIAGNOSTIC_NONE = 0,
  LVKW_DIAGNOSTIC_INFO,                ///< Informational message that does not indicate
                                       ///< an error condition.
  LVKW_DIAGNOSTIC_OUT_OF_MEMORY,         ///< The system or custom allocator failed to
                                         ///< provide memory.
  LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,  ///< OS-level resource could not be
                                         ///< obtained.
  LVKW_DIAGNOSTIC_DYNAMIC_LIB_FAILURE,   ///< Failed to load a required system
                                         ///< library
  LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,   ///< Something is not available on the
                                         ///< current backend or system.
  LVKW_DIAGNOSTIC_BACKEND_FAILURE,       ///< Something went wrong in the underbelly
                                         ///< of the library.
  LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE,   ///< The requested backend cannot be
                                         ///< created on this system.
  LVKW_DIAGNOSTIC_VULKAN_FAILURE,        ///< Something vulkan-related went wrong.
  LVKW_DIAGNOSTIC_UNKNOWN,               ///< An error occurred that does not fit into other
                                         ///< categories.

  /* Debug Diagnostics (Generally unrecoverable; may abort in Debug builds) */

  LVKW_DIAGNOSTIC_INVALID_ARGUMENT,      ///< An argument provided to a public API
                                         ///< call was invalid.
  LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,  ///< API state violation (e.g., calling
                                         ///< a method from the wrong thread).
  LVKW_DIAGNOSTIC_INTERNAL,              ///< An internal logic error or inconsistent state
                                         ///< was detected within LVKW.
} LVKW_Diagnostic;

/**
 * @brief Detailed information about a diagnostic event.
 */
typedef struct LVKW_DiagnosticInfo {
  LVKW_Diagnostic diagnostic;
  LVKW_TRANSIENT const char *message;
  LVKW_Context *context;  ///< NULL if not applicable.
  LVKW_Window *window;    ///< NULL if not applicable.
} LVKW_DiagnosticInfo;

/**
 * @brief Application-defined callback for receiving library diagnostics.
 * @param info Detailed information about the diagnostic event. Do NOT store
 * this pointer; its content is only valid during the call.
 * @param userdata The user-defined pointer provided when the callback was
 * registered.
 */
typedef void (*LVKW_DiagnosticCallback)(const LVKW_DiagnosticInfo *info, void *userdata);

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
 * @param context The context to query.
 * @param category The category of metrics to retrieve.
 * @param out_data Pointer to a structure matching the category (e.g.,
 * LVKW_EventMetrics).
 * @param reset If true, and the category supports it, counters/watermarks will
 * be reset after retrieval.
 * @note Threading: callable from any thread with external synchronization
 * against @ref lvkw_events_commit and @ref lvkw_context_destroy.
 */
LVKW_COLD LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *context,
                                                      LVKW_MetricsCategory category,
                                                      void *out_data, bool reset);

/**
 * @brief Convenience setter for context diagnostic callback attributes.
 */
LVKW_COLD LVKW_Status lvkw_instrumentation_setDiagnosticCallback(
    LVKW_Context *context, LVKW_DiagnosticCallback callback, void *userdata);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_INSTRUMENTATION_H_INCLUDED
