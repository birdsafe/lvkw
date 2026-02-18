// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "lvkw/c/core.h"
#include "lvkw/lvkw.h"
#include "lvkw_mock_internal.h"

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_BackendType backend = create_info->backend;

  // In mock-only builds, only AUTO backend selection is valid
  if (backend != LVKW_BACKEND_AUTO) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE,
                                     "Only AUTO backend selection available in mock build");
    return LVKW_ERROR;
  }

  return lvkw_ctx_create_Mock(create_info, out_ctx_handle);
}

LVKW_Status lvkw_context_destroy(LVKW_Context *ctx_handle) { return lvkw_ctx_destroy_Mock(ctx_handle); }

LVKW_Status lvkw_display_listVkExtensions(LVKW_Context *ctx_handle, uint32_t *out_count,
                                     const char *const **out_extensions) {
  return lvkw_ctx_getVkExtensions_Mock(ctx_handle, out_count, out_extensions);
}

LVKW_Status lvkw_events_pump(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  return lvkw_ctx_pumpEvents_Mock(ctx_handle, timeout_ms);
}

LVKW_Status lvkw_events_commit(LVKW_Context *ctx_handle) {
  return lvkw_ctx_commitEvents_Mock(ctx_handle);
}

LVKW_Status _lvkw_ctx_post_backend(LVKW_Context *ctx_handle, LVKW_EventType type,
                                   LVKW_Window *window, const LVKW_Event *evt) {
  return lvkw_ctx_postEvent_Mock(ctx_handle, type, window, evt);
}

