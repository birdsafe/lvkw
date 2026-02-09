#include <iostream>

#include "lvkw/lvkw.h"
#include "lvkw/lvkw.hpp"
#include "vulkan_engine.hpp"

struct AppState {
  VulkanEngine engine;
  bool keep_going = true;
  bool fullscreen = false;
  bool cursor_locked = false;
  int cursor_shape_index = 0;
};

int main() {
  const LVKW_CursorShape test_shapes[] = {
      LVKW_CURSOR_SHAPE_DEFAULT, LVKW_CURSOR_SHAPE_POINTER, LVKW_CURSOR_SHAPE_CROSSHAIR, LVKW_CURSOR_SHAPE_TEXT,
      LVKW_CURSOR_SHAPE_WAIT,    LVKW_CURSOR_SHAPE_HELP,    LVKW_CURSOR_SHAPE_MOVE,      LVKW_CURSOR_SHAPE_PROGRESS};
  const int num_shapes = sizeof(test_shapes) / sizeof(test_shapes[0]);

  try {
    lvkw::Context ctx([](const LVKW_DiagnosisInfo *info, void *) {
      std::cerr << "Diagnosis: " << info->message << " (Code: " << (int)info->diagnosis << ")" << std::endl;
    });

    LVKW_WindowCreateInfo window_info = {
        .title = "LVKW Example",
        .app_id = "org.lvkw.example",
        .size = {800, 600},
        .content_type = LVKW_CONTENT_TYPE_GAME,
        .user_data = nullptr,
    };
    lvkw::Window window(ctx, window_info);

    auto extensions = ctx.getVulkanInstanceExtensions();

    AppState state;
    bool engine_initialized = false;

    while (state.keep_going) {
      ctx.pollEvents(
          [&](const LVKW_WindowReadyEvent &) {
            state.engine.init(ctx, window, extensions);
            engine_initialized = true;
          },
          [&](const LVKW_WindowCloseEvent &) { state.keep_going = false; },
          [&](const LVKW_WindowResizedEvent &evt) {
            if (engine_initialized) {
              state.engine.onResized(evt.framebufferSize.width, evt.framebufferSize.height);
            }
          },
          [&](const LVKW_KeyboardEvent &evt) {
            if (evt.key == LVKW_KEY_ESCAPE && evt.state == LVKW_KEY_STATE_PRESSED) state.keep_going = false;
            if (evt.key == LVKW_KEY_F && evt.state == LVKW_KEY_STATE_PRESSED) {
              state.fullscreen = !state.fullscreen;
              window.setFullscreen(state.fullscreen);
            }
            if (evt.key == LVKW_KEY_L && evt.state == LVKW_KEY_STATE_PRESSED) {
              state.cursor_locked = !state.cursor_locked;
              window.setCursorMode(state.cursor_locked ? LVKW_CURSOR_LOCKED : LVKW_CURSOR_NORMAL);
            }
            if (evt.key == LVKW_KEY_S && evt.state == LVKW_KEY_STATE_PRESSED) {
              state.cursor_shape_index = (state.cursor_shape_index + 1) % num_shapes;
              window.setCursorShape(test_shapes[state.cursor_shape_index]);
              std::cout << "Cursor Shape: " << (int)test_shapes[state.cursor_shape_index] << std::endl;
            }
          },
          [&](const LVKW_MouseMotionEvent &evt) {
            std::cout << "Mouse Motion: pos=" << evt.x << "," << evt.y << " delta=" << evt.dx << "," << evt.dy
                      << std::endl;
          });

      if (engine_initialized) {
        state.engine.drawFrame();
      }
    }

    if (engine_initialized) {
      state.engine.cleanup();
    }
  } catch (const std::exception &e) {
    std::cerr << "Unhandled exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}