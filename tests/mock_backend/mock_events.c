// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <string.h>

#include "lvkw_api_constraints.h"
#include "lvkw_mock.h"
#include "lvkw_mock_internal.h"

#include <stdio.h>

void lvkw_mock_pushEvent(LVKW_Context *handle, LVKW_EventType type, LVKW_Window* window, const LVKW_Event *evt) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;

  if (!((uint32_t)ctx->base.prv.event_mask & (uint32_t)type)) {
    return;
  }

  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, type, window, evt);
}

LVKW_Monitor* lvkw_mock_addMonitor(LVKW_Context *handle, const char* name, LVKW_LogicalVec logical_size) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;
  
  LVKW_Monitor_Mock *m = (LVKW_Monitor_Mock *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Monitor_Mock));
  if (!m) return NULL;

  memset(m, 0, sizeof(LVKW_Monitor_Mock));
  m->base.prv.ctx_base = &ctx->base;
  m->base.pub.name = _lvkw_string_cache_intern(&ctx->base.prv.string_cache, &ctx->base, name);
  m->base.pub.logical_size = logical_size;
  m->base.pub.scale = 1.0;
  m->base.pub.is_primary = (ctx->base.prv.monitor_list == NULL);

  // Add to monitor list
  m->base.prv.next = ctx->base.prv.monitor_list;
  ctx->base.prv.monitor_list = &m->base;

  /* Push a connection event */
  LVKW_Event evt = {0};
  evt.monitor_connection.monitor = &m->base.pub;
  evt.monitor_connection.connected = true;
  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, LVKW_EVENT_TYPE_MONITOR_CONNECTION, NULL, &evt);

  return &m->base.pub;
}

void lvkw_mock_removeMonitor(LVKW_Context *handle, LVKW_Monitor *monitor) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;
  LVKW_Monitor_Base *m_base = (LVKW_Monitor_Base *)monitor;

  m_base->pub.flags |= LVKW_MONITOR_STATE_LOST;

  /* Push a disconnection event */
  LVKW_Event evt = {0};
  evt.monitor_connection.monitor = monitor;
  evt.monitor_connection.connected = false;
  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, LVKW_EVENT_TYPE_MONITOR_CONNECTION, NULL, &evt);
}

void lvkw_mock_addMonitorMode(LVKW_Context *handle, LVKW_Monitor *monitor, LVKW_VideoMode mode) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;
  LVKW_Monitor_Mock *m_mock = (LVKW_Monitor_Mock *)monitor;

  if (m_mock->mode_count >= LVKW_MOCK_MAX_MODES_PER_MONITOR) return;

  m_mock->modes[m_mock->mode_count++] = mode;

  /* Push a mode event */
  LVKW_Event evt = {0};
  evt.monitor_mode.monitor = monitor;
  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, LVKW_EVENT_TYPE_MONITOR_MODE, NULL, &evt);
}