LVKW_Status lvkw_events_scan(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  return lvkw_ctx_scanEvents_Mock(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_context_update(LVKW_Context *ctx_handle, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  return lvkw_ctx_update_Mock(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_display_listMonitors(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs, uint32_t *count) {
  return lvkw_ctx_getMonitors_Mock(ctx_handle, out_refs, count);
}

LVKW_Status lvkw_display_createMonitor(LVKW_MonitorRef *monitor_ref, LVKW_Monitor **out_monitor) {
  LVKW_Monitor_Base *m = (LVKW_Monitor_Base *)monitor_ref;
  m->prv.user_refcount++;
  *out_monitor = &m->pub;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_destroyMonitor(LVKW_Monitor *monitor) {
  LVKW_Monitor_Base *m = (LVKW_Monitor_Base *)monitor;
  m->prv.user_refcount--;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_listMonitorModes(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor, LVKW_VideoMode *out_modes,
                                     uint32_t *count) {
  return lvkw_ctx_getMonitorModes_Mock(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *ctx_handle, LVKW_MetricsCategory category, void *out_data,
                                  bool reset) {
  return lvkw_ctx_getMetrics_Mock(ctx_handle, category, out_data, reset);
}

LVKW_Status lvkw_display_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  return lvkw_ctx_createWindow_Mock(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_display_destroyWindow(LVKW_Window *window_handle) { return lvkw_wnd_destroy_Mock(window_handle); }

LVKW_Status lvkw_display_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  return lvkw_wnd_createVkSurface_Mock(window_handle, instance, out_surface);
}

LVKW_Status lvkw_display_getWindowGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  return lvkw_wnd_getGeometry_Mock(window_handle, out_geometry);
}

LVKW_Status lvkw_display_updateWindow(LVKW_Window *window_handle, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {

  return lvkw_wnd_update_Mock(window_handle, field_mask, attributes);

}



LVKW_Status lvkw_display_requestWindowFocus(LVKW_Window *window_handle) { return lvkw_wnd_requestFocus_Mock(window_handle); }

LVKW_Status lvkw_data_setClipboardText(LVKW_Window *window, const char *text) {
  return lvkw_data_pushText(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, text);
}

LVKW_Status lvkw_data_getClipboardText(LVKW_Window *window, const char **out_text) {
  return lvkw_data_pullText(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, out_text);
}

LVKW_Status lvkw_data_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  return lvkw_data_pushData(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD,
                            (const LVKW_DataBuffer *)data, count);
}

LVKW_Status lvkw_data_getClipboardData(LVKW_Window *window, const char *mime_type, const void **out_data,
                                       size_t *out_size) {
  return lvkw_data_pullData(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, mime_type, out_data,
                            out_size);
}

LVKW_Status lvkw_data_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  return lvkw_data_listBufferMimeTypes(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD,
                                       out_mime_types, count);
}

LVKW_Status lvkw_data_pushText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *text) {
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_setClipboardText_Mock(window, text);
}

LVKW_Status lvkw_data_pullText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char **out_text) {
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_getClipboardText_Mock(window, out_text);
}

LVKW_Status lvkw_data_pushData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const LVKW_DataBuffer *data, uint32_t count) {
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_setClipboardData_Mock(window, (const LVKW_ClipboardData *)data, count);
}

LVKW_Status lvkw_data_pullData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *mime_type, const void **out_data, size_t *out_size) {
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_getClipboardData_Mock(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_data_listBufferMimeTypes(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                          const char ***out_mime_types, uint32_t *count) {
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_getClipboardMimeTypes_Mock(window, out_mime_types, count);
}

LVKW_Status lvkw_data_pullTextAsync(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                    void *user_tag) {
  (void)window; (void)target; (void)user_tag;
  return LVKW_ERROR;
}

LVKW_Status lvkw_data_pullDataAsync(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                    const char *mime_type, void *user_tag) {
  (void)window; (void)target; (void)mime_type; (void)user_tag;
  return LVKW_ERROR;
}

LVKW_Status lvkw_display_getStandardCursor(LVKW_Context *ctx, LVKW_CursorShape shape,
                                       LVKW_Cursor **out_cursor) {
  return lvkw_ctx_getStandardCursor_Mock(ctx, shape, out_cursor);
}



LVKW_Status lvkw_display_createCursor(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,

                                  LVKW_Cursor **out_cursor) {

  return lvkw_ctx_createCursor_Mock(ctx, create_info, out_cursor);

}



LVKW_Status lvkw_display_destroyCursor(LVKW_Cursor *cursor) { return lvkw_cursor_destroy_Mock(cursor); }

#ifdef LVKW_ENABLE_CONTROLLER
LVKW_Status lvkw_input_createController(LVKW_ControllerRef *controller_ref,
                                        LVKW_Controller **out_controller) {
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller_ref;
  ctrl->prv.user_refcount++;
  *out_controller = &ctrl->pub;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_destroyController(LVKW_Controller *controller) {
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  ctrl->prv.user_refcount--;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_getControllerInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  return lvkw_ctrl_getInfo_Mock(controller, out_info);
}

LVKW_Status lvkw_input_listControllers(LVKW_Context *ctx, LVKW_ControllerRef **out_refs, uint32_t *out_count) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx;
  if (!out_refs) {
    uint32_t count = 0;
    for (LVKW_Controller_Base *c = ctx_base->prv.controller_list; c; c = c->prv.next) {
      if (c->pub.flags & LVKW_CONTROLLER_STATE_LOST) continue;
      count++;
    }
    *out_count = count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *out_count;
  uint32_t filled = 0;
  for (LVKW_Controller_Base *c = ctx_base->prv.controller_list; c && filled < room; c = c->prv.next) {
    if (c->pub.flags & LVKW_CONTROLLER_STATE_LOST) continue;
    out_refs[filled++] = (LVKW_ControllerRef *)&c->pub;
  }
  *out_count = filled;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_setControllerHapticLevels(LVKW_Controller *controller, uint32_t first_haptic, uint32_t count,
                                      const LVKW_Scalar *intensities) {
  return lvkw_ctrl_setHapticLevels_Mock(controller, first_haptic, count, intensities);
}
#endif
