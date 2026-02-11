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
    LVKW_ContextCreateInfo ctx_info = {};
    ctx_info.attributes.diagnosis_cb = [](const LVKW_DiagnosisInfo *info, void *) {
      std::cerr << "Diagnosis: " << info->message << " (Code: " << (int)info->diagnosis << ")" << std::endl;
    };
    ctx_info.backend = LVKW_BACKEND_AUTO;
    ctx_info.attributes.idle_timeout_ms = LVKW_IDLE_NEVER;
    ctx_info.attributes.inhibit_idle = false;
    lvkw::Context ctx(ctx_info);

    LVKW_WindowCreateInfo window_info = {
        .attributes =
            {
                .title = "LVKW Example",
                .logicalSize = {800, 600},
            },
        .app_id = "org.lvkw.example",
        .content_type = LVKW_CONTENT_TYPE_GAME,
        .transparent = false,
        .userdata = nullptr,
    };
    lvkw::Window window = ctx.createWindow(window_info);

    auto extensions = ctx.getVkExtensions();

    AppState state;
    bool engine_initialized = false;

    while (state.keep_going) {
      ctx.pollEvents(
          [&](lvkw::WindowReadyEvent) {
            state.engine.init(ctx, window, extensions);
            engine_initialized = true;
          },
          [&](lvkw::WindowCloseEvent) { state.keep_going = false; },
          [&](lvkw::WindowResizedEvent evt) {
            if (engine_initialized) {
              state.engine.onResized(evt->geometry.pixelSize.width, evt->geometry.pixelSize.height);
            }
          },
          [&](lvkw::KeyboardEvent evt) {
            if (evt->key == LVKW_KEY_ESCAPE && evt->state == LVKW_BUTTON_STATE_PRESSED) state.keep_going = false;
            if (evt->key == LVKW_KEY_F && evt->state == LVKW_BUTTON_STATE_PRESSED) {
              state.fullscreen = !state.fullscreen;
              window.setFullscreen(state.fullscreen);
            }
            if (evt->key == LVKW_KEY_L && evt->state == LVKW_BUTTON_STATE_PRESSED) {
              state.cursor_locked = !state.cursor_locked;
              window.setCursorMode(state.cursor_locked ? LVKW_CURSOR_LOCKED : LVKW_CURSOR_NORMAL);
            }
            if (evt->key == LVKW_KEY_S && evt->state == LVKW_BUTTON_STATE_PRESSED) {
              state.cursor_shape_index = (state.cursor_shape_index + 1) % num_shapes;
              window.setCursorShape(test_shapes[state.cursor_shape_index]);
              std::cout << "Cursor Shape: " << (int)test_shapes[state.cursor_shape_index] << std::endl;
            }
          },
          [&](lvkw::MouseMotionEvent evt) {
            std::cout << "Mouse Motion: pos=" << evt->x << "," << evt->y << " delta=" << evt->dx << "," << evt->dy
                      << std::endl;
          });

      if (engine_initialized) {
        state.engine.drawFrame();
      }
    }

    if (engine_initialized) {
      state.engine.cleanup();
    }
  } catch (const lvkw::ContextLostException &e) {
    std::cerr << "CRITICAL: Connection to the display server was lost! (" << e.what() << ")" << std::endl;
    return EXIT_FAILURE;
  } catch (const lvkw::Exception &e) {
    std::cerr << "LVKW Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const std::exception &e) {
    std::cerr << "Unhandled exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}