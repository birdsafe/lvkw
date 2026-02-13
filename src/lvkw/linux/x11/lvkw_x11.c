#include "lvkw/lvkw-core.h"
#include "lvkw/lvkw.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND

const LVKW_Backend _lvkw_x11_backend = {
    .context =
        {
            .destroy = lvkw_ctx_destroy_X11,
            .get_vulkan_instance_extensions = lvkw_ctx_getVkExtensions_X11,
            .poll_events = lvkw_ctx_pollEvents_X11,
            .wait_events = lvkw_ctx_waitEvents_X11,
            .update = lvkw_ctx_update_X11,
            .get_monitors = lvkw_ctx_getMonitors_X11,
            .get_monitor_modes = lvkw_ctx_getMonitorModes_X11,
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

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  return lvkw_ctx_create_X11(create_info, out_context);
}
LVKW_Status lvkw_ctx_destroy(LVKW_Context *handle) { return lvkw_ctx_destroy_X11(handle); }

LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count, const char *const **out_extensions) {
  return lvkw_ctx_getVkExtensions_X11(ctx, count, out_extensions);
}
LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  return lvkw_ctx_pollEvents_X11(ctx, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  return lvkw_ctx_waitEvents_X11(ctx, timeout_ms, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_update(LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  return lvkw_ctx_update_X11(ctx, field_mask, attributes);
}
LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx, LVKW_Monitor **out_monitors, uint32_t *count) {
  return lvkw_ctx_getMonitors_X11(ctx, out_monitors, count);
}

LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx, const LVKW_Monitor *monitor, LVKW_VideoMode *out_modes,
                                     uint32_t *count) {
  return lvkw_ctx_getMonitorModes_X11(ctx, monitor, out_modes, count);
}
LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window) {
  return lvkw_ctx_createWindow_X11(ctx, create_info, out_window);
}
LVKW_Status lvkw_wnd_destroy(LVKW_Window *handle) { return lvkw_wnd_destroy_X11(handle); }
LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface) {
  return lvkw_wnd_createVkSurface_X11(window, instance, out_surface);
}
LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window, LVKW_WindowGeometry *out_geometry) {
  return lvkw_wnd_getGeometry_X11(window, out_geometry);
}
LVKW_Status lvkw_wnd_update(LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {
  return lvkw_wnd_update_X11(window, field_mask, attributes);
}
LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window) { return lvkw_wnd_requestFocus_X11(window); }

LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  return lvkw_wnd_setClipboardText_X11(window, text);
}

LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  return lvkw_wnd_getClipboardText_X11(window, out_text);
}

LVKW_Status lvkw_wnd_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  return lvkw_wnd_setClipboardData_X11(window, data, count);
}

LVKW_Status lvkw_wnd_getClipboardData(LVKW_Window *window, const char *mime_type, const void **out_data,
                                       size_t *out_size) {
  return lvkw_wnd_getClipboardData_X11(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  return lvkw_wnd_getClipboardMimeTypes_X11(window, out_mime_types, count);
}

#endif