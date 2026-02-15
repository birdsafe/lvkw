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
              << "  [L]   Toggle Cursor Lock\n"
              << "  [Ctrl/Cmd+V] Print pasted text to stderr\n" << std::endl;

    // 4. Main Loop
    while (state.keep_going) {
      // Synchronize with OS and scan events. 
      // The C++ API uses lambdas to mask which events you are interested in.
      lvkw::syncEvents(ctx);
      lvkw::scanEvents(
          ctx,
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
          [&](lvkw::DndHoverEvent evt) {
            if (evt->entered) {
              std::cout << "[DND] Enter at (" << evt->position.x << ", " << evt->position.y
                        << "), paths=" << evt->path_count << std::endl;
              for (uint16_t i = 0; i < evt->path_count; i++) {
                std::cout << "  - " << evt->paths[i] << std::endl;
              }
            }
          },
          [&](lvkw::DndLeaveEvent) {
            std::cout << "[DND] Leave" << std::endl;
          },
          [&](lvkw::DndDropEvent evt) {
            std::cout << "[DND] Drop at (" << evt->position.x << ", " << evt->position.y
                      << "), paths=" << evt->path_count << std::endl;
            for (uint16_t i = 0; i < evt->path_count; i++) {
              std::cout << "  - " << evt->paths[i] << std::endl;
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
                    (evt->modifiers & LVKW_MODIFIER_CONTROL) || (evt->modifiers & LVKW_MODIFIER_SUPER);
                if (paste_shortcut) {
                  try {
                    const char *text = window.getClipboardText();
                    std::cerr << "[PASTE] " << (text ? text : "") << std::endl;
                  } catch (const std::exception &primary_error) {
                    try {
                      try {
                        const auto mime_types = window.getClipboardMimeTypes();
                        std::cerr << "[PASTE] Available MIME types (" << mime_types.size() << "):";
                        for (const char *mime : mime_types) {
                          std::cerr << " " << (mime ? mime : "<null>");
                        }
                        std::cerr << std::endl;
                      } catch (const std::exception &e) {
                        std::cerr << "[PASTE] Failed to enumerate MIME types: " << e.what() << std::endl;
                      }

                      const char *mime_candidates[] = {
                          "text/plain;charset=utf-8",
                          "text/plain;charset=UTF-8",
                          "text/plain;charset=utf8",
                          "UTF8_STRING",
                          "text/plain",
                          "STRING",
                          "TEXT",
                      };

                      bool printed = false;
                      for (const char *mime : mime_candidates) {
                        try {
                          const void *data = nullptr;
                          size_t size = 0;
                          window.getClipboardData(mime, &data, &size);
                          if (!data || size == 0) continue;

                          std::cerr << "[PASTE] ";
                          std::cerr.write(static_cast<const char *>(data), static_cast<std::streamsize>(size));
                          std::cerr << std::endl;
                          printed = true;
                          break;
                        } catch (const std::exception &) {
                        }
                      }

                      if (!printed) {
                        std::cerr << "[PASTE] Failed to read clipboard text: " << primary_error.what()
                                  << std::endl;
                      }
                    } catch (const std::exception &) {
                      std::cerr << "[PASTE] Failed to read clipboard text: " << primary_error.what()
                                << std::endl;
                    }
                  }
                }
              } break;
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
