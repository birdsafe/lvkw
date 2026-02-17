// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "win32_internal.h"

LVKW_Status lvkw_ctx_pumpEvents_Win32(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;
  (void)timeout_ms;
  // TODO: Implement Win32 event gathering
  (void)ctx;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_commitEvents_Win32(LVKW_Context *ctx_handle) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;
  lvkw_event_queue_begin_gather(&ctx->base.prv.event_queue);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_Win32(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                     const LVKW_Event *evt) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;
  LVKW_Event empty_evt = {0};
  if (!evt) evt = &empty_evt;

  if (!lvkw_event_queue_push_external(&ctx->base.prv.event_queue, type, window, evt)) {
    return LVKW_ERROR;
  }
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_Win32(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata) {
