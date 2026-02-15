// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>

#include "dlib/X11.h"
#include "dlib/Xi.h"  // IWYU pragma: keep
#include "dlib/Xss.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_x11_internal.h"

#define LVKW_X11_MAX_EVENTS 4096

LVKW_Status lvkw_ctx_syncEvents_X11(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  // TODO: Implement X11 event gathering with timeout
  (void)timeout_ms;
  
  lvkw_event_queue_begin_gather(&ctx->base.prv.event_queue);
  return LVKW_SUCCESS;
}

void _lvkw_x11_push_event_cb(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                             const LVKW_Event *evt) {
  LVKW_Context_X11 *ctx_x11 = (LVKW_Context_X11 *)ctx;
  if (type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
      type == LVKW_EVENT_TYPE_WINDOW_RESIZED) {
    lvkw_event_queue_push_compressible(&ctx_x11->base, &ctx_x11->base.prv.event_queue, type, window, evt);
  } else {
    lvkw_event_queue_push(&ctx_x11->base, &ctx_x11->base.prv.event_queue, type, window, evt);
  }
}

LVKW_Status lvkw_ctx_postEvent_X11(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                   const LVKW_Event *evt) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  LVKW_Event empty_evt = {0};
  if (!evt) evt = &empty_evt;
  
  if (!lvkw_event_queue_push_external(&ctx->base.prv.event_queue, type, window, evt)) {
    return LVKW_ERROR;
  }
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_X11(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  lvkw_event_queue_scan(&ctx->base.prv.event_queue, event_mask, callback, userdata);
  return LVKW_SUCCESS;
}
