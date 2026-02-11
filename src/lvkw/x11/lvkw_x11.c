#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
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
        },
};

#else

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  lvkw_check_createContext(create_info, out_context);
  return lvkw_ctx_create_X11(create_info, out_context);
}
void lvkw_ctx_destroy(LVKW_Context *handle) {
  lvkw_check_ctx_destroy(handle);
  lvkw_ctx_destroy_X11(handle);
}
const char *const *lvkw_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count) {
  lvkw_check_ctx_getVkExtensions(ctx, count);
  return lvkw_ctx_getVkExtensions_X11(ctx, count);
}
LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                    void *userdata) {
  lvkw_check_ctx_pollEvents(ctx, event_mask, callback, userdata);
  return lvkw_ctx_pollEvents_X11(ctx, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_X11(ctx, timeout_ms, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_update(LVKW_Context *ctx, uint32_t field_mask,
                                          const LVKW_ContextAttributes *attributes) {
  lvkw_check_ctx_update(ctx, field_mask, attributes);
  return lvkw_ctx_update_X11(ctx, field_mask, attributes);
}
LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  lvkw_check_ctx_getMonitors(ctx, out_monitors, count);
  return lvkw_ctx_getMonitors_X11(ctx, out_monitors, count);
}
LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx, LVKW_MonitorId monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  lvkw_check_ctx_getMonitorModes(ctx, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_X11(ctx, monitor, out_modes, count);
}
LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info, LVKW_Window **out_window) {
  lvkw_check_ctx_createWindow(ctx, create_info, out_window);
  return lvkw_ctx_createWindow_X11(ctx, create_info, out_window);
}
void lvkw_wnd_destroy(LVKW_Window *handle) {
  lvkw_check_wnd_destroy(handle);
  lvkw_wnd_destroy_X11(handle);
}
LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface) {
  lvkw_check_wnd_createVkSurface(window, instance, out_surface);
  return lvkw_wnd_createVkSurface_X11(window, instance, out_surface);
}
LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window, LVKW_WindowGeometry *out_geometry) {
  lvkw_check_wnd_getGeometry(window, out_geometry);
  return lvkw_wnd_getGeometry_X11(window, out_geometry);
}
LVKW_Status lvkw_wnd_update(LVKW_Window *window, uint32_t field_mask,
                                          const LVKW_WindowAttributes *attributes) {
  lvkw_check_wnd_update(window, field_mask, attributes);
  return lvkw_wnd_update_X11(window, field_mask, attributes);
}
LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window) {
  lvkw_check_wnd_requestFocus(window);
  return lvkw_wnd_requestFocus_X11(window);
}

#endif