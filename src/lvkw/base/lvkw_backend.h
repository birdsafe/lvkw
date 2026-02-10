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
    typeof(lvkw_ctx_update) *update;
  } context;

  struct {
    typeof(lvkw_ctx_createWindow) *create;
    typeof(lvkw_wnd_update) *update;
    typeof(lvkw_wnd_destroy) *destroy;
    typeof(lvkw_wnd_createVkSurface) *create_vk_surface;
    typeof(lvkw_wnd_getFramebufferSize) *get_framebuffer_size;
    typeof(lvkw_wnd_requestFocus) *request_focus;
  } window;
} LVKW_Backend;
#endif

#endif // LVKW_BACKEND_H_INCLUDED
