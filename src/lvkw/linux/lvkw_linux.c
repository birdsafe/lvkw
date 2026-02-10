#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_internal.h"

// N.B.
// The lack of argument checking here is INTENTIONAL. The task is entirely
// delegated to the selected backend.

// Forward declarations for backend-specific creation functions
LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  lvkw_check_createContext(create_info, out_ctx_handle);

  *out_ctx_handle = NULL;

  LVKW_BackendType backend = create_info->backend;

  if (backend == LVKW_BACKEND_WAYLAND || backend == LVKW_BACKEND_AUTO) {
    LVKW_Status res = lvkw_ctx_create_WL(create_info, out_ctx_handle);
    if (res == LVKW_SUCCESS) {
      return res;
    }
  }

  if (backend == LVKW_BACKEND_X11 || backend == LVKW_BACKEND_AUTO) {
    return lvkw_ctx_create_X11(create_info, out_ctx_handle);
  }

  // Default probing (LVKW_BACKEND_AUTO): try Wayland then X11
  return LVKW_ERROR;
}

void lvkw_ctx_destroy(LVKW_Context *ctx_handle) {
  lvkw_check_ctx_destroy(ctx_handle);

  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;

  ctx_base->prv.backend->context.destroy(ctx_handle);
}

const char *const *lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count) {
  lvkw_check_ctx_getVkExtensions(ctx_handle, count);

  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  return ctx_base->prv.backend->context.get_vulkan_instance_extensions(ctx_handle, count);
}

LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  lvkw_check_ctx_pollEvents(ctx_handle, event_mask, callback, userdata);

  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  return ctx_base->prv.backend->context.poll_events(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_waitEvents(ctx_handle, timeout_ms, event_mask, callback, userdata);

  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  return ctx_base->prv.backend->context.wait_events(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask,
                                          const LVKW_ContextAttributes *attributes) {
  lvkw_check_ctx_update(ctx_handle, field_mask, attributes);

  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;

  return ctx_base->prv.backend->context.update(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  lvkw_check_ctx_createWindow(ctx_handle, create_info, out_window_handle);

  *out_window_handle = NULL;

  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;

  return ctx_base->prv.backend->window.create(ctx_handle, create_info, out_window_handle);
}

void lvkw_wnd_destroy(LVKW_Window *window_handle) {
  lvkw_check_wnd_destroy(window_handle);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  window_base->prv.backend->window.destroy(window_handle);
}

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  lvkw_check_wnd_createVkSurface(window_handle, instance, out_surface);

  *out_surface = VK_NULL_HANDLE;

  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;

  return window_base->prv.backend->window.create_vk_surface(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getFramebufferSize(LVKW_Window *window_handle, LVKW_Size *out_size) {
  lvkw_check_wnd_getFramebufferSize(window_handle, out_size);

  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;

  return window_base->prv.backend->window.get_framebuffer_size(window_handle, out_size);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask,
                                          const LVKW_WindowAttributes *attributes) {
  lvkw_check_wnd_update(window_handle, field_mask, attributes);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  return window_base->prv.backend->window.update(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  lvkw_check_wnd_requestFocus(window_handle);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  return window_base->prv.backend->window.request_focus(window_handle);
}
