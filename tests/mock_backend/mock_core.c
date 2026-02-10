#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_mock_internal.h"

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  lvkw_check_createContext(create_info, out_ctx_handle);

  LVKW_BackendType backend = create_info->backend;

  // In mock-only builds, only AUTO backend selection is valid
  if (backend != LVKW_BACKEND_AUTO) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_BACKEND_FAILURE,
                                    "Only AUTO backend selection available in mock build");
    return LVKW_ERROR;
  }

  return lvkw_ctx_create_Mock(create_info, out_ctx_handle);
}

void lvkw_ctx_destroy(LVKW_Context *ctx_handle) {
  lvkw_check_ctx_destroy(ctx_handle);
  lvkw_ctx_destroy_Mock(ctx_handle);
}

void lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                              const char **out_extensions) {
  lvkw_check_ctx_getVkExtensions(ctx_handle, count, out_extensions);
  lvkw_ctx_getVkExtensions_Mock(ctx_handle, count, out_extensions);
}

LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                            LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_pollEvents(ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_pollEvents_Mock(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                            LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_waitEvents(ctx_handle, timeout_ms, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_Mock(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_updateAttributes(LVKW_Context *ctx_handle, uint32_t field_mask,
                                          const LVKW_ContextAttributes *attributes) {
  lvkw_check_ctx_updateAttributes(ctx_handle, field_mask, attributes);
  return lvkw_ctx_updateAttributes_Mock(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window_handle) {
  lvkw_check_ctx_createWindow(ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_Mock(ctx_handle, create_info, out_window_handle);
}

void lvkw_wnd_destroy(LVKW_Window *window_handle) {
  lvkw_check_wnd_destroy(window_handle);
  lvkw_wnd_destroy_Mock(window_handle);
}

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                              VkSurfaceKHR *out_surface) {
  lvkw_check_wnd_createVkSurface(window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_Mock(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getFramebufferSize(LVKW_Window *window_handle, LVKW_Size *out_size) {
  lvkw_check_wnd_getFramebufferSize(window_handle, out_size);
  return lvkw_wnd_getFramebufferSize_Mock(window_handle, out_size);
}

LVKW_Status lvkw_wnd_updateAttributes(LVKW_Window *window_handle, uint32_t field_mask,
                                          const LVKW_WindowAttributes *attributes) {
  lvkw_check_wnd_updateAttributes(window_handle, field_mask, attributes);
  return lvkw_wnd_updateAttributes_Mock(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_setFullscreen(LVKW_Window *window_handle, bool enabled) {
  lvkw_check_wnd_setFullscreen(window_handle, enabled);
  return lvkw_wnd_setFullscreen_Mock(window_handle, enabled);
}

LVKW_Status lvkw_wnd_setCursorMode(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  lvkw_check_wnd_setCursorMode(window_handle, mode);
  return lvkw_wnd_setCursorMode_Mock(window_handle, mode);
}

LVKW_Status lvkw_wnd_setCursorShape(LVKW_Window *window_handle, LVKW_CursorShape shape) {
  lvkw_check_wnd_setCursorShape(window_handle, shape);
  return lvkw_wnd_setCursorShape_Mock(window_handle, shape);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  lvkw_check_wnd_requestFocus(window_handle);
  return lvkw_wnd_requestFocus_Mock(window_handle);
}