#ifndef LVKW_BACKEND_H_INCLUDED
#define LVKW_BACKEND_H_INCLUDED

#include "lvkw_types_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
typedef struct LVKW_Backend {
  struct {
    typeof(lvkw_ctx_destroy) *destroy;
    typeof(lvkw_ctx_getVkExtensions) *get_vulkan_instance_extensions;
    typeof(lvkw_ctx_pollEvents) *poll_events;
    typeof(lvkw_ctx_waitEvents) *wait_events;
    typeof(lvkw_ctx_updateAttributes) *update_attributes;
  } context;

  struct {
    typeof(lvkw_ctx_createWindow) *create;
    typeof(lvkw_wnd_updateAttributes) *update_attributes;
    typeof(lvkw_wnd_destroy) *destroy;
    typeof(lvkw_wnd_createVkSurface) *create_vk_surface;
    typeof(lvkw_wnd_getFramebufferSize) *get_framebuffer_size;
    typeof(lvkw_wnd_setFullscreen) *set_fullscreen;
    typeof(lvkw_wnd_setCursorMode) *set_cursor_mode;
    typeof(lvkw_wnd_setCursorShape) *set_cursor_shape;
    typeof(lvkw_wnd_requestFocus) *request_focus;
  } window;
} LVKW_Backend;
#endif

#endif // LVKW_BACKEND_H_INCLUDED
