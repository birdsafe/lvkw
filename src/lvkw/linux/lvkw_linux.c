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
LVKW_Status _lvkw_context_create_WL(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
LVKW_Status _lvkw_context_create_X11(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);

LVKW_Status lvkw_context_create(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  lvkw_check_context_create(create_info, out_ctx_handle);

  *out_ctx_handle = NULL;

  LVKW_BackendType backend = create_info->backend;

  if (backend == LVKW_BACKEND_WAYLAND || backend == LVKW_BACKEND_AUTO) {
    auto res = _lvkw_context_create_WL(create_info, out_ctx_handle);
    if (res == LVKW_OK) {
      return res;
    }
  }

  if (backend == LVKW_BACKEND_X11 || backend == LVKW_BACKEND_AUTO) {
    return _lvkw_context_create_X11(create_info, out_ctx_handle);
  }

  // Default probing (LVKW_BACKEND_AUTO): try Wayland then X11
  return LVKW_RESULT_CONTEXT_LOST_BIT;
}

void lvkw_context_destroy(LVKW_Context *ctx_handle) {
  lvkw_check_context_destroy(ctx_handle);

  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;

  ctx_base->backend->context.destroy(ctx_handle);
}

void *lvkw_context_getUserData(const LVKW_Context *ctx_handle) {
  lvkw_check_context_getUserData(ctx_handle);

  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  return ctx_base->backend->context.get_user_data(ctx_handle);
}

void lvkw_context_getVulkanInstanceExtensions(const LVKW_Context *ctx_handle, uint32_t *count,
                                              const char **out_extensions) {
  lvkw_check_context_getVulkanInstanceExtensions(ctx_handle, count, out_extensions);

  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  ctx_base->backend->context.get_vulkan_instance_extensions(ctx_handle, count, out_extensions);
}

LVKW_ContextResult lvkw_context_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                           LVKW_EventCallback callback,

                                           void *userdata) {
  lvkw_check_context_pollEvents(ctx_handle, event_mask, callback, userdata);

  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  return ctx_base->backend->context.poll_events(ctx_handle, event_mask, callback, userdata);
}

LVKW_ContextResult lvkw_context_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                           LVKW_EventCallback callback, void *userdata) {
  lvkw_check_context_pollEvents(ctx_handle, event_mask, callback, userdata);

  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;

  return ctx_base->backend->context.wait_events(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_context_setIdleTimeout(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  lvkw_check_context_setIdleTimeout(ctx_handle, timeout_ms);

  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;

  if (ctx_base->backend->context.set_idle_timeout) {
    return ctx_base->backend->context.set_idle_timeout(ctx_handle, timeout_ms);
  }

  LVKW_REPORT_CTX_DIAGNOSIS(ctx_handle, LVKW_DIAGNOSIS_FEATURE_UNSUPPORTED, "Idle timeout not supported");

  return LVKW_ERROR_NOOP;
}

LVKW_ContextResult lvkw_window_create(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,

                                      LVKW_Window **out_window_handle) {
  lvkw_check_window_create(ctx_handle, create_info, out_window_handle);

  *out_window_handle = NULL;

  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;

  return ctx_base->backend->window.create(ctx_handle, create_info, out_window_handle);
}

void lvkw_window_destroy(LVKW_Window *window_handle) {
  lvkw_check_window_destroy(window_handle);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  window_base->backend->window.destroy(window_handle);
}

LVKW_WindowResult lvkw_window_createVkSurface(const LVKW_Window *window_handle, VkInstance instance,

                                              VkSurfaceKHR *out_surface) {
  lvkw_check_window_createVkSurface(window_handle, instance, out_surface);

  *out_surface = VK_NULL_HANDLE;

  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;

  return window_base->backend->window.create_vk_surface(window_handle, instance, out_surface);
}

LVKW_WindowResult lvkw_window_getFramebufferSize(const LVKW_Window *window_handle, LVKW_Size *out_size) {
  lvkw_check_window_getFramebufferSize(window_handle, out_size);

  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;

  return window_base->backend->window.get_framebuffer_size(window_handle, out_size);
}

void *lvkw_window_getUserData(const LVKW_Window *window_handle) {
  lvkw_check_window_getUserData(window_handle);

  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;

  return window_base->backend->window.get_user_data(window_handle);
}

LVKW_WindowResult lvkw_window_setFullscreen(LVKW_Window *window_handle, bool enabled) {
  lvkw_check_window_setFullscreen(window_handle, enabled);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  return window_base->backend->window.set_fullscreen(window_handle, enabled);
}

LVKW_Status lvkw_window_setCursorMode(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  lvkw_check_window_setCursorMode(window_handle, mode);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  return window_base->backend->window.set_cursor_mode(window_handle, mode);
}

LVKW_Status lvkw_window_setCursorShape(LVKW_Window *window_handle, LVKW_CursorShape shape) {
  lvkw_check_window_setCursorShape(window_handle, shape);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  return window_base->backend->window.set_cursor_shape(window_handle, shape);
}

LVKW_Status lvkw_window_requestFocus(LVKW_Window *window_handle) {
  lvkw_check_window_requestFocus(window_handle);

  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;

  return window_base->backend->window.request_focus(window_handle);
}
