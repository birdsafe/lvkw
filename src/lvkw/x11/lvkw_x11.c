#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND

const LVKW_Backend _lvkw_x11_backend = {
    .context =
        {
            .destroy = lvkw_destroyContext_X11,
            .get_vulkan_instance_extensions = lvkw_context_getVulkanInstanceExtensions_X11,
            .poll_events = lvkw_context_pollEvents_X11,
            .wait_events = lvkw_context_waitEvents_X11,
            .set_idle_timeout = lvkw_context_setIdleTimeout_X11,
        },
    .window =
        {
            .create = lvkw_context_createWindow_X11,
            .destroy = lvkw_destroyWindow_X11,
            .create_vk_surface = lvkw_window_createVkSurface_X11,
            .get_framebuffer_size = lvkw_window_getFramebufferSize_X11,
            .set_fullscreen = lvkw_window_setFullscreen_X11,
            .set_cursor_mode = lvkw_window_setCursorMode_X11,
            .set_cursor_shape = lvkw_window_setCursorShape_X11,
            .request_focus = lvkw_window_requestFocus_X11,
        },
};

#else

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  lvkw_check_createContext(create_info, out_context);
  return lvkw_createContext_X11(create_info, out_context);
}
void lvkw_destroyContext(LVKW_Context *handle) {
  lvkw_check_destroyContext(handle);
  lvkw_destroyContext_X11(handle);
}
void lvkw_context_getVulkanInstanceExtensions(LVKW_Context *ctx, uint32_t *count, const char **out_extensions) {
  lvkw_check_context_getVulkanInstanceExtensions(ctx, count, out_extensions);
  lvkw_context_getVulkanInstanceExtensions_X11(ctx, count, out_extensions);
}
LVKW_Status lvkw_context_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                    void *userdata) {
  lvkw_check_context_pollEvents(ctx, event_mask, callback, userdata);
  return lvkw_context_pollEvents_X11(ctx, event_mask, callback, userdata);
}
LVKW_Status lvkw_context_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata) {
  lvkw_check_context_pollEvents(ctx, event_mask, callback, userdata);
  return lvkw_context_waitEvents_X11(ctx, timeout_ms, event_mask, callback, userdata);
}
LVKW_Status lvkw_context_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  lvkw_check_context_setIdleTimeout(ctx, timeout_ms);
  return lvkw_context_setIdleTimeout_X11(ctx, timeout_ms);
}
LVKW_Status lvkw_context_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info, LVKW_Window **out_window) {
  lvkw_check_context_createWindow(ctx, create_info, out_window);
  return lvkw_context_createWindow_X11(ctx, create_info, out_window);
}
void lvkw_destroyWindow(LVKW_Window *handle) {
  lvkw_check_destroyWindow(handle);
  lvkw_destroyWindow_X11(handle);
}
LVKW_Status lvkw_window_createVkSurface(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface) {
  lvkw_check_window_createVkSurface(window, instance, out_surface);
  return lvkw_window_createVkSurface_X11(window, instance, out_surface);
}
LVKW_Status lvkw_window_getFramebufferSize(LVKW_Window *window, LVKW_Size *out_size) {
  lvkw_check_window_getFramebufferSize(window, out_size);
  return lvkw_window_getFramebufferSize_X11(window, out_size);
}
LVKW_Status lvkw_window_setFullscreen(LVKW_Window *window, bool enabled) {
  lvkw_check_window_setFullscreen(window, enabled);
  return lvkw_window_setFullscreen_X11(window, enabled);
}
LVKW_Status lvkw_window_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  lvkw_check_window_setCursorMode(window, mode);
  return lvkw_window_setCursorMode_X11(window, mode);
}
LVKW_Status lvkw_window_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  lvkw_check_window_setCursorShape(window, shape);
  return lvkw_window_setCursorShape_X11(window, shape);
}
LVKW_Status lvkw_window_requestFocus(LVKW_Window *window) {
  lvkw_check_window_requestFocus(window);
  return lvkw_window_requestFocus_X11(window);
}

#endif