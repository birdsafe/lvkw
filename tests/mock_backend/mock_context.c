// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "lvkw_api_constraints.h"
#include "lvkw_mock_internal.h"

LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx_handle, uint32_t field_mask,
                                 const LVKW_ContextAttributes *attributes);

#ifdef LVKW_INDIRECT_BACKEND
const LVKW_Backend _lvkw_mock_backend = {
    .context =
        {
            .destroy = lvkw_ctx_destroy_Mock,
            .get_vulkan_instance_extensions = lvkw_ctx_getVkExtensions_Mock,
            .sync_events = lvkw_ctx_syncEvents_Mock,
            .post_event = lvkw_ctx_postEvent_Mock,
            .scan_events = lvkw_ctx_scanEvents_Mock,
            .update = lvkw_ctx_update_Mock,
            .get_monitors = lvkw_ctx_getMonitors_Mock,
            .get_monitor_modes = lvkw_ctx_getMonitorModes_Mock,
            .get_telemetry = lvkw_ctx_getTelemetry_Mock,
        },

    .window =
        {
            .create = lvkw_ctx_createWindow_Mock,
            .destroy = lvkw_wnd_destroy_Mock,
            .create_vk_surface = lvkw_wnd_createVkSurface_Mock,
            .get_geometry = lvkw_wnd_getGeometry_Mock,
            .update = lvkw_wnd_update_Mock,
            .request_focus = lvkw_wnd_requestFocus_Mock,
            .set_clipboard_text = lvkw_wnd_setClipboardText_Mock,
            .get_clipboard_text = lvkw_wnd_getClipboardText_Mock,
            .set_clipboard_data = lvkw_wnd_setClipboardData_Mock,
            .get_clipboard_data = lvkw_wnd_getClipboardData_Mock,
            .get_clipboard_mime_types = lvkw_wnd_getClipboardMimeTypes_Mock,
        },

    .cursor =
        {
            .get_standard = lvkw_ctx_getStandardCursor_Mock,
            .create = lvkw_ctx_createCursor_Mock,
            .destroy = lvkw_cursor_destroy_Mock,
        },
};
#endif

LVKW_Status lvkw_ctx_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;

  LVKW_Context_Mock *ctx =
      (LVKW_Context_Mock *)lvkw_context_alloc_bootstrap(create_info, sizeof(LVKW_Context_Mock));
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_OUT_OF_MEMORY,
                                     "Failed to allocate storage for context");
    return LVKW_ERROR;
  }

  memset(ctx, 0, sizeof(*ctx));
  if (_lvkw_context_init_base(&ctx->base, create_info) != LVKW_SUCCESS) {
    lvkw_context_free(&ctx->base, ctx);
    return LVKW_ERROR;
  }
#ifdef LVKW_INDIRECT_BACKEND
  ctx->base.prv.backend = &_lvkw_mock_backend;
#endif

  *out_ctx_handle = (LVKW_Context *)ctx;

  for (int i = 1; i <= 12; i++) {
    ctx->standard_cursors[i].base.pub.flags = LVKW_CURSOR_FLAG_SYSTEM;
    ctx->standard_cursors[i].base.prv.ctx_base = &ctx->base;
    ctx->standard_cursors[i].shape = (LVKW_CursorShape)i;
  }

  // Apply initial attributes
  lvkw_ctx_update_Mock((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_destroy_Mock(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  // Destroy all windows in list
  while (ctx->base.prv.window_list) {
    lvkw_wnd_destroy_Mock((LVKW_Window *)ctx->base.prv.window_list);
  }

  _lvkw_context_cleanup_base(&ctx->base);

  lvkw_context_free(&ctx->base, ctx);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getVkExtensions_Mock(LVKW_Context *ctx_handle, uint32_t *count,
                                          const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);

  (void)ctx_handle;
  static const char *extensions[] = {NULL};

  if (count) {
    *count = 0;
  }
  if (out_extensions) {
    *out_extensions = extensions;
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_syncEvents_Mock(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;
  (void)timeout_ms;
  lvkw_event_queue_begin_gather(&ctx->base.prv.event_queue);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_Mock(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                    const LVKW_Event *evt) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;
  LVKW_Event empty_evt = {0};
  if (!evt) evt = &empty_evt;

  if (!lvkw_event_queue_push_external(&ctx->base.prv.event_queue, type, window, evt)) {
    return LVKW_ERROR;
  }
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitors_Mock(LVKW_Context *ctx_handle, LVKW_Monitor **out_monitors, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_monitors, count);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  if (!out_monitors) {
    uint32_t monitor_count = 0;
    for (LVKW_Monitor_Base *m = ctx->base.prv.monitor_list; m != NULL; m = m->prv.next) {
      if (!(m->pub.flags & LVKW_MONITOR_STATE_LOST)) {
        monitor_count++;
      }
    }
    *count = monitor_count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *count;
  uint32_t filled = 0;
  for (LVKW_Monitor_Base *m = ctx->base.prv.monitor_list; m != NULL; m = m->prv.next) {
    if (m->pub.flags & LVKW_MONITOR_STATE_LOST) continue;

    if (filled < room) {
      out_monitors[filled++] = &m->pub;
    }
    else {
      break;
    }
  }
  *count = filled;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitorModes_Mock(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                          LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);

  LVKW_Monitor_Mock *m_mock = (LVKW_Monitor_Mock *)monitor;

  if (!out_modes) {
    *count = m_mock->mode_count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *count;
  uint32_t to_copy = (m_mock->mode_count < room) ? m_mock->mode_count : room;

  for (uint32_t i = 0; i < to_copy; i++) {
    out_modes[i] = m_mock->modes[i];
  }
  *count = to_copy;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx_handle, uint32_t field_mask,
                                 const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    ctx->idle_timeout_ms = attributes->idle_timeout_ms;
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    ctx->inhibit_idle = attributes->inhibit_idle;
  }

  _lvkw_update_base_attributes(&ctx->base, field_mask, attributes);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_Mock(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                     LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;
  lvkw_event_queue_scan(&ctx->base.prv.event_queue, event_mask, callback, userdata);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getStandardCursor_Mock(LVKW_Context *ctx_handle, LVKW_CursorShape shape,
                                            LVKW_Cursor **out_cursor) {
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;
  *out_cursor = (LVKW_Cursor *)&ctx->standard_cursors[shape];
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getTelemetry_Mock(LVKW_Context *ctx_handle, LVKW_TelemetryCategory category, void *out_data,
                                        bool reset) {
  LVKW_API_VALIDATE(ctx_getTelemetry, ctx_handle, category, out_data, reset);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  switch (category) {
    case LVKW_TELEMETRY_CATEGORY_EVENTS:
      lvkw_event_queue_get_telemetry(&ctx->base.prv.event_queue, (LVKW_EventTelemetry *)out_data, reset);
      return LVKW_SUCCESS;
    default: return LVKW_ERROR;
  }
}
