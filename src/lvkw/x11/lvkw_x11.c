#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND

const LVKW_Backend _lvkw_x11_backend = {
    .context =
        {
            .destroy = lvkw_ctx_destroy_X11,
            .get_vulkan_instance_extensions = lvkw_ctx_getVkExtensions_X11,
            .poll_events = lvkw_ctx_pollEvents_X11,
            .wait_events = lvkw_ctx_waitEvents_X11,
            .set_idle_timeout = lvkw_ctx_setIdleTimeout_X11,
        },
    .window =
        {
            .create = lvkw_ctx_createWindow_X11,
            .destroy = lvkw_wnd_destroy_X11,
            .create_vk_surface = lvkw_wnd_createVkSurface_X11,
            .get_framebuffer_size = lvkw_wnd_getFramebufferSize_X11,
            .set_fullscreen = lvkw_wnd_setFullscreen_X11,
            .set_cursor_mode = lvkw_wnd_setCursorMode_X11,
            .set_cursor_shape = lvkw_wnd_setCursorShape_X11,
            .request_focus = lvkw_wnd_requestFocus_X11,
        },
};

#else

LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context) {
  lvkw_check_createContext(create_info, out_context);
  return lvkw_ctx_create_X11(create_info, out_context);
}
void lvkw_ctx_destroy(LVKW_Context *handle) {
  lvkw_check_ctx_destroy(handle);
  lvkw_ctx_destroy_X11(handle);
}
void lvkw_ctx_getVkExtensions(LVKW_Context *ctx, uint32_t *count, const char **out_extensions) {
  lvkw_check_ctx_getVkExtensions(ctx, count, out_extensions);
  lvkw_ctx_getVkExtensions_X11(ctx, count, out_extensions);
}
LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                    void *userdata) {
  lvkw_check_ctx_pollEvents(ctx, event_mask, callback, userdata);
  return lvkw_ctx_pollEvents_X11(ctx, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_waitEvents(ctx, timeout_ms, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_X11(ctx, timeout_ms, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  lvkw_check_ctx_setIdleTimeout(ctx, timeout_ms);
  return lvkw_ctx_setIdleTimeout_X11(ctx, timeout_ms);
}
LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info, LVKW_Window **out_window) {
  lvkw_check_ctx_createWindow(ctx, create_info, out_window);
  return lvkw_ctx_createWindow_X11(ctx, create_info, out_window);
}
void lvkw_wnd_destroy(LVKW_Window *handle) {
  lvkw_check_wnd_destroy(handle);
  lvkw_wnd_destroy_X11(handle);
}
LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface) {
  lvkw_check_wnd_createVkSurface(window, instance, out_surface);
  return lvkw_wnd_createVkSurface_X11(window, instance, out_surface);
}
LVKW_Status lvkw_wnd_getFramebufferSize(LVKW_Window *window, LVKW_Size *out_size) {
  lvkw_check_wnd_getFramebufferSize(window, out_size);
  return lvkw_wnd_getFramebufferSize_X11(window, out_size);
}
LVKW_Status lvkw_wnd_setFullscreen(LVKW_Window *window, bool enabled) {
  lvkw_check_wnd_setFullscreen(window, enabled);
  return lvkw_wnd_setFullscreen_X11(window, enabled);
}
LVKW_Status lvkw_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  lvkw_check_wnd_setCursorMode(window, mode);
  return lvkw_wnd_setCursorMode_X11(window, mode);
}
LVKW_Status lvkw_wnd_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  lvkw_check_wnd_setCursorShape(window, shape);
  return lvkw_wnd_setCursorShape_X11(window, shape);
}
LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window) {
  lvkw_check_wnd_requestFocus(window);
  return lvkw_wnd_requestFocus_X11(window);
}

#endif