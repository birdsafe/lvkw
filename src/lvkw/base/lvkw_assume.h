// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_BASE_ASSUME_H_INCLUDED
#define LVKW_BASE_ASSUME_H_INCLUDED

#include "lvkw_diagnostic_internal.h"

#ifdef LVKW_ENABLE_INTERNAL_CHECKS

#define LVKW_CTX_ASSUME(ctx, cond, msg)                                             \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_INTERNAL, msg);               \
    }                                                                               \
  } while (0)
#define LVKW_WND_ASSUME(wnd, cond, msg)                                              \
  do {                                                                               \
    if (!(cond)) {                                                                   \
      LVKW_REPORT_WIND_DIAGNOSTIC(wnd, LVKW_DIAGNOSTIC_INTERNAL, msg);               \
    }                                                                                \
  } while (0)

#else
#define LVKW_CTX_ASSUME(ctx, cond, msg) (void)0
#define LVKW_WND_ASSUME(wnd, cond, msg) (void)0
#endif

#endif
