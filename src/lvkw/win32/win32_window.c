// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_api_constraints.h"
#include "lvkw_win32_internal.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

LVKW_Status lvkw_ctx_createWindow_Win32(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  (void)ctx_handle;
  (void)create_info;
  *out_window_handle = NULL;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_destroy_Win32(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  (void)window_handle;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_createVkSurface_Win32(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  (void)window_handle;
  (void)instance;
  *out_surface = VK_NULL_HANDLE;

  // IMPLEMENTATION NOTE:
  // This function should first check ctx->base.prv.vk_loader.
  // If NULL, it should attempt to retrieve 'vkGetInstanceProcAddr' from 'vulkan-1.dll'
  // using GetModuleHandle("vulkan-1.dll") and GetProcAddress.
  // This ensures support for both implicitly linked (lib) and manually loaded (dll) Vulkan setups.

  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getGeometry_Win32(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  (void)window_handle;
  (void)out_geometry;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_update_Win32(LVKW_Window *window_handle, uint32_t field_mask,
                                  const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  (void)window_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_requestFocus_Win32(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  (void)window_handle;
  return LVKW_ERROR;
}
