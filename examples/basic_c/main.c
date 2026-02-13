#include <lvkw/lvkw.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "vulkan_engine.h"

typedef struct AppState {
  LVKW_Context* ctx;
  VulkanEngine engine;
  bool engine_initialized;
  bool keep_going;
  bool fullscreen;
  bool cursor_locked;
  LVKW_Window* window;
  uint32_t extension_count;
  const char** extensions;
} AppState;

void on_event(LVKW_EventType type, LVKW_Window* window, const LVKW_Event* event, void* userdata) {
  AppState* state = (AppState*)userdata;

  switch (type) {
    case LVKW_EVENT_TYPE_WINDOW_READY:
      vulkan_engine_init(&state->engine, state->ctx, state->window, state->extension_count, state->extensions);
      state->engine_initialized = true;
      break;
    case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
      state->keep_going = false;
      break;
    case LVKW_EVENT_TYPE_WINDOW_RESIZED:
      if (state->engine_initialized) {
        vulkan_engine_on_resized(&state->engine, (uint32_t)event->resized.geometry.pixelSize.x,
                                 (uint32_t)event->resized.geometry.pixelSize.y);
      }
      break;
    case LVKW_EVENT_TYPE_KEY:
      if (event->key.key == LVKW_KEY_ESCAPE && event->key.state == LVKW_BUTTON_STATE_PRESSED) {
        state->keep_going = false;
      }
      if (event->key.key == LVKW_KEY_F && event->key.state == LVKW_BUTTON_STATE_PRESSED) {
        state->fullscreen = !state->fullscreen;
        if (lvkw_wnd_setFullscreen(state->window, state->fullscreen) != LVKW_SUCCESS) {
          fprintf(stderr, "Failed to toggle fullscreen\n");
        }
      }
      if (event->key.key == LVKW_KEY_L && event->key.state == LVKW_BUTTON_STATE_PRESSED) {
        state->cursor_locked = !state->cursor_locked;
        LVKW_CursorMode mode = state->cursor_locked ? LVKW_CURSOR_LOCKED : LVKW_CURSOR_NORMAL;
        if (lvkw_wnd_setCursorMode(state->window, mode) != LVKW_SUCCESS) {
          fprintf(stderr, "Failed to toggle cursor lock\n");
        }
      }
      break;
    case LVKW_EVENT_TYPE_MOUSE_MOTION:
      printf("Mouse Motion: pos=%.2f,%.2f delta=%.2f,%.2f\n", event->mouse_motion.position.x,
             event->mouse_motion.position.y, event->mouse_motion.delta.x, event->mouse_motion.delta.y);
      break;
    default:
      break;
  }
}

void on_lvkw_diagnostic(const LVKW_DiagnosticInfo* info, void* userdata) {
  fprintf(stderr, "LVKW Diagnostic: %s (Code: %d)\n", info->message, info->diagnostic);
}

int main() {
  LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  ctx_info.attributes.diagnostic_cb = on_lvkw_diagnostic;

  LVKW_Context* ctx = NULL;

  if (lvkw_createContext(&ctx_info, &ctx) != LVKW_SUCCESS) {
    fprintf(stderr, "Failed to create LVKW context\n");
    return EXIT_FAILURE;
  }

  LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  window_info.attributes.title = "LVKW C Example";
  window_info.attributes.logicalSize = (LVKW_LogicalVec){1280, 720};
  window_info.content_type = LVKW_CONTENT_TYPE_GAME;

  LVKW_Window* window = NULL;
  if (lvkw_ctx_createWindow(ctx, &window_info, &window) != LVKW_SUCCESS) {
    fprintf(stderr, "Failed to create LVKW window\n");
    lvkw_ctx_destroy(ctx);
    return EXIT_FAILURE;
  }

  uint32_t monitor_count = 0;
  lvkw_ctx_getMonitors(ctx, NULL, &monitor_count);
  printf("Monitors detected: %u\n", monitor_count);
  if (monitor_count > 0) {
    LVKW_Monitor** monitors = malloc(sizeof(LVKW_Monitor*) * monitor_count);
    lvkw_ctx_getMonitors(ctx, monitors, &monitor_count);
    for (uint32_t i = 0; i < monitor_count; ++i) {
      printf("  Monitor: %s - %.2fx%.2fmm, Current Mode: %dx%d@%uHz\n",
             monitors[i]->name, monitors[i]->physical_size.x, monitors[i]->physical_size.y,
             monitors[i]->current_mode.size.x, monitors[i]->current_mode.size.y,
             (unsigned int)(monitors[i]->current_mode.refresh_rate_mhz / 1000));

      uint32_t mode_count = 0;
      lvkw_ctx_getMonitorModes(ctx, monitors[i], NULL, &mode_count);
      printf("    Available Modes: %u\n", mode_count);
      if (mode_count > 0) {
        LVKW_VideoMode* modes = malloc(sizeof(LVKW_VideoMode) * mode_count);
        lvkw_ctx_getMonitorModes(ctx, monitors[i], modes, &mode_count);
        for (uint32_t j = 0; j < mode_count; ++j) {
          printf("      Mode: %dx%d@%uHz\n", modes[j].size.x, modes[j].size.y,
                 (unsigned int)(modes[j].refresh_rate_mhz / 1000));
        }
        free(modes);
      }
    }
    free(monitors);
  }

  uint32_t extension_count = 0;
  const char* const* extensions;
  lvkw_ctx_getVkExtensions(ctx, &extension_count, &extensions);

  AppState state = {0};
  state.ctx = ctx;
  state.keep_going = true;
  state.window = window;
  state.extension_count = extension_count;
  state.extensions = (const char**)extensions;

  while (state.keep_going) {
    if (lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, on_event, &state) != LVKW_SUCCESS) {
      fprintf(stderr, "Poll events failed\n");
      break;
    }
    if (state.engine_initialized) {
      vulkan_engine_draw_frame(&state.engine);
    }
  }

  if (state.engine_initialized) {
    vulkan_engine_cleanup(&state.engine);
  }

  lvkw_wnd_destroy(window);
  lvkw_ctx_destroy(ctx);

  return EXIT_SUCCESS;
}