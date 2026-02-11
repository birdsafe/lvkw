#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_win32_internal.h"

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  lvkw_check_createContext(create_info, out_ctx_handle);
  return lvkw_ctx_create_Win32(create_info, out_ctx_handle);
}

void lvkw_ctx_destroy(LVKW_Context *ctx_handle) {
  lvkw_check_ctx_destroy(ctx_handle);
  lvkw_ctx_destroy_Win32(ctx_handle);
}

const char *const *lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count) {
  lvkw_check_ctx_getVkExtensions(ctx_handle, count);
  return lvkw_ctx_getVkExtensions_Win32(ctx_handle, count);
}

LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  lvkw_check_ctx_pollEvents(ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_pollEvents_Win32(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_waitEvents(ctx_handle, timeout_ms, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_Win32(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  lvkw_check_ctx_createWindow(ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_Win32(ctx_handle, create_info, out_window_handle);
}

void lvkw_wnd_destroy(LVKW_Window *window_handle) {
  lvkw_check_wnd_destroy(window_handle);
  lvkw_wnd_destroy_Win32(window_handle);
}

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  lvkw_check_wnd_createVkSurface(window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_Win32(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  lvkw_check_wnd_getGeometry(window_handle, out_geometry);
  return lvkw_wnd_getGeometry_Win32(window_handle, out_geometry);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {
  lvkw_check_wnd_update(window_handle, field_mask, attributes);
  return lvkw_wnd_update_Win32(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  lvkw_check_wnd_requestFocus(window_handle);
  return lvkw_wnd_requestFocus_Win32(window_handle);
}