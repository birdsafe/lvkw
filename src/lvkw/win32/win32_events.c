// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_win32_internal.h"

LVKW_Status lvkw_ctx_pollEvents_Win32(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata) {
  (void)ctx_handle;
  (void)event_mask;
  (void)callback;
  (void)userdata;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_waitEvents_Win32(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                      LVKW_EventType event_mask, LVKW_EventCallback callback,
                                      void *userdata) {
  (void)ctx_handle;
  (void)timeout_ms;
  (void)event_mask;
  (void)callback;
  (void)userdata;
  return LVKW_ERROR;
}
