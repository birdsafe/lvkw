// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <iostream>
#include <vector>
#include <optional>

#include "lvkw/lvkw.hpp"
#include "vulkan_engine.hpp"

struct AppState {
  VulkanEngine engine;
  bool keep_going = true;
  bool fullscreen = false;
  bool cursor_locked = false;
};

int main() {
  try {
    // 1. Initialize the LVKW Context. 
    // The context manages the connection to the display server (Wayland/X11/Win32).
    LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
    
    // We register a diagnostic callback to receive warnings or errors from the library.
    ctx_info.attributes.diagnostic_cb = [](const LVKW_DiagnosticInfo *info, void *) {
      std::cerr << "LVKW [" << (int)info->diagnostic << "]: " << info->message << std::endl;
    };
    
    lvkw::Context ctx(ctx_info);

    std::cout << "LVKW Context initialized. Detected " << ctx.getMonitors().size() << " monitors." << std::endl;

    // 2. Create a Window.
    LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
    window_info.attributes.title = "LVKW Basic Example (C++)";
    window_info.attributes.logicalSize = {1280, 720};
    window_info.app_id = "org.lvkw.example_cpp";
    window_info.content_type = LVKW_CONTENT_TYPE_GAME;

    lvkw::Window window = ctx.createWindow(window_info);

    // 3. Setup Vulkan integration.
    // LVKW provides the required instance extensions and handles surface creation.
    auto extensions = ctx.getVkExtensions();
    
    AppState state;
    bool engine_initialized = false;
    std::optional<lvkw::Controller> active_controller;

    std::cout << "Controls:\n"
              << "  [ESC] Close\n"
              << "  [F]   Toggle Fullscreen\n"
              << "  [L]   Toggle Cursor Lock\n" << std::endl;

    // 4. Main Loop
    while (state.keep_going) {
      // Poll and process events. 
      // The C++ API uses lambdas to mask which events you are interested in.
      ctx.pollEvents(
          [&](lvkw::ControllerConnectionEvent evt) {
            if (evt->connected) {
              std::cout << "Controller connected: " << evt->id << std::endl;
              active_controller.emplace(ctx.createController(evt->id));
            } else {
              std::cout << "Controller disconnected." << std::endl;
              active_controller.reset();
            }
          },
          [&](lvkw::WindowReadyEvent) {
            // WindowReadyEvent is the signal that OS resources are allocated 
            // and it's now safe to create a Vulkan surface.
            state.engine.init(ctx, window, extensions);
            engine_initialized = true;
          },
          [&](lvkw::WindowCloseEvent) { 
            state.keep_going = false; 
          },
          [&](lvkw::WindowResizedEvent evt) {
            if (engine_initialized) {
              state.engine.onResized(static_cast<uint32_t>(evt->geometry.pixelSize.x),
                                     static_cast<uint32_t>(evt->geometry.pixelSize.y));
            }
          },
          [&](lvkw::KeyboardEvent evt) {
            if (evt->state != LVKW_BUTTON_STATE_PRESSED) return;

            switch (evt->key) {
              case LVKW_KEY_ESCAPE: state.keep_going = false; break;
              case LVKW_KEY_F:
                state.fullscreen = !state.fullscreen;
                window.setFullscreen(state.fullscreen);
                break;
              case LVKW_KEY_L:
                state.cursor_locked = !state.cursor_locked;
                window.setCursorMode(state.cursor_locked ? LVKW_CURSOR_LOCKED : LVKW_CURSOR_NORMAL);
                break;
              default: break;
            }
          }
      );

      if (engine_initialized) {
        state.engine.drawFrame();
      }
    }

    // 5. Cleanup
    if (engine_initialized) {
      state.engine.cleanup();
    }

  } catch (const lvkw::ContextLostException &e) {
    std::cerr << "CRITICAL: Connection to the display server was lost!" << std::endl;
    return EXIT_FAILURE;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
