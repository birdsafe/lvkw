// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw/c/core.h"
#include "lvkw/lvkw.h"
#include "api_constraints.h"
#include "x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND

const LVKW_Backend _lvkw_x11_backend = {
    .context =
        {
            .destroy = lvkw_ctx_destroy_X11,
            .get_vulkan_instance_extensions = lvkw_ctx_getVkExtensions_X11,
            .pump_events = lvkw_ctx_pumpEvents_X11,
            .post_event = lvkw_ctx_postEvent_X11,
            .get_monitors = lvkw_ctx_getMonitors_X11,
            .get_monitor_modes = lvkw_ctx_getMonitorModes_X11,
            .get_metrics = lvkw_ctx_getMetrics_X11,
        },
    .window =
        {
            .create = lvkw_ctx_createWindow_X11,
            .destroy = lvkw_wnd_destroy_X11,
            .create_vk_surface = lvkw_wnd_createVkSurface_X11,
            .get_geometry = lvkw_wnd_getGeometry_X11,
            .update = lvkw_wnd_update_X11,
            .request_focus = lvkw_wnd_requestFocus_X11,
            .push_text = lvkw_wnd_setClipboardText_X11,
            .pull_text = lvkw_wnd_getClipboardText_X11,
            .push_data = lvkw_wnd_setClipboardData_X11,
            .pull_data = lvkw_wnd_getClipboardData_X11,
            .list_buffer_mime_types = lvkw_wnd_getClipboardMimeTypes_X11,
            .pull_text_async = NULL,
            .pull_data_async = NULL,
        },
    .cursor =
        {
            .get_standard = lvkw_ctx_getStandardCursor_X11,
            .create = lvkw_ctx_createCursor_X11,
            .destroy = lvkw_cursor_destroy_X11,
        },
};

#else

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info,
                                     LVKW_Context **out_context) {
  return lvkw_ctx_create_X11(create_info, out_context);
}
LVKW_Status lvkw_context_destroy(LVKW_Context *handle) {
  LVKW_API_VALIDATE(ctx_destroy, handle);
  return lvkw_ctx_destroy_X11(handle);
}

LVKW_Status lvkw_display_listVkExtensions(LVKW_Context *ctx, uint32_t *count,
                                     const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx, count, out_extensions);
  return lvkw_ctx_getVkExtensions_X11(ctx, count, out_extensions);
}
LVKW_Status lvkw_events_pump(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_pumpEvents, ctx, timeout_ms);
  return lvkw_ctx_pumpEvents_X11(ctx, timeout_ms);
}
LVKW_Status _lvkw_ctx_post_backend(LVKW_Context *ctx, LVKW_EventType type, LVKW_Window *window,
                                   const LVKW_Event *evt) {
  return lvkw_ctx_postEvent_X11(ctx, type, window, evt);
}
LVKW_Status lvkw_display_listMonitors(LVKW_Context *ctx, LVKW_MonitorRef **out_refs, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx, out_refs, count);
  return lvkw_ctx_getMonitors_X11(ctx, out_refs, count);
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

LVKW_Status lvkw_display_listMonitorModes(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_X11(ctx, monitor, out_modes, count);
}
LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *ctx, LVKW_MetricsCategory category,
                                  void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx, category, out_data, reset);
  return lvkw_ctx_getMetrics_X11(ctx, category, out_data, reset);
}
LVKW_Status lvkw_display_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx, create_info, out_window);
  return lvkw_ctx_createWindow_X11(ctx, create_info, out_window);
}
LVKW_Status lvkw_display_destroyWindow(LVKW_Window *handle) {
  LVKW_API_VALIDATE(wnd_destroy, handle);
  return lvkw_wnd_destroy_X11(handle);
}
LVKW_Status lvkw_display_createVkSurface(LVKW_Window *window, VkInstance instance,
                                     VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window, instance, out_surface);
  return lvkw_wnd_createVkSurface_X11(window, instance, out_surface);
}
LVKW_Status lvkw_display_getWindowGeometry(LVKW_Window *window, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window, out_geometry);
  return lvkw_wnd_getGeometry_X11(window, out_geometry);
}
LVKW_Status lvkw_display_updateWindow(LVKW_Window *window, uint32_t field_mask,
                            const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window, field_mask, attributes);
  return lvkw_wnd_update_X11(window, field_mask, attributes);
}
LVKW_Status lvkw_display_requestWindowFocus(LVKW_Window *window) {
  LVKW_API_VALIDATE(wnd_requestFocus, window);
  return lvkw_wnd_requestFocus_X11(window);
}

