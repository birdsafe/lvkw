#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_mock_internal.h"

LVKW_Status lvkw_context_create(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  lvkw_check_context_create(create_info, out_ctx_handle);

  LVKW_BackendType backend = create_info->backend;

  // In mock-only builds, only AUTO backend selection is valid
  if (backend != LVKW_BACKEND_AUTO) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_BACKEND_FAILURE,
                                    "Only AUTO backend selection available in mock build");
    return LVKW_ERROR_NOOP;
  }

  return lvkw_context_create_Mock(create_info, out_ctx_handle);
}

void lvkw_context_destroy(LVKW_Context *ctx_handle) {
  lvkw_check_context_destroy(ctx_handle);
  lvkw_context_destroy_Mock(ctx_handle);
}

void lvkw_context_getVulkanInstanceExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                              const char **out_extensions) {
  lvkw_check_context_getVulkanInstanceExtensions(ctx_handle, count, out_extensions);
  lvkw_context_getVulkanInstanceExtensions_Mock(ctx_handle, count, out_extensions);
}

LVKW_ContextResult lvkw_context_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                            LVKW_EventCallback callback, void *userdata) {
  lvkw_check_context_pollEvents(ctx_handle, event_mask, callback, userdata);
  return lvkw_context_pollEvents_Mock(ctx_handle, event_mask, callback, userdata);
}

LVKW_ContextResult lvkw_context_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                            LVKW_EventCallback callback, void *userdata) {
  lvkw_check_context_pollEvents(ctx_handle, event_mask, callback, userdata);
  return lvkw_context_waitEvents_Mock(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_context_setIdleTimeout(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  lvkw_check_context_setIdleTimeout(ctx_handle, timeout_ms);
  return lvkw_context_setIdleTimeout_Mock(ctx_handle, timeout_ms);
}

LVKW_ContextResult lvkw_window_create(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window_handle) {
  lvkw_check_window_create(ctx_handle, create_info, out_window_handle);
  return lvkw_window_create_Mock(ctx_handle, create_info, out_window_handle);
}

void lvkw_window_destroy(LVKW_Window *window_handle) {
  lvkw_check_window_destroy(window_handle);
  lvkw_window_destroy_Mock(window_handle);
}

LVKW_WindowResult lvkw_window_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                              VkSurfaceKHR *out_surface) {
  lvkw_check_window_createVkSurface(window_handle, instance, out_surface);
  return lvkw_window_createVkSurface_Mock(window_handle, instance, out_surface);
}

LVKW_WindowResult lvkw_window_getFramebufferSize(LVKW_Window *window_handle, LVKW_Size *out_size) {
  lvkw_check_window_getFramebufferSize(window_handle, out_size);
  return lvkw_window_getFramebufferSize_Mock(window_handle, out_size);
}

LVKW_WindowResult lvkw_window_setFullscreen(LVKW_Window *window_handle, bool enabled) {
  lvkw_check_window_setFullscreen(window_handle, enabled);
  return lvkw_window_setFullscreen_Mock(window_handle, enabled);
}

LVKW_Status lvkw_window_setCursorMode(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  lvkw_check_window_setCursorMode(window_handle, mode);
  return lvkw_window_setCursorMode_Mock(window_handle, mode);
}

LVKW_Status lvkw_window_setCursorShape(LVKW_Window *window_handle, LVKW_CursorShape shape) {
  lvkw_check_window_setCursorShape(window_handle, shape);
  return lvkw_window_setCursorShape_Mock(window_handle, shape);
}

LVKW_Status lvkw_window_requestFocus(LVKW_Window *window_handle) {
  lvkw_check_window_requestFocus(window_handle);
  return lvkw_window_requestFocus_Mock(window_handle);
}
