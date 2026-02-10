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

void on_event(const LVKW_Event* event, void* userdata) {
  AppState* state = (AppState*)userdata;

  switch (event->type) {
    case LVKW_EVENT_TYPE_WINDOW_READY:
      vulkan_engine_init(&state->engine, state->ctx, state->window, state->extension_count, state->extensions);
      state->engine_initialized = true;
      break;
    case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
      state->keep_going = false;
      break;
    case LVKW_EVENT_TYPE_WINDOW_RESIZED:
      if (state->engine_initialized) {
        vulkan_engine_on_resized(&state->engine, event->resized.framebufferSize.width,
                                 event->resized.framebufferSize.height);
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
        if (lvkw_wnd_setCursorMode(state->window, state->cursor_locked ? LVKW_CURSOR_LOCKED : LVKW_CURSOR_NORMAL) !=
            LVKW_SUCCESS) {
          fprintf(stderr, "Failed to toggle cursor lock\n");
        }
      }
      break;
    case LVKW_EVENT_TYPE_MOUSE_MOTION:
      printf("Mouse Motion: pos=%.2f,%.2f delta=%.2f,%.2f\n", event->mouse_motion.x, event->mouse_motion.y,
             event->mouse_motion.dx, event->mouse_motion.dy);
      break;
    default:
      break;
  }
}

void on_lvkw_diagnosis(const LVKW_DiagnosisInfo* info, void* userdata) {
  fprintf(stderr, "LVKW Diagnosis: %s (Code: %d)\n", info->message, info->diagnosis);
}

int main() {
  LVKW_ContextCreateInfo ctx_info = lvkw_ctx_defaultCreateInfo();
  ctx_info.diagnosis_cb = on_lvkw_diagnosis;

  LVKW_Context* ctx = NULL;

  if (lvkw_createContext(&ctx_info, &ctx) != LVKW_SUCCESS) {
    fprintf(stderr, "Failed to create LVKW context\n");
    return EXIT_FAILURE;
  }

  LVKW_WindowCreateInfo window_info = lvkw_wnd_defaultCreateInfo();
  window_info.title = "LVKW C Example";
  window_info.content_type = LVKW_CONTENT_TYPE_GAME;

  LVKW_Window* window = NULL;
  if (lvkw_ctx_createWindow(ctx, &window_info, &window) != LVKW_SUCCESS) {
    fprintf(stderr, "Failed to create LVKW window\n");
    lvkw_ctx_destroy(ctx);
    return EXIT_FAILURE;
  }

  uint32_t extension_count = 0;
  lvkw_ctx_getVkExtensions(ctx, &extension_count, NULL);
  const char** extensions = (const char**)malloc(sizeof(const char*) * extension_count);
  lvkw_ctx_getVkExtensions(ctx, &extension_count, extensions);

  AppState state = {0};
  state.ctx = ctx;
  state.keep_going = true;
  state.window = window;
  state.extension_count = extension_count;
  state.extensions = extensions;

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

  free(extensions);

  lvkw_wnd_destroy(window);
  lvkw_ctx_destroy(ctx);

  return EXIT_SUCCESS;
}