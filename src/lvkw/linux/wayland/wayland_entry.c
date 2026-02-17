// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "wayland_internal.h"
#include "api_constraints.h"

#ifdef LVKW_INDIRECT_BACKEND
const LVKW_Backend _lvkw_wayland_backend = {
    .context =
        {
            .destroy = lvkw_ctx_destroy_WL,
            .get_vulkan_instance_extensions = lvkw_ctx_getVkExtensions_WL,
            .pump_events = lvkw_ctx_pumpEvents_WL,
            .commit_events = lvkw_ctx_commitEvents_WL,
            .post_event = lvkw_ctx_postEvent_WL,
            .scan_events = lvkw_ctx_scanEvents_WL,
            .update = lvkw_ctx_update_WL,
            .get_monitors = lvkw_ctx_getMonitors_WL,
            .get_monitor_modes = lvkw_ctx_getMonitorModes_WL,
            .get_metrics = lvkw_ctx_getMetrics_WL,
        },

    .window =
        {
            .create = lvkw_ctx_createWindow_WL,
            .destroy = lvkw_wnd_destroy_WL,
            .create_vk_surface = lvkw_wnd_createVkSurface_WL,
            .get_geometry = lvkw_wnd_getGeometry_WL,
            .update = lvkw_wnd_update_WL,
            .request_focus = lvkw_wnd_requestFocus_WL,
            .set_clipboard_text = lvkw_wnd_setClipboardText_WL,
            .get_clipboard_text = lvkw_wnd_getClipboardText_WL,
            .set_clipboard_data = lvkw_wnd_setClipboardData_WL,
            .get_clipboard_data = lvkw_wnd_getClipboardData_WL,
            .get_clipboard_mime_types = lvkw_wnd_getClipboardMimeTypes_WL,
        },

    .cursor =
        {
            .get_standard = lvkw_ctx_getStandardCursor_WL,
            .create = lvkw_ctx_createCursor_WL,
            .destroy = lvkw_cursor_destroy_WL,
        },

#ifdef LVKW_ENABLE_CONTROLLER
    .ctrl =
        {
            .create = lvkw_input_createController,
            .destroy = lvkw_input_destroyController,
            .getInfo = lvkw_ctrl_getInfo_Linux,
            .setHapticLevels = lvkw_ctrl_setHapticLevels_Linux,
            .list = lvkw_ctrl_list_Linux,
        },
#endif
};
#else
LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info,
                                     LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  return lvkw_ctx_create_WL(create_info, out_ctx_handle);
}

LVKW_Status lvkw_context_destroy(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  return lvkw_ctx_destroy_WL(ctx_handle);
}

LVKW_Status lvkw_display_listVkExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                     const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  return lvkw_ctx_getVkExtensions_WL(ctx_handle, count, out_extensions);
}
LVKW_Status lvkw_events_pump(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_pumpEvents, ctx_handle, timeout_ms);
  return lvkw_ctx_pumpEvents_WL(ctx_handle, timeout_ms);
}
LVKW_Status lvkw_events_commit(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_commitEvents, ctx_handle);
  return lvkw_ctx_commitEvents_WL(ctx_handle);
}
LVKW_Status lvkw_events_post(LVKW_Context *ctx_handle, LVKW_EventType type,
                               LVKW_Window *window, const LVKW_Event *evt) {
  LVKW_API_VALIDATE(ctx_postEvent, ctx_handle, type, window, evt);
  return lvkw_ctx_postEvent_WL(ctx_handle, type, window, evt);
}
LVKW_Status lvkw_events_scan(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_scanEvents, ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_scanEvents_WL(ctx_handle, event_mask, callback, userdata);
}
LVKW_Status lvkw_context_update(LVKW_Context *ctx_handle, uint32_t field_mask,
                            const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  return lvkw_ctx_update_WL(ctx_handle, field_mask, attributes);
}
LVKW_Status lvkw_display_listMonitors(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs,
                                 uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_refs, count);
  return lvkw_ctx_getMonitors_WL(ctx_handle, out_refs, count);
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
LVKW_Status lvkw_display_listMonitorModes(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_WL(ctx_handle, monitor, out_modes, count);
}
LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *ctx_handle, LVKW_MetricsCategory category,
                                  void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx_handle, category, out_data, reset);
  return lvkw_ctx_getMetrics_WL(ctx_handle, category, out_data, reset);
}
LVKW_Status lvkw_display_createWindow(LVKW_Context *ctx_handle,
                                  const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_WL(ctx_handle, create_info, out_window_handle);
}
LVKW_Status lvkw_display_destroyWindow(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  return lvkw_wnd_destroy_WL(window_handle);
}
LVKW_Status lvkw_display_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                     VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_WL(window_handle, instance, out_surface);
}
LVKW_Status lvkw_display_getWindowGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  return lvkw_wnd_getGeometry_WL(window_handle, out_geometry);
}

LVKW_Status lvkw_display_updateWindow(LVKW_Window *window_handle, uint32_t field_mask,
                            const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  return lvkw_wnd_update_WL(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_display_requestWindowFocus(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  return lvkw_wnd_requestFocus_WL(window_handle);
}

LVKW_Status lvkw_data_setClipboardText(LVKW_Window *window_handle, const char *text) {
  LVKW_API_VALIDATE(wnd_setClipboardText, window_handle, text);
  return lvkw_wnd_setClipboardText_WL(window_handle, text);
}

LVKW_Status lvkw_data_getClipboardText(LVKW_Window *window_handle, const char **out_text) {
  LVKW_API_VALIDATE(wnd_getClipboardText, window_handle, out_text);
  return lvkw_wnd_getClipboardText_WL(window_handle, out_text);
}

LVKW_Status lvkw_data_setClipboardData(LVKW_Window *window_handle, const LVKW_ClipboardData *data,
                                      uint32_t count) {
  LVKW_API_VALIDATE(wnd_setClipboardData, window_handle, data, count);
  return lvkw_wnd_setClipboardData_WL(window_handle, data, count);
}

LVKW_Status lvkw_data_getClipboardData(LVKW_Window *window_handle, const char *mime_type,
                                      const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(wnd_getClipboardData, window_handle, mime_type, out_data, out_size);
  return lvkw_wnd_getClipboardData_WL(window_handle, mime_type, out_data, out_size);
}

LVKW_Status lvkw_data_getClipboardMimeTypes(LVKW_Window *window_handle, const char ***out_mime_types,
                                           uint32_t *count) {
  LVKW_API_VALIDATE(wnd_getClipboardMimeTypes, window_handle, out_mime_types, count);
  return lvkw_wnd_getClipboardMimeTypes_WL(window_handle, out_mime_types, count);
}

LVKW_Status lvkw_display_getStandardCursor(LVKW_Context *ctx, LVKW_CursorShape shape,
                                       LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_getStandardCursor, ctx, shape, out_cursor);
  return lvkw_ctx_getStandardCursor_WL(ctx, shape, out_cursor);
}

LVKW_Status lvkw_display_createCursor(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                  LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx, create_info, out_cursor);
  return lvkw_ctx_createCursor_WL(ctx, create_info, out_cursor);
}

LVKW_Status lvkw_display_destroyCursor(LVKW_Cursor *cursor) {
  LVKW_API_VALIDATE(cursor_destroy, cursor);
  return lvkw_cursor_destroy_WL(cursor);
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
