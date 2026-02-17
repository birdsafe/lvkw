// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <lvkw/lvkw.h>
#include <vulkan_renderer.h>

typedef struct AppState {
  LVKW_Context* ctx;
  LVKW_Window* window;
  VulkanRenderer renderer;
  bool renderer_initialized;
  bool keep_going;
  bool fullscreen;
  bool cursor_locked;

  uint32_t vk_extension_count;
  const char** vk_extensions;
} AppState;

// Event callback function
void on_event(LVKW_EventType type, LVKW_Window* window, const LVKW_Event* event, void* userdata) {
  AppState* state = (AppState*)userdata;

  switch (type) {
    case LVKW_EVENT_TYPE_WINDOW_READY: {
      // OS resources are ready, we can initialize our renderer.
      VkSurfaceKHR surface;
      if (lvkw_display_createVkSurface(window, state->renderer.instance, &surface) != LVKW_SUCCESS) {
        fprintf(stderr, "failed to create window surface!\n");
        exit(EXIT_FAILURE);
      }

      LVKW_WindowGeometry geometry;
      if (lvkw_display_getWindowGeometry(window, &geometry) != LVKW_SUCCESS) {
        fprintf(stderr, "failed to get window geometry!\n");
        exit(EXIT_FAILURE);
      }

      vulkan_renderer_setup_surface(&state->renderer, surface, (uint32_t)geometry.pixelSize.x,
                                    (uint32_t)geometry.pixelSize.y);
      state->renderer_initialized = true;
      break;
    }

    case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
      state->keep_going = false;
      break;

    case LVKW_EVENT_TYPE_WINDOW_RESIZED:
      if (state->renderer_initialized) {
        vulkan_renderer_on_resized(&state->renderer, 
                                 (uint32_t)event->resized.geometry.pixelSize.x,
                                 (uint32_t)event->resized.geometry.pixelSize.y);
      }
      break;

    case LVKW_EVENT_TYPE_KEY:
      if (event->key.state != LVKW_BUTTON_STATE_PRESSED) break;

      if (event->key.key == LVKW_KEY_ESCAPE) {
        state->keep_going = false;
      } else if (event->key.key == LVKW_KEY_F) {
        state->fullscreen = !state->fullscreen;
        lvkw_display_setWindowFullscreen(state->window, state->fullscreen);
      } else if (event->key.key == LVKW_KEY_L) {
        state->cursor_locked = !state->cursor_locked;
        lvkw_display_setWindowCursorMode(state->window, state->cursor_locked ? LVKW_CURSOR_LOCKED : LVKW_CURSOR_NORMAL);
      }
      break;

    default:
      break;
  }
}

void on_lvkw_diagnostic(const LVKW_DiagnosticInfo* info, void* userdata) {
  (void)userdata;
  fprintf(stderr, "LVKW Diagnostic: %s (Code: %d)\n", info->message, info->diagnostic);
}

int main() {
  // 1. Initialize Context
  LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  ctx_info.attributes.diagnostic_cb = on_lvkw_diagnostic;

  LVKW_Context* ctx = NULL;
  if (lvkw_context_create(&ctx_info, &ctx) != LVKW_SUCCESS) {
    fprintf(stderr, "Failed to create LVKW context\n");
    return EXIT_FAILURE;
  }

  // 2. Create Window
  LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  window_info.attributes.title = "LVKW Basic Example (C)";
  window_info.attributes.logicalSize = (LVKW_LogicalVec){1280, 720};
  window_info.app_id = "org.lvkw.example_c";
  window_info.content_type = LVKW_CONTENT_TYPE_GAME;

  LVKW_Window* window = NULL;
  if (lvkw_display_createWindow(ctx, &window_info, &window) != LVKW_SUCCESS) {
    fprintf(stderr, "Failed to create LVKW window\n");
    lvkw_context_destroy(ctx);
    return EXIT_FAILURE;
  }

  // 3. Get Vulkan extensions required by LVKW
  uint32_t extension_count = 0;
  const char* const* extensions = NULL;
  lvkw_display_listVkExtensions(ctx, &extension_count, &extensions);

  AppState state = {
    .ctx = ctx,
    .window = window,
    .keep_going = true,
    .renderer_initialized = false,
    .vk_extension_count = extension_count,
    .vk_extensions = (const char**)extensions
  };

  vulkan_renderer_init(&state.renderer, state.vk_extension_count, state.vk_extensions);

  printf("LVKW Basic Example started.\n");
  printf("Controls: [ESC] Quit, [F] Fullscreen, [L] Cursor Lock\n");

  // 3. Main Loop
  while (state.keep_going) {
    // Process all pending events
    lvkw_events_pump(ctx, 0);
    lvkw_events_commit(ctx);
    lvkw_events_scan(ctx, LVKW_EVENT_TYPE_ALL, on_event, &state);

    if (state.renderer_initialized) {
      vulkan_renderer_draw_frame(&state.renderer);
    }
  }

  // 4. Cleanup
  if (state.renderer_initialized) {
    vulkan_renderer_cleanup(&state.renderer);
  }

  lvkw_display_destroyWindow(window);
  lvkw_context_destroy(ctx);

  return EXIT_SUCCESS;
}
