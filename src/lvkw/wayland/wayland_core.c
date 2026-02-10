#include "lvkw_api_checks.h"
#include "lvkw_wayland_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
const LVKW_Backend _lvkw_wayland_backend = {
    .context =
        {
            .destroy = lvkw_ctx_destroy_WL,
            .get_vulkan_instance_extensions = lvkw_ctx_getVkExtensions_WL,
            .poll_events = lvkw_ctx_pollEvents_WL,
            .wait_events = lvkw_ctx_waitEvents_WL,
            .update = lvkw_ctx_update_WL,
        },

    .window =
        {
            .create = lvkw_ctx_createWindow_WL,
            .destroy = lvkw_wnd_destroy_WL,
            .create_vk_surface = lvkw_wnd_createVkSurface_WL,
            .get_framebuffer_size = lvkw_wnd_getFramebufferSize_WL,
            .update = lvkw_wnd_update_WL,
            .request_focus = lvkw_wnd_requestFocus_WL,
        },
};
#else
LVKW_Status lvkw_createContext(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  lvkw_check_createContext(create_info, out_ctx_handle);
  return lvkw_ctx_create_WL(create_info, out_ctx_handle);
}
void lvkw_ctx_destroy(LVKW_Context *ctx_handle) {
  lvkw_check_ctx_destroy(ctx_handle);
  lvkw_ctx_destroy_WL(ctx_handle);
}

void lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                              const char **out_extensions) {
  lvkw_check_ctx_getVkExtensions(ctx_handle, count, out_extensions);
  lvkw_ctx_getVkExtensions_WL(ctx_handle, count, out_extensions);
}
LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                            LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_pollEvents(ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_pollEvents_WL(ctx_handle, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                            LVKW_EventCallback callback, void *userdata) {
  lvkw_check_ctx_waitEvents(ctx_handle, timeout_ms, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_WL(ctx_handle, timeout_ms, event_mask, callback, userdata);
}
LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask,
                                          const LVKW_ContextAttributes *attributes) {
  lvkw_check_ctx_update(ctx_handle, field_mask, attributes);
  return lvkw_ctx_update_WL(ctx_handle, field_mask, attributes);
}
LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window_handle) {
  lvkw_check_ctx_createWindow(ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_WL(ctx_handle, create_info, out_window_handle);
}
void lvkw_wnd_destroy(LVKW_Window *window_handle) {
  lvkw_check_wnd_destroy(window_handle);
  lvkw_wnd_destroy_WL(window_handle);
}
LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                              VkSurfaceKHR *out_surface) {
  lvkw_check_wnd_createVkSurface(window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_WL(window_handle, instance, out_surface);
}
LVKW_Status lvkw_wnd_getFramebufferSize(LVKW_Window *window_handle, LVKW_Size *out_size) {
  lvkw_check_wnd_getFramebufferSize(window_handle, out_size);
  return lvkw_wnd_getFramebufferSize_WL(window_handle, out_size);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask,
                                          const LVKW_WindowAttributes *attributes) {
  lvkw_check_wnd_update(window_handle, field_mask, attributes);
  return lvkw_wnd_update_WL(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  lvkw_check_wnd_requestFocus(window_handle);
  return lvkw_wnd_requestFocus_WL(window_handle);
}
#endif