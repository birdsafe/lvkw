#include "lvkw_api_checks.h"
#include "lvkw_win32_internal.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

LVKW_Status lvkw_ctx_createWindow_Win32(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window_handle) {
  (void)ctx_handle;
  (void)create_info;
  *out_window_handle = NULL;
  return LVKW_ERROR;
}

void lvkw_wnd_destroy_Win32(LVKW_Window *window_handle) {
  (void)window_handle;
}

LVKW_Status lvkw_wnd_createVkSurface_Win32(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  (void)window_handle;
  (void)instance;
  *out_surface = VK_NULL_HANDLE;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getGeometry_Win32(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  (void)window_handle;
  (void)out_geometry;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_update_Win32(LVKW_Window *window_handle, uint32_t field_mask,
                                  const LVKW_WindowAttributes *attributes) {
  (void)window_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_requestFocus_Win32(LVKW_Window *window_handle) {
  (void)window_handle;
  return LVKW_ERROR;
}
