#ifndef LVKW_DIAG_INTERNAL_H_INCLUDED
#define LVKW_DIAG_INTERNAL_H_INCLUDED

#include <stdlib.h>

#include "lvkw_types_internal.h"

// LVKW_ENABLE_DEBUG_DIAGNOSIS implies LVKW_ENABLE_DIAGNOSIS
#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
#ifndef LVKW_ENABLE_DIAGNOSIS
#define LVKW_ENABLE_DIAGNOSIS
#endif
#endif

// Diagnosis Management
#ifdef LVKW_ENABLE_DIAGNOSIS
/* Diagnosis reporting helpers */
void _lvkw_report_bootstrap_diagnosis_internal(const LVKW_ContextCreateInfo *create_info, LVKW_Diagnosis diagnosis,
                                               const char *message);

#define LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, diagnosis, msg) \
  _lvkw_report_bootstrap_diagnosis_internal(create_info, diagnosis, msg)

#define LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, diagnosis, msg) \
  _lvkw_reportDiagnosis((LVKW_Context *)(ctx_base), NULL, (diagnosis), (msg))

#define LVKW_REPORT_WIND_DIAGNOSIS(window_base, diagnosis, msg)                                         \
  _lvkw_reportDiagnosis(                                                                                \
      (window_base) ? (LVKW_Context *)(((const LVKW_Window_Base *)(window_base))->prv.ctx_base) : NULL, \
      (LVKW_Window *)(window_base), (diagnosis), (msg))

#else
#define LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, diagnosis, msg) ((void)0)
#define LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, diagnosis, msg) ((void)0)
#define LVKW_REPORT_WIND_DIAGNOSIS(window_base, diagnosis, msg) ((void)0)
#endif

#ifdef LVKW_ENABLE_DEBUG_DIAGNOSIS
#define LVKW_CTX_ASSERT_THREAD_AFFINITY(ctx_base)                                                          \
  if (!thrd_equal(thrd_current(), ((const LVKW_Context_Base *)(ctx_base))->prv.creator_thread)) {          \
    LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, "Thread affinity violation"); \
    abort();                                                                                               \
  }

#define _lvkw_debug_ctx_check(ctx_base, cond, diagnosis, msg)                      \
  do {                                                                             \
    LVKW_CTX_ASSERT_THREAD_AFFINITY(ctx_base);                                     \
    if (!(cond)) {                                                                 \
      _lvkw_reportDiagnosis((LVKW_Context *)(ctx_base), NULL, (diagnosis), (msg)); \
      abort();                                                                     \
    }                                                                              \
  } while (0)

#define _lvkw_debug_wind_check(window_base, cond, diagnosis, msg)                               \
  do {                                                                                          \
    if (window_base) {                                                                          \
      LVKW_CTX_ASSERT_THREAD_AFFINITY(((const LVKW_Window_Base *)(window_base))->prv.ctx_base); \
    }                                                                                           \
    if (!(cond)) {                                                                              \
      LVKW_REPORT_WIND_DIAGNOSIS(window_base, diagnosis, msg);                                  \
      abort();                                                                                  \
    }                                                                                           \
  } while (0)
#else
#define LVKW_CTX_ASSERT_THREAD_AFFINITY(ctx_base) ((void)0)
#define _lvkw_debug_ctx_check(ctx_base, cond, diagnosis, msg) ((void)0)
#define _lvkw_debug_wind_check(window_base, cond, diagnosis, msg) ((void)0)
#endif

// Assertion Macros
#define LVKW_CTX_ASSERT_ARG(ctx_base, cond, msg) \
  _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg)
#define LVKW_WND_ASSERT_ARG(window_base, cond, msg) \
  _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSIS_INVALID_ARGUMENT, msg)

#define LVKW_CTX_ASSERT_PRECONDITION(ctx_base, cond, msg) \
  _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg)
#define LVKW_WND_ASSERT_PRECONDITION(window_base, cond, msg) \
  _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSIS_PRECONDITION_FAILURE, msg)

#define LVKW_CTX_ASSUME(ctx_base, cond, msg) _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSIS_INTERNAL, msg)
#define LVKW_WND_ASSUME(window_base, cond, msg) _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSIS_INTERNAL, msg)

#endif  // LVKW_DIAG_INTERNAL_H_INCLUDED