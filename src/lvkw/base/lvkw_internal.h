// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_INTERNAL_H_INCLUDED
#define LVKW_INTERNAL_H_INCLUDED

/*
 * This is the umbrella header for all internal LVKW definitions.
 * It is split into specialized headers to improve maintainability.
 */

#include "lvkw_backend.h"
#include "lvkw_diagnostic_internal.h"
#include "lvkw_mem_internal.h"
#include "lvkw_types_internal.h"

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info,
                                     LVKW_Context **out_context);

uint64_t _lvkw_get_timestamp_ms(void);

#endif  // LVKW_INTERNAL_H_INCLUDED
