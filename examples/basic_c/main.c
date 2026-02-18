// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdio.h>
#include <stdbool.h>

#include <lvkw/lvkw.h>
#include <vulkan_renderer.h>

typedef struct AppState {
  VulkanRenderer renderer;
  bool renderer_initialized;
  bool keep_going;
} AppState;

void on_event(LVKW_EventType type, LVKW_Window* window, const LVKW_Event* event, void* userdata) {
  AppState* state = (AppState*)userdata;

  switch (type) {
    case LVKW_EVENT_TYPE_WINDOW_READY: {
      VkSurfaceKHR surface;
      lvkw_display_createVkSurface(window, state->renderer.instance, &surface);

      LVKW_WindowGeometry geometry;
      lvkw_display_getWindowGeometry(window, &geometry);

      vulkan_renderer_setup_surface(&state->renderer, surface, (uint32_t)geometry.pixel_size.x,
                                    (uint32_t)geometry.pixel_size.y);
      state->renderer_initialized = true;
      break;
    }

    case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
      state->keep_going = false;
      break;

    case LVKW_EVENT_TYPE_WINDOW_RESIZED:
      if (state->renderer_initialized) {
        vulkan_renderer_on_resized(&state->renderer, 
                                 (uint32_t)event->resized.geometry.pixel_size.x,
                                 (uint32_t)event->resized.geometry.pixel_size.y);
      }
      break;

    default:
      break;
  }
}

int main() {
  AppState state = {
    .keep_going = true,
    .renderer_initialized = false
  };

  LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  ctx_info.attributes.event_callback = on_event;
  ctx_info.attributes.event_userdata = &state;
  
  LVKW_Context* ctx = NULL;
  if (lvkw_context_create(&ctx_info, &ctx) != LVKW_SUCCESS) return 1;

  LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  window_info.attributes.title = "LVKW Basic Example (C)";
  window_info.attributes.logical_size = (LVKW_LogicalVec){1280, 720};

  LVKW_Window* window = NULL;
  if (lvkw_display_createWindow(ctx, &window_info, &window) != LVKW_SUCCESS) return 1;

  uint32_t extension_count = 0;
  const char* const* extensions = NULL;
  lvkw_display_listVkExtensions(ctx, &extension_count, &extensions);

  vulkan_renderer_init(&state.renderer, extension_count, (const char**)extensions);

  while (state.keep_going) {
    lvkw_events_pump(ctx, 0);

    if (state.renderer_initialized) {
      vulkan_renderer_draw_frame(&state.renderer);
    }
  }

  if (state.renderer_initialized) {
    vulkan_renderer_cleanup(&state.renderer);
  }

  lvkw_display_destroyWindow(window);
  lvkw_context_destroy(ctx);

  return 0;
}
