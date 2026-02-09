#include "lvkw_api_checks.h"
#include "lvkw_wayland_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
const LVKW_Backend _lvkw_wayland_backend = {
    .context =
        {
            .destroy = lvkw_context_destroy_WL,
            .get_vulkan_instance_extensions = lvkw_context_getVulkanInstanceExtensions_WL,
            .poll_events = lvkw_context_pollEvents_WL,
            .set_idle_timeout = lvkw_context_setIdleTimeout_WL,
            .get_user_data = lvkw_context_getUserData_WL,
        },
    .window =
        {
            .create = lvkw_window_create_WL,
            .destroy = lvkw_window_destroy_WL,
            .create_vk_surface = lvkw_window_createVkSurface_WL,
            .get_framebuffer_size = lvkw_window_getFramebufferSize_WL,
            .get_user_data = lvkw_window_getUserData_WL,
            .set_fullscreen = lvkw_window_setFullscreen_WL,
            .set_cursor_mode = lvkw_window_setCursorMode_WL,
            .set_cursor_shape = lvkw_window_setCursorShape_WL,
            .request_focus = lvkw_window_requestFocus_WL,
        },
};
#else
LVKW_Status lvkw_context_create(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  lvkw_check_context_create(create_info, out_ctx_handle);
  return _lvkw_context_create_WL(create_info, out_ctx_handle);
}
void lvkw_context_destroy(LVKW_Context *ctx_handle) {
  lvkw_check_context_destroy(ctx_handle);
  lvkw_context_destroy_WL(ctx_handle);
}
void *lvkw_context_getUserData(const LVKW_Context *ctx_handle) {
  lvkw_check_context_getUserData(ctx_handle);
  return lvkw_context_getUserData_WL(ctx_handle);
}
void lvkw_context_getVulkanInstanceExtensions(const LVKW_Context *ctx_handle, uint32_t *count,
                                              const char **out_extensions) {
  lvkw_check_context_getVulkanInstanceExtensions(ctx_handle, count, out_extensions);
  lvkw_context_getVulkanInstanceExtensions_WL(ctx_handle, count, out_extensions);
}
LVKW_ContextResult lvkw_context_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                           LVKW_EventCallback callback, void *userdata) {
  lvkw_check_context_pollEvents(ctx_handle, event_mask, callback, userdata);
  return lvkw_context_pollEvents_WL(ctx_handle, event_mask, callback, userdata);
}
LVKW_Status lvkw_context_setIdleTimeout(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  lvkw_check_context_setIdleTimeout(ctx_handle, timeout_ms);
  return lvkw_context_setIdleTimeout_WL(ctx_handle, timeout_ms);
}
LVKW_ContextResult lvkw_window_create(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window_handle) {
  lvkw_check_window_create(ctx_handle, create_info, out_window_handle);
  return lvkw_window_create_WL(ctx_handle, create_info, out_window_handle);
}
void lvkw_window_destroy(LVKW_Window *window_handle) {
  lvkw_check_window_destroy(window_handle);
  lvkw_window_destroy_WL(window_handle);
}
LVKW_WindowResult lvkw_window_createVkSurface(const LVKW_Window *window_handle, VkInstance instance,
                                              VkSurfaceKHR *out_surface) {
  lvkw_check_window_createVkSurface(window_handle, instance, out_surface);
  return lvkw_window_createVkSurface_WL(window_handle, instance, out_surface);
}
LVKW_WindowResult lvkw_window_getFramebufferSize(const LVKW_Window *window_handle, LVKW_Size *out_size) {
  lvkw_check_window_getFramebufferSize(window_handle, out_size);
  return lvkw_window_getFramebufferSize_WL(window_handle, out_size);
}
void *lvkw_window_getUserData(const LVKW_Window *window_handle) {
  lvkw_check_window_getUserData(window_handle);
  return lvkw_window_getUserData_WL(window_handle);
}
LVKW_WindowResult lvkw_window_setFullscreen(LVKW_Window *window_handle, bool enabled) {
  lvkw_check_window_setFullscreen(window_handle, enabled);
  return lvkw_window_setFullscreen_WL(window_handle, enabled);
}
LVKW_Status lvkw_window_setCursorMode(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  lvkw_check_window_setCursorMode(window_handle, mode);
  return lvkw_window_setCursorMode_WL(window_handle, mode);
}
LVKW_Status lvkw_window_setCursorShape(LVKW_Window *window_handle, LVKW_CursorShape shape) {
  lvkw_check_window_setCursorShape(window_handle, shape);
  return lvkw_window_setCursorShape_WL(window_handle, shape);
}
LVKW_Status lvkw_window_requestFocus(LVKW_Window *window_handle) {
  lvkw_check_window_requestFocus(window_handle);
  return lvkw_window_requestFocus_WL(window_handle);
}
#endif
