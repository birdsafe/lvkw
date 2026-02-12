#include <string.h>

#include "lvkw_api_constraints.h"
#include "lvkw_mock.h"
#include "lvkw_mock_internal.h"

void lvkw_mock_pushEvent(LVKW_Context *handle, const LVKW_Event *evt) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;

  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, evt);
}

void lvkw_mock_addMonitor(LVKW_Context *handle, const LVKW_MonitorInfo *info) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;
  if (ctx->monitor_count >= LVKW_MOCK_MAX_MONITORS) return;

  /* Intern the name through the string cache */
  LVKW_MonitorInfo interned = *info;
  if (info->name) {
    interned.name = _lvkw_string_cache_intern(&ctx->base.prv.string_cache, &ctx->base, info->name);
  }

  ctx->monitors[ctx->monitor_count] = interned;
  ctx->monitor_mode_counts[ctx->monitor_count] = 0;
  ctx->monitor_count++;

  /* Push a connection event */
  LVKW_Event evt = {0};
  evt.type = LVKW_EVENT_TYPE_MONITOR_CONNECTION;
  evt.window = NULL;
  evt.monitor_connection.monitor = interned;
  evt.monitor_connection.connected = true;
  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, &evt);
}

void lvkw_mock_removeMonitor(LVKW_Context *handle, LVKW_MonitorId id) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;

  for (uint32_t i = 0; i < ctx->monitor_count; i++) {
    if (ctx->monitors[i].id == id) {
      LVKW_MonitorInfo removed = ctx->monitors[i];

      /* Shift remaining monitors down */
      for (uint32_t j = i; j < ctx->monitor_count - 1; j++) {
        ctx->monitors[j] = ctx->monitors[j + 1];
        memcpy(ctx->monitor_modes[j], ctx->monitor_modes[j + 1], sizeof(ctx->monitor_modes[j]));
        ctx->monitor_mode_counts[j] = ctx->monitor_mode_counts[j + 1];
      }
      ctx->monitor_count--;

      /* Push a disconnection event */
      LVKW_Event evt = {0};
      evt.type = LVKW_EVENT_TYPE_MONITOR_CONNECTION;
      evt.window = NULL;
      evt.monitor_connection.monitor = removed;
      evt.monitor_connection.connected = false;
      lvkw_event_queue_push(&ctx->base, &ctx->event_queue, &evt);
      return;
    }
  }
}

void lvkw_mock_addMonitorMode(LVKW_Context *handle, LVKW_MonitorId id, LVKW_VideoMode mode) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)handle;

  for (uint32_t i = 0; i < ctx->monitor_count; i++) {
    if (ctx->monitors[i].id == id) {
      if (ctx->monitor_mode_counts[i] >= LVKW_MOCK_MAX_MODES_PER_MONITOR) return;

      ctx->monitor_modes[i][ctx->monitor_mode_counts[i]] = mode;
      ctx->monitor_mode_counts[i]++;
      return;
    }
  }
}
