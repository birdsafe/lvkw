// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_DIAGNOSTIC_INTERNAL_H_INCLUDED
#define LVKW_DIAGNOSTIC_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"

#ifdef __cplusplus
extern "C" {
#endif
// Diagnostics Management
#ifdef LVKW_ENABLE_DIAGNOSTICS

void _lvkw_reportDiagnostic(LVKW_Context *ctx_handle, LVKW_Window *window_handle,
                            LVKW_Diagnostic diagnostic, const char *message);

/* Diagnostic reporting helpers */
void _lvkw_report_bootstrap_diagnostic_internal(const LVKW_ContextCreateInfo *create_info,
                                                LVKW_Diagnostic diagnostic, const char *message);

#define LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, diagnostic, msg) \
  _lvkw_report_bootstrap_diagnostic_internal(create_info, diagnostic, msg)

#define LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, diagnostic, msg) \
  _lvkw_reportDiagnostic((LVKW_Context *)(ctx_base), NULL, (diagnostic), (msg))

#define LVKW_REPORT_WIND_DIAGNOSTIC(window_base, diagnostic, msg)                               \
  _lvkw_reportDiagnostic(                                                                       \
      (window_base) ? (LVKW_Context *)(((const LVKW_Window_Base *)(window_base))->prv.ctx_base) \
                    : NULL,                                                                     \
      (LVKW_Window *)(window_base), (diagnostic), (msg))

#else
#define LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, diagnostic, msg) ((void)0)
#define LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, diagnostic, msg) ((void)0)
#define LVKW_REPORT_WIND_DIAGNOSTIC(window_base, diagnostic, msg) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif  // LVKW_DIAGNOSTIC_INTERNAL_H_INCLUDED
