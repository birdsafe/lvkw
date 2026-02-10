#ifndef LVKW_BACKEND_H_INCLUDED
#define LVKW_BACKEND_H_INCLUDED

#include "lvkw_types_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
typedef struct LVKW_Backend {
  struct {
    typeof(lvkw_destroyContext) *destroy;
    typeof(lvkw_context_getVulkanInstanceExtensions) *get_vulkan_instance_extensions;
    typeof(lvkw_context_pollEvents) *poll_events;
    typeof(lvkw_context_waitEvents) *wait_events;
    typeof(lvkw_context_setIdleTimeout) *set_idle_timeout;
  } context;

  struct {
    typeof(lvkw_context_createWindow) *create;
    typeof(lvkw_destroyWindow) *destroy;
    typeof(lvkw_window_createVkSurface) *create_vk_surface;
    typeof(lvkw_window_getFramebufferSize) *get_framebuffer_size;
    typeof(lvkw_window_setFullscreen) *set_fullscreen;
    typeof(lvkw_window_setCursorMode) *set_cursor_mode;
    typeof(lvkw_window_setCursorShape) *set_cursor_shape;
    typeof(lvkw_window_requestFocus) *request_focus;
  } window;
} LVKW_Backend;
#endif

#endif // LVKW_BACKEND_H_INCLUDED
