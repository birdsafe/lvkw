// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_INTERNAL_H_INCLUDED
#define LVKW_INTERNAL_H_INCLUDED

#include "backend.h"
#include "diagnostic_internal.h"
#include "mem_internal.h"
#include "types_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info,
                                     LVKW_Context **out_context);
LVKW_Status _lvkw_ctx_post_backend(LVKW_Context *ctx, LVKW_EventType type, LVKW_Window *window,
                                   const LVKW_Event *evt);

uint64_t _lvkw_get_timestamp_ms(void);

/**
 * @brief Dispatches an event to the user callback if allowed by the mask.
 * Also updates internal library state based on the event.
 */
void _lvkw_dispatch_event(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                          const LVKW_Event *evt);

/**
 * @brief Internal helper to push to the thread-safe notification ring.
 */
bool _lvkw_notification_ring_push(LVKW_EventNotificationRing *ring, LVKW_EventType type,
                                  LVKW_Window *window, const LVKW_Event *evt);

/**
 * @brief Internal helper to drain and dispatch all pending notifications.
 */
void _lvkw_notification_ring_dispatch_all(LVKW_Context_Base *ctx);

#ifdef __cplusplus
}
#endif

#endif  // LVKW_INTERNAL_H_INCLUDED
