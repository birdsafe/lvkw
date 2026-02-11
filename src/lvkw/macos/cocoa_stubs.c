#include "lvkw_api_checks.h"
#include "lvkw_macos_internal.h"

#include <vulkan/vulkan.h>

LVKW_Status lvkw_ctx_create_Cocoa(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;
  LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_BACKEND_UNAVAILABLE, "MacOS backend is currently stubbed");
  return LVKW_ERROR;
}

void lvkw_ctx_destroy_Cocoa(LVKW_Context *ctx_handle) {
  (void)ctx_handle;
}

const char *const *lvkw_ctx_getVkExtensions_Cocoa(LVKW_Context *ctx_handle, uint32_t *count) {
  (void)ctx_handle;
  if (count) *count = 0;
  return NULL;
}

LVKW_Status lvkw_ctx_pollEvents_Cocoa(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata) {
  (void)ctx_handle;
  (void)event_mask;
  (void)callback;
  (void)userdata;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_waitEvents_Cocoa(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                      LVKW_EventType event_mask, LVKW_EventCallback callback,
                                      void *userdata) {
  (void)ctx_handle;
  (void)timeout_ms;
  (void)event_mask;
  (void)callback;
  (void)userdata;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_update_Cocoa(LVKW_Context *ctx_handle, uint32_t field_mask,
                                  const LVKW_ContextAttributes *attributes) {
  (void)ctx_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_createWindow_Cocoa(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window_handle) {
  (void)ctx_handle;
  (void)create_info;
  *out_window_handle = NULL;
  return LVKW_ERROR;
}

void lvkw_wnd_destroy_Cocoa(LVKW_Window *window_handle) {
  (void)window_handle;
}

LVKW_Status lvkw_wnd_createVkSurface_Cocoa(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  (void)window_handle;
  (void)instance;
  *out_surface = VK_NULL_HANDLE;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getGeometry_Cocoa(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  (void)window_handle;
  (void)out_geometry;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_update_Cocoa(LVKW_Window *window_handle, uint32_t field_mask,
                                  const LVKW_WindowAttributes *attributes) {
  (void)window_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_requestFocus_Cocoa(LVKW_Window *window_handle) {
  (void)window_handle;
  return LVKW_ERROR;
}