LVKW_Status lvkw_data_setClipboardText(LVKW_Window *window, const char *text) {
  return lvkw_data_pushText(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, text);
}

LVKW_Status lvkw_data_getClipboardText(LVKW_Window *window, const char **out_text) {
  return lvkw_data_pullText(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, out_text);
}

LVKW_Status lvkw_data_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data,
                                      uint32_t count) {
  return lvkw_data_pushData(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD,
                            (const LVKW_DataBuffer *)data, count);
}

LVKW_Status lvkw_data_getClipboardData(LVKW_Window *window, const char *mime_type,
                                      const void **out_data, size_t *out_size) {
  return lvkw_data_pullData(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, mime_type, out_data,
                            out_size);
}

LVKW_Status lvkw_data_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types,
                                           uint32_t *count) {
  return lvkw_data_listBufferMimeTypes(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD,
                                       out_mime_types, count);
}

LVKW_Status lvkw_data_pushText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *text) {
  LVKW_API_VALIDATE(data_pushText, window, target, text);
  return lvkw_wnd_setClipboardText_X11(window, target, text);
}

LVKW_Status lvkw_data_pullText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char **out_text) {
  LVKW_API_VALIDATE(data_pullText, window, target, out_text);
  return lvkw_wnd_getClipboardText_X11(window, target, out_text);
}

LVKW_Status lvkw_data_pushData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const LVKW_DataBuffer *data, uint32_t count) {
  LVKW_API_VALIDATE(data_pushData, window, target, data, count);
  return lvkw_wnd_setClipboardData_X11(window, target, data, count);
}

LVKW_Status lvkw_data_pullData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *mime_type, const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(data_pullData, window, target, mime_type, out_data, out_size);
  return lvkw_wnd_getClipboardData_X11(window, target, mime_type, out_data, out_size);
}

LVKW_Status lvkw_data_listBufferMimeTypes(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                          const char ***out_mime_types, uint32_t *count) {
  LVKW_API_VALIDATE(data_listBufferMimeTypes, window, target, out_mime_types, count);
  return lvkw_wnd_getClipboardMimeTypes_X11(window, target, out_mime_types, count);
}

LVKW_Status lvkw_data_pullTextAsync(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                    void *user_tag) {
  LVKW_API_VALIDATE(data_pullTextAsync, window, target, user_tag);
  return LVKW_ERROR;
}

LVKW_Status lvkw_data_pullDataAsync(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                    const char *mime_type, void *user_tag) {
  LVKW_API_VALIDATE(data_pullDataAsync, window, target, mime_type, user_tag);
  return LVKW_ERROR;
}

LVKW_Status lvkw_display_getStandardCursor(LVKW_Context *ctx, LVKW_CursorShape shape,
                                       LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_getStandardCursor, ctx, shape, out_cursor);
  return lvkw_ctx_getStandardCursor_X11(ctx, shape, out_cursor);
}

LVKW_Status lvkw_display_createCursor(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                  LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx, create_info, out_cursor);
  return lvkw_ctx_createCursor_X11(ctx, create_info, out_cursor);
}

LVKW_Status lvkw_display_destroyCursor(LVKW_Cursor *cursor) {
  LVKW_API_VALIDATE(cursor_destroy, cursor);
  return lvkw_cursor_destroy_X11(cursor);
}

#ifdef LVKW_ENABLE_CONTROLLER
LVKW_Status lvkw_input_createController(LVKW_ControllerRef *controller_ref,
                                        LVKW_Controller **out_controller) {
  LVKW_API_VALIDATE(ctrl_create, controller_ref, out_controller);
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller_ref;
  ctrl->prv.user_refcount++;
  *out_controller = &ctrl->pub;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_destroyController(LVKW_Controller *controller) {
  LVKW_API_VALIDATE(ctrl_destroy, controller);
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  ctrl->prv.user_refcount--;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_getControllerInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_API_VALIDATE(ctrl_getInfo, controller, out_info);
  return lvkw_ctrl_getInfo_Linux(controller, out_info);
}

LVKW_Status lvkw_input_listControllers(LVKW_Context *ctx, LVKW_ControllerRef **out_refs, uint32_t *out_count) {
  LVKW_API_VALIDATE(ctrl_list, ctx, out_refs, out_count);
  return lvkw_ctrl_list_Linux(ctx, out_refs, out_count);
}

LVKW_Status lvkw_input_setControllerHapticLevels(LVKW_Controller *controller, uint32_t first_haptic,
                                      uint32_t count, const LVKW_Scalar *intensities) {
  LVKW_API_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);
  return lvkw_ctrl_setHapticLevels_Linux(controller, first_haptic, count, intensities);
}
#endif

#endif
