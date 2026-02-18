// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "api_constraints.h"
#include "lvkw/lvkw.h"
#include "macos_internal.h"

#include <mach/mach_time.h>
#include "internal.h"

uint64_t _lvkw_get_timestamp_ms(void) {
  static mach_timebase_info_data_t timebase;
  if (timebase.denom == 0) {
    mach_timebase_info(&timebase);
  }
  return (mach_absolute_time() * timebase.numer / timebase.denom) / 1000000;
}

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  if (create_info->backend != LVKW_BACKEND_AUTO && create_info->backend != LVKW_BACKEND_COCOA) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE,
                                     "Requested backend is not supported on MacOS");
    return LVKW_ERROR;
  }

  return lvkw_ctx_create_Cocoa(create_info, out_ctx_handle);
}

LVKW_Status lvkw_context_destroy(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  return lvkw_ctx_destroy_Cocoa(ctx_handle);
}

LVKW_Status lvkw_display_listVkExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                     const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  return lvkw_ctx_getVkExtensions_Cocoa(ctx_handle, count, out_extensions);
}

LVKW_Status lvkw_events_pump(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_pumpEvents, ctx_handle, timeout_ms);
  return lvkw_ctx_pumpEvents_Cocoa(ctx_handle, timeout_ms);
}

LVKW_Status _lvkw_ctx_post_backend(LVKW_Context *ctx_handle, LVKW_EventType type,
                                   LVKW_Window *window, const LVKW_Event *evt) {
  return lvkw_ctx_postEvent_Cocoa(ctx_handle, type, window, evt);
}

LVKW_Status lvkw_display_listMonitors(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_refs, count);
  return lvkw_ctx_getMonitors_Cocoa(ctx_handle, out_refs, count);
}
LVKW_Status lvkw_display_createMonitor(LVKW_MonitorRef *monitor_ref, LVKW_Monitor **out_monitor) {
  LVKW_API_VALIDATE(monitor_createRef, monitor_ref, out_monitor);
  LVKW_Monitor_Base *monitor_base = (LVKW_Monitor_Base *)monitor_ref;
  monitor_base->prv.user_refcount++;
  *out_monitor = &monitor_base->pub;
  return LVKW_SUCCESS;
}
LVKW_Status lvkw_display_destroyMonitor(LVKW_Monitor *monitor) {
  LVKW_API_VALIDATE(monitor_destroy, monitor);
  LVKW_Monitor_Base *monitor_base = (LVKW_Monitor_Base *)monitor;
  monitor_base->prv.user_refcount--;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_listMonitorModes(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor, LVKW_VideoMode *out_modes,
                                     uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_Cocoa(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *ctx, LVKW_MetricsCategory category, void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx, category, out_data, reset);
  return lvkw_ctx_getMetrics_Cocoa(ctx, category, out_data, reset);
}

LVKW_Status lvkw_ctx_getMetrics_Cocoa(LVKW_Context *ctx, LVKW_MetricsCategory category, void *out_data,
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
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_Cocoa(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_display_destroyWindow(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  return lvkw_wnd_destroy_Cocoa(window_handle);
}

LVKW_Status lvkw_display_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_Cocoa(window_handle, instance, out_surface);
}

LVKW_Status lvkw_display_getWindowGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  return lvkw_wnd_getGeometry_Cocoa(window_handle, out_geometry);
}

LVKW_Status lvkw_display_updateWindow(LVKW_Window *window_handle, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  return lvkw_wnd_update_Cocoa(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_display_requestWindowFocus(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  return lvkw_wnd_requestFocus_Cocoa(window_handle);
}

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
  LVKW_API_VALIDATE(data_pushText, window, target, text);
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_setClipboardText_Cocoa(window, text);
}

LVKW_Status lvkw_data_pullText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char **out_text) {
  LVKW_API_VALIDATE(data_pullText, window, target, out_text);
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_getClipboardText_Cocoa(window, out_text);
}

LVKW_Status lvkw_data_pushData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const LVKW_DataBuffer *data, uint32_t count) {
  LVKW_API_VALIDATE(data_pushData, window, target, data, count);
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_setClipboardData_Cocoa(window, (const LVKW_ClipboardData *)data, count);
}

LVKW_Status lvkw_data_pullData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *mime_type, const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(data_pullData, window, target, mime_type, out_data, out_size);
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_getClipboardData_Cocoa(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_data_listBufferMimeTypes(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                          const char ***out_mime_types, uint32_t *count) {
  LVKW_API_VALIDATE(data_listBufferMimeTypes, window, target, out_mime_types, count);
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) return LVKW_ERROR;
  return lvkw_wnd_getClipboardMimeTypes_Cocoa(window, out_mime_types, count);
}

LVKW_Status lvkw_wnd_setClipboardText_Cocoa(LVKW_Window *window, const char *text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardText_Cocoa(LVKW_Window *window, const char **out_text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_setClipboardData_Cocoa(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardData_Cocoa(LVKW_Window *window, const char *mime_type, const void **out_data,
                                            size_t *out_size) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_Cocoa(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_display_getStandardCursor(LVKW_Context *ctx_handle, LVKW_CursorShape shape, LVKW_Cursor **out_cursor_handle) {
  LVKW_API_VALIDATE(ctx_getStandardCursor, ctx_handle, shape, out_cursor_handle);
  return lvkw_ctx_getStandardCursor_Cocoa(ctx_handle, shape, out_cursor_handle);
}

LVKW_Status lvkw_display_createCursor(LVKW_Context *ctx_handle, const LVKW_CursorCreateInfo *create_info,
                                      LVKW_Cursor **out_cursor_handle) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx_handle, create_info, out_cursor_handle);
  return lvkw_ctx_createCursor_Cocoa(ctx_handle, create_info, out_cursor_handle);
}

LVKW_Status lvkw_display_destroyCursor(LVKW_Cursor *cursor_handle) {
  LVKW_API_VALIDATE(cursor_destroy, cursor_handle);
  return lvkw_cursor_destroy_Cocoa(cursor_handle);
}

#ifdef LVKW_ENABLE_CONTROLLER
LVKW_Status lvkw_input_createController(LVKW_ControllerRef *controller_ref,
                                        LVKW_Controller **out_controller) {
  LVKW_API_VALIDATE(ctrl_create, controller_ref, out_controller);
  *out_controller = NULL;
  return LVKW_ERROR;
}

LVKW_Status lvkw_input_destroyController(LVKW_Controller *controller) {
  LVKW_API_VALIDATE(ctrl_destroy, controller);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_listControllers(LVKW_Context *ctx, LVKW_ControllerRef **out_refs, uint32_t *out_count) {
  LVKW_API_VALIDATE(ctrl_list, ctx, out_refs, out_count);
  *out_count = 0;
  return LVKW_ERROR;
}

LVKW_Status lvkw_input_getControllerInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_API_VALIDATE(ctrl_getInfo, controller, out_info);
  return LVKW_ERROR;
}

LVKW_Status lvkw_input_setControllerHapticLevels(LVKW_Controller *controller, uint32_t first_haptic, uint32_t count,
                                      const LVKW_Scalar *intensities) {
  LVKW_API_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);
  return LVKW_ERROR;
}
#endif
