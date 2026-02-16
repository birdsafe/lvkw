// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw/lvkw-core.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND

const LVKW_Backend _lvkw_x11_backend = {
    .context =
        {
            .destroy = lvkw_ctx_destroy_X11,
            .get_vulkan_instance_extensions = lvkw_ctx_getVkExtensions_X11,
            .sync_events = lvkw_ctx_syncEvents_X11,
            .post_event = lvkw_ctx_postEvent_X11,
            .scan_events = lvkw_ctx_scanEvents_X11,
            .update = lvkw_ctx_update_X11,
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
            .set_clipboard_text = lvkw_wnd_setClipboardText_X11,
            .get_clipboard_text = lvkw_wnd_getClipboardText_X11,
            .set_clipboard_data = lvkw_wnd_setClipboardData_X11,
            .get_clipboard_data = lvkw_wnd_getClipboardData_X11,
            .get_clipboard_mime_types = lvkw_wnd_getClipboardMimeTypes_X11,
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
LVKW_Status lvkw_ctx_destroy(LVKW_Context *handle) {
  LVKW_API_VALIDATE(ctx_destroy, handle);
  return lvkw_ctx_destroy_X11(handle);
}

LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count,
                                     const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx, count, out_extensions);
  return lvkw_ctx_getVkExtensions_X11(ctx, count, out_extensions);
}
LVKW_Status lvkw_ctx_syncEvents(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_syncEvents, ctx, timeout_ms);
  return lvkw_ctx_syncEvents_X11(ctx, timeout_ms);
}
LVKW_Status lvkw_ctx_postEvent(LVKW_Context *ctx, LVKW_EventType type, LVKW_Window *window,
                               const LVKW_Event *evt) {
  LVKW_API_VALIDATE(ctx_postEvent, ctx, type, window, evt);
  return lvkw_ctx_postEvent_X11(ctx, type, window, evt);
}
LVKW_Status lvkw_ctx_scanEvents(LVKW_Context *ctx, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_scanEvents, ctx, event_mask, callback, userdata);
  return lvkw_ctx_scanEvents_X11(ctx, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_update(LVKW_Context *ctx, uint32_t field_mask,
                            const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx, field_mask, attributes);
  return lvkw_ctx_update_X11(ctx, field_mask, attributes);
}
LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx, LVKW_Monitor **out_monitors, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx, out_monitors, count);
  return lvkw_ctx_getMonitors_X11(ctx, out_monitors, count);
}

LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_X11(ctx, monitor, out_modes, count);
}
LVKW_Status lvkw_ctx_getMetrics(LVKW_Context *ctx, LVKW_MetricsCategory category,
                                  void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx, category, out_data, reset);
  return lvkw_ctx_getMetrics_X11(ctx, category, out_data, reset);
}
LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx, create_info, out_window);
  return lvkw_ctx_createWindow_X11(ctx, create_info, out_window);
}
LVKW_Status lvkw_wnd_destroy(LVKW_Window *handle) {
  LVKW_API_VALIDATE(wnd_destroy, handle);
  return lvkw_wnd_destroy_X11(handle);
}
LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window, VkInstance instance,
                                     VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window, instance, out_surface);
  return lvkw_wnd_createVkSurface_X11(window, instance, out_surface);
}
LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window, out_geometry);
  return lvkw_wnd_getGeometry_X11(window, out_geometry);
}
LVKW_Status lvkw_wnd_update(LVKW_Window *window, uint32_t field_mask,
                            const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window, field_mask, attributes);
  return lvkw_wnd_update_X11(window, field_mask, attributes);
}
LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window) {
  LVKW_API_VALIDATE(wnd_requestFocus, window);
  return lvkw_wnd_requestFocus_X11(window);
}

LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_API_VALIDATE(wnd_setClipboardText, window, text);
  return lvkw_wnd_setClipboardText_X11(window, text);
}

LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_API_VALIDATE(wnd_getClipboardText, window, out_text);
  return lvkw_wnd_getClipboardText_X11(window, out_text);
}

LVKW_Status lvkw_wnd_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data,
                                      uint32_t count) {
  LVKW_API_VALIDATE(wnd_setClipboardData, window, data, count);
  return lvkw_wnd_setClipboardData_X11(window, data, count);
}

LVKW_Status lvkw_wnd_getClipboardData(LVKW_Window *window, const char *mime_type,
                                      const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(wnd_getClipboardData, window, mime_type, out_data, out_size);
  return lvkw_wnd_getClipboardData_X11(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types,
                                           uint32_t *count) {
  LVKW_API_VALIDATE(wnd_getClipboardMimeTypes, window, out_mime_types, count);
  return lvkw_wnd_getClipboardMimeTypes_X11(window, out_mime_types, count);
}

LVKW_Status lvkw_ctx_getStandardCursor(LVKW_Context *ctx, LVKW_CursorShape shape,
                                       LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_getStandardCursor, ctx, shape, out_cursor);
  return lvkw_ctx_getStandardCursor_X11(ctx, shape, out_cursor);
}

LVKW_Status lvkw_ctx_createCursor(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                  LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx, create_info, out_cursor);
  return lvkw_ctx_createCursor_X11(ctx, create_info, out_cursor);
}

LVKW_Status lvkw_cursor_destroy(LVKW_Cursor *cursor) {
  LVKW_API_VALIDATE(cursor_destroy, cursor);
  return lvkw_cursor_destroy_X11(cursor);
}

#ifdef LVKW_ENABLE_CONTROLLER
LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  LVKW_API_VALIDATE(ctrl_create, ctx, id, out_controller);
  return lvkw_ctrl_create_Linux(ctx, id, out_controller);
}

LVKW_Status lvkw_ctrl_destroy(LVKW_Controller *ctrl) {
  LVKW_API_VALIDATE(ctrl_destroy, ctrl);
  return lvkw_ctrl_destroy_Linux(ctrl);
}

LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_API_VALIDATE(ctrl_getInfo, controller, out_info);
  return lvkw_ctrl_getInfo_Linux(controller, out_info);
}

LVKW_Status lvkw_ctrl_list(LVKW_Context *ctx, LVKW_CtrlId *out_ids, uint32_t *out_count) {
  LVKW_API_VALIDATE(ctrl_list, ctx, out_ids, out_count);
  return lvkw_ctrl_list_Linux(ctx, out_ids, out_count);
}

LVKW_Status lvkw_ctrl_setHapticLevels(LVKW_Controller *controller, uint32_t first_haptic,
                                      uint32_t count, const LVKW_Scalar *intensities) {
  LVKW_API_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);
  return lvkw_ctrl_setHapticLevels_Linux(controller, first_haptic, count, intensities);
}
#endif

#endif
