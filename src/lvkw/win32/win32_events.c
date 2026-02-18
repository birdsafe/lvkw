// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "win32_internal.h"

LVKW_Status lvkw_ctx_pumpEvents_Win32(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;
  
  if (ctx->base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  _lvkw_notification_ring_dispatch_all(&ctx->base);

  MSG msg;
  if (timeout_ms == 0) {
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  } else {
    // Basic implementation of timeout for Win32 pump
    uint64_t start = _lvkw_get_timestamp_ms();
    for (;;) {
      if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      } else {
        if (timeout_ms != LVKW_NEVER) {
          uint64_t now = _lvkw_get_timestamp_ms();
          if (now - start >= timeout_ms) break;
          MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD)(timeout_ms - (now - start)), QS_ALLINPUT);
        } else {
          WaitMessage();
        }
      }
    }
  }

  _lvkw_notification_ring_dispatch_all(&ctx->base);

  LVKW_Event sync_evt = {0};
  _lvkw_dispatch_event(&ctx->base, LVKW_EVENT_TYPE_SYNC, NULL, &sync_evt);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_Win32(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                     const LVKW_Event *evt) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;
  if (!_lvkw_notification_ring_push(&ctx->base.prv.external_notifications, type, window, evt)) {
    return LVKW_ERROR;
  }
  // Wake up Win32 event loop
  PostMessageW(NULL, WM_NULL, 0, 0);
  return LVKW_SUCCESS;
}
