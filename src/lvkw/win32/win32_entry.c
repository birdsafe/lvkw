// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "win32_internal.h"
#include "api_constraints.h"

#define LVKW_VALIDATE(func, ...) do { \
    LVKW_Status _s = _lvkw_api_constraints_##func(__VA_ARGS__); \
    if (_s != LVKW_SUCCESS) return _s; \
} while(0)

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  if (create_info->backend != LVKW_BACKEND_AUTO && create_info->backend != LVKW_BACKEND_WIN32) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE, "Requested backend is not supported on Windows");
    return LVKW_ERROR;
  }

  return lvkw_ctx_create_Win32(create_info, out_ctx_handle);
}

LVKW_Status lvkw_context_destroy(LVKW_Context *ctx_handle) {
  LVKW_VALIDATE(ctx_destroy, ctx_handle);
  lvkw_ctx_destroy_Win32(ctx_handle);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_listVkExtensions(LVKW_Context *ctx_handle, uint32_t *count, const char *const **out_extensions) {
  LVKW_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  *out_extensions = lvkw_ctx_getVkExtensions_Win32(ctx_handle, count);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_events_pump(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_VALIDATE(ctx_pumpEvents, ctx_handle, timeout_ms);
  return lvkw_ctx_pumpEvents_Win32(ctx_handle, timeout_ms);
}

LVKW_Status lvkw_events_commit(LVKW_Context *ctx_handle) {
  LVKW_VALIDATE(ctx_commitEvents, ctx_handle);
  return lvkw_ctx_commitEvents_Win32(ctx_handle);
}

LVKW_Status lvkw_events_post(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                               const LVKW_Event *evt) {
  LVKW_VALIDATE(ctx_postEvent, ctx_handle, type, window, evt);
  return lvkw_ctx_postEvent_Win32(ctx_handle, type, window, evt);
}

LVKW_Status lvkw_events_scan(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  LVKW_VALIDATE(ctx_scanEvents, ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_scanEvents_Win32(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_context_update(LVKW_Context *ctx_handle, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  LVKW_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  return lvkw_ctx_update_Win32(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_display_listMonitors(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs, uint32_t *count) {
  LVKW_VALIDATE(ctx_getMonitors, ctx_handle, out_refs, count);
  return lvkw_ctx_getMonitors_Win32(ctx_handle, out_refs, count);
}
LVKW_Status lvkw_display_createMonitor(LVKW_MonitorRef *monitor_ref, LVKW_Monitor **out_monitor) {
  LVKW_VALIDATE(monitor_createRef, monitor_ref, out_monitor);
  LVKW_Monitor_Base *monitor_base = (LVKW_Monitor_Base *)monitor_ref;
  monitor_base->prv.user_refcount++;
  *out_monitor = &monitor_base->pub;
  return LVKW_SUCCESS;
}
LVKW_Status lvkw_display_destroyMonitor(LVKW_Monitor *monitor) {
  LVKW_VALIDATE(monitor_destroy, monitor);
  LVKW_Monitor_Base *monitor_base = (LVKW_Monitor_Base *)monitor;
  monitor_base->prv.user_refcount--;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_listMonitorModes(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_Win32(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *ctx, LVKW_MetricsCategory category, void *out_data, bool reset) {
  LVKW_VALIDATE(ctx_getMetrics, ctx, category, out_data, reset);
  return lvkw_ctx_get_metrics_Win32(ctx, category, out_data, reset);
}

LVKW_Status lvkw_ctx_getMetrics_Win32(LVKW_Context *ctx, LVKW_MetricsCategory category, void *out_data,
                                         bool reset) {
  (void)ctx;
  (void)category;
  (void)reset;
  
  if (category == LVKW_METRICS_CATEGORY_EVENTS) {
    memset(out_data, 0, sizeof(LVKW_EventMetrics));
    return LVKW_SUCCESS;
  }

  return LVKW_ERROR;
}

LVKW_Status lvkw_display_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  LVKW_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_Win32(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_display_destroyWindow(LVKW_Window *window_handle) {
  LVKW_VALIDATE(wnd_destroy, window_handle);
  lvkw_wnd_destroy_Win32(window_handle);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_Win32(window_handle, instance, out_surface);
}

LVKW_Status lvkw_display_getWindowGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  return lvkw_wnd_getGeometry_Win32(window_handle, out_geometry);
}

LVKW_Status lvkw_display_updateWindow(LVKW_Window *window_handle, uint32_t field_mask,
                            const LVKW_WindowAttributes *attributes) {
  LVKW_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  return lvkw_wnd_update_Win32(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_display_requestWindowFocus(LVKW_Window *window_handle) {
  LVKW_VALIDATE(wnd_requestFocus, window_handle);
  return lvkw_wnd_requestFocus_Win32(window_handle);
}

LVKW_Status lvkw_data_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_VALIDATE(wnd_setClipboardText, window, text);
  return lvkw_wnd_setClipboardText_Win32(window, text);
}

LVKW_Status lvkw_data_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_VALIDATE(wnd_getClipboardText, window, out_text);
  return lvkw_wnd_getClipboardText_Win32(window, out_text);
}

LVKW_Status lvkw_data_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  LVKW_VALIDATE(wnd_setClipboardData, window, data, count);
  return lvkw_wnd_setClipboardData_Win32(window, data, count);
}

LVKW_Status lvkw_data_getClipboardData(LVKW_Window *window, const char *mime_type, const void **out_data,
                                       size_t *out_size) {
  LVKW_VALIDATE(wnd_getClipboardData, window, mime_type, out_data, out_size);
  return lvkw_wnd_getClipboardData_Win32(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_data_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  LVKW_VALIDATE(wnd_getClipboardMimeTypes, window, out_mime_types, count);
  return lvkw_wnd_getClipboardMimeTypes_Win32(window, out_mime_types, count);
}

LVKW_Status lvkw_wnd_setClipboardText_Win32(LVKW_Window *window, const char *text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Win32");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardText_Win32(LVKW_Window *window, const char **out_text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Win32");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_setClipboardData_Win32(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Win32");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardData_Win32(LVKW_Window *window, const char *mime_type, const void **out_data,
                                            size_t *out_size) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Win32");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_Win32(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Win32");
  return LVKW_ERROR;
}

#ifdef LVKW_ENABLE_CONTROLLER
LVKW_Status lvkw_input_createController(LVKW_ControllerRef *controller_ref,
                                        LVKW_Controller **out_controller) {
  LVKW_VALIDATE(ctrl_create, controller_ref, out_controller);
  *out_controller = NULL;
  return LVKW_ERROR;
}

LVKW_Status lvkw_input_destroyController(LVKW_Controller *controller) {
  LVKW_VALIDATE(ctrl_destroy, controller);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_listControllers(LVKW_Context *ctx, LVKW_ControllerRef **out_refs, uint32_t *out_count) {
  LVKW_VALIDATE(ctrl_list, ctx, out_refs, out_count);
  *out_count = 0;
  return LVKW_ERROR;
}

LVKW_Status lvkw_input_getControllerInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_VALIDATE(ctrl_getInfo, controller, out_info);
  return LVKW_ERROR;
}

LVKW_Status lvkw_input_setControllerHapticLevels(LVKW_Controller *controller, uint32_t first_haptic, uint32_t count,
                                      const LVKW_Scalar *intensities) {
  LVKW_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);
  return LVKW_ERROR;
}
#endif
