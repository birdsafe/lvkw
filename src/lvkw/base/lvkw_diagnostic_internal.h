#ifndef LVKW_DIAGNOSTIC_INTERNAL_H_INCLUDED
#define LVKW_DIAGNOSTIC_INTERNAL_H_INCLUDED

#include <stdlib.h>

#include "lvkw_types_internal.h"
#include "lvkw_thread_internal.h"

// LVKW_ENABLE_DEBUG_DIAGNOSTICS implies LVKW_ENABLE_DIAGNOSTICS
#ifdef LVKW_ENABLE_DEBUG_DIAGNOSTICS
#ifndef LVKW_ENABLE_DIAGNOSTICS
#define LVKW_ENABLE_DIAGNOSTICS
#endif
#endif

// Diagnostics Management
#ifdef LVKW_ENABLE_DIAGNOSTICS
/* Diagnostic reporting helpers */
void _lvkw_report_bootstrap_diagnostic_internal(const LVKW_ContextCreateInfo *create_info, LVKW_Diagnostic diagnostic,
                                               const char *message);

#define LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, diagnostic, msg) \
  _lvkw_report_bootstrap_diagnostic_internal(create_info, diagnostic, msg)

#define LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, diagnostic, msg) \
  _lvkw_reportDiagnostic((LVKW_Context *)(ctx_base), NULL, (diagnostic), (msg))

#define LVKW_REPORT_WIND_DIAGNOSTIC(window_base, diagnostic, msg)                                         \
  _lvkw_reportDiagnostic(                                                                                \
      (window_base) ? (LVKW_Context *)(((const LVKW_Window_Base *)(window_base))->prv.ctx_base) : NULL, \
      (LVKW_Window *)(window_base), (diagnostic), (msg))

#else
#define LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, diagnostic, msg) ((void)0)
#define LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, diagnostic, msg) ((void)0)
#define LVKW_REPORT_WIND_DIAGNOSTIC(window_base, diagnostic, msg) ((void)0)
#endif

#ifdef LVKW_ENABLE_DEBUG_DIAGNOSTICS
#define LVKW_CTX_ASSERT_THREAD_AFFINITY(ctx_base)                                                                   \
  if (_lvkw_get_current_thread_id() != ((const LVKW_Context_Base *)(ctx_base))->prv.creator_thread) {               \
    LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE, "Thread affinity violation");          \
    abort();                                                                                                        \
  }

#define _lvkw_debug_ctx_check(ctx_base, cond, diagnostic, msg)                      \
  do {                                                                             \
    LVKW_CTX_ASSERT_THREAD_AFFINITY(ctx_base);                                     \
    if (!(cond)) {                                                                 \
      _lvkw_reportDiagnostic((LVKW_Context *)(ctx_base), NULL, (diagnostic), (msg)); \
      abort();                                                                     \
    }                                                                              \
  } while (0)

#define _lvkw_debug_wind_check(window_base, cond, diagnostic, msg)                               \
  do {                                                                                          \
    if (window_base) {                                                                          \
      LVKW_CTX_ASSERT_THREAD_AFFINITY(((const LVKW_Window_Base *)(window_base))->prv.ctx_base); \
    }                                                                                           \
    if (!(cond)) {                                                                              \
      LVKW_REPORT_WIND_DIAGNOSTIC(window_base, diagnostic, msg);                                  \
      abort();                                                                                  \
    }                                                                                           \
  } while (0)
#else
#define LVKW_CTX_ASSERT_THREAD_AFFINITY(ctx_base) ((void)0)
#define _lvkw_debug_ctx_check(ctx_base, cond, diagnostic, msg) ((void)0)
#define _lvkw_debug_wind_check(window_base, cond, diagnostic, msg) ((void)0)
#endif

// Assertion Macros
#define LVKW_CTX_ASSERT_ARG(ctx_base, cond, msg) \
  _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, msg)
#define LVKW_WND_ASSERT_ARG(window_base, cond, msg) \
  _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, msg)

#define LVKW_CTX_ASSERT_PRECONDITION(ctx_base, cond, msg) \
  _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE, msg)
#define LVKW_WND_ASSERT_PRECONDITION(window_base, cond, msg) \
  _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE, msg)

#define LVKW_CTX_ASSUME(ctx_base, cond, msg) _lvkw_debug_ctx_check(ctx_base, cond, LVKW_DIAGNOSTIC_INTERNAL, msg)
#define LVKW_WND_ASSUME(window_base, cond, msg) _lvkw_debug_wind_check(window_base, cond, LVKW_DIAGNOSTIC_INTERNAL, msg)

#endif  // LVKW_DIAGNOSTIC_INTERNAL_H_INCLUDED
