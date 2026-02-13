#include "lvkw_api_constraints.h"
#include "lvkw_macos_internal.h"

#include <vulkan/vulkan.h>

LVKW_Status lvkw_ctx_create_Cocoa(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;
  LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE, "MacOS backend is currently stubbed");
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_destroy_Cocoa(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  (void)ctx_handle;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getVkExtensions_Cocoa(LVKW_Context *ctx_handle, uint32_t *count,
                                           const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  (void)ctx_handle;
  if (count) *count = 0;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_pollEvents_Cocoa(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_pollEvents, ctx_handle, event_mask, callback, userdata);
  (void)ctx_handle;
  (void)event_mask;
  (void)callback;
  (void)userdata;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_waitEvents_Cocoa(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                      LVKW_EventType event_mask, LVKW_EventCallback callback,
                                      void *userdata) {
  LVKW_API_VALIDATE(ctx_waitEvents, ctx_handle, timeout_ms, event_mask, callback, userdata);
  (void)ctx_handle;
  (void)timeout_ms;
  (void)event_mask;
  (void)callback;
  (void)userdata;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_update_Cocoa(LVKW_Context *ctx_handle, uint32_t field_mask,
                                  const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  (void)ctx_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_getMonitors_Cocoa(LVKW_Context *ctx_handle, LVKW_Monitor **out_monitors, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_monitors, count);
  (void)ctx_handle;
  (void)out_monitors;
  (void)count;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_getMonitorModes_Cocoa(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                           LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  (void)ctx_handle;
  (void)monitor;
  (void)out_modes;
  (void)count;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_createWindow_Cocoa(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  (void)ctx_handle;
  (void)create_info;
  *out_window_handle = NULL;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_destroy_Cocoa(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  (void)window_handle;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_createVkSurface_Cocoa(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  (void)window_handle;
  (void)instance;
  *out_surface = VK_NULL_HANDLE;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getGeometry_Cocoa(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  (void)window_handle;
  (void)out_geometry;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_update_Cocoa(LVKW_Window *window_handle, uint32_t field_mask,
                                  const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  (void)window_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_requestFocus_Cocoa(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  (void)window_handle;
  return LVKW_ERROR;
}
