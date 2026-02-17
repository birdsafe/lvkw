// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <iostream>
#include <vector>

#include "lvkw/lvkw.hpp"
#include <vulkan_renderer.h>

struct AppState {
  VulkanRenderer renderer;
  bool renderer_initialized = false;
  bool keep_going = true;
  bool fullscreen = false;
  bool cursor_locked = false;

  uint32_t vk_extension_count = 0;
  const char** vk_extensions = nullptr;
};

int main() {
  try {
    // 1. Initialize the LVKW Context.
    LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
    
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

    // 3. Get Vulkan extensions required by LVKW
    auto extensions_vec = ctx.getVkExtensions();

    AppState state;
    state.vk_extension_count = static_cast<uint32_t>(extensions_vec.size());
    state.vk_extensions = extensions_vec.data();

    vulkan_renderer_init(&state.renderer, state.vk_extension_count, state.vk_extensions);

    LVKW_Controller *active_controller = nullptr;

    std::cout << "Controls:\n"
              << "  [ESC] Close\n"
              << "  [F]   Toggle Fullscreen\n"
              << "  [L]   Toggle Cursor Lock\n"
              << "  [Ctrl/Cmd+V] Print pasted text to stderr\n" << std::endl;

    // 3. Main Loop
    while (state.keep_going) {
      lvkw::pumpEvents(ctx);
      lvkw::commitEvents(ctx);
      lvkw::scanEvents(
          ctx,
          [&](lvkw::ControllerConnectionEvent evt) {
            if (evt->connected) {
              std::cout << "Controller connected." << std::endl;
              lvkw_input_createController(evt->controller_ref, &active_controller);
            } else {
              std::cout << "Controller disconnected." << std::endl;
              if (active_controller == (LVKW_Controller *)evt->controller_ref && active_controller) {
                lvkw_input_destroyController(active_controller);
                active_controller = nullptr;
              }
            }
          },
          [&](lvkw::WindowReadyEvent) {
            VkSurfaceKHR surface = window.createVkSurface(state.renderer.instance);
            auto geometry = window.getGeometry();
            vulkan_renderer_setup_surface(&state.renderer, surface,
                                          static_cast<uint32_t>(geometry.pixelSize.x),
                                          static_cast<uint32_t>(geometry.pixelSize.y));
            state.renderer_initialized = true;
          },
          [&](lvkw::WindowCloseEvent) { 
            state.keep_going = false; 
          },
          [&](lvkw::WindowResizedEvent evt) {
            if (state.renderer_initialized) {
              vulkan_renderer_on_resized(&state.renderer, 
                                        static_cast<uint32_t>(evt->geometry.pixelSize.x),
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
              case LVKW_KEY_V: {
                const bool paste_shortcut =
                    (evt->modifiers & LVKW_MODIFIER_CONTROL) || (evt->modifiers & LVKW_MODIFIER_META);
                if (paste_shortcut) {
                  try {
                    const char *text = window.getClipboardText();
                    std::cerr << "[PASTE] " << (text ? text : "") << std::endl;
                  } catch (const std::exception &) {
                    std::cerr << "[PASTE] Failed to read clipboard text." << std::endl;
                  }
                }
              } break;
              default: break;
            }
          }
      );

      if (state.renderer_initialized) {
        vulkan_renderer_draw_frame(&state.renderer);
      }
    }

    // 4. Cleanup
    if (state.renderer_initialized) {
      vulkan_renderer_cleanup(&state.renderer);
    }
    if (active_controller) {
      lvkw_input_destroyController(active_controller);
    }

  } catch (const lvkw::ContextLostException &) {
    std::cerr << "CRITICAL: Connection to the display server was lost!" << std::endl;
    return EXIT_FAILURE;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
