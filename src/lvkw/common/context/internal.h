// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_INTERNAL_H_INCLUDED
#define LVKW_INTERNAL_H_INCLUDED

/*
 * This is the umbrella header for all internal LVKW definitions.
 * It is split into specialized headers to improve maintainability.
 */

#include "backend.h"
#include "diagnostic_internal.h"
#include "mem_internal.h"
#include "types_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info,
                                     LVKW_Context **out_context);

uint64_t _lvkw_get_timestamp_ms(void);

void _lvkw_update_state_from_event(LVKW_EventType type, LVKW_Window *window, const LVKW_Event *evt,
                                   void *userdata);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_INTERNAL_H_INCLUDED
