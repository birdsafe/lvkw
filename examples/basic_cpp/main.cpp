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
};

int main() {
  // 1. Initialize the LVKW Context.
  LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  
  ctx_info.attributes.diagnostic_cb = [](const LVKW_DiagnosticInfo *info, void *) {
    std::cerr << "LVKW [" << (int)info->diagnostic << "]: " << info->message << std::endl;
  };
  
  lvkw::Context ctx(ctx_info);

  // 2. Create a Window.
  LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  window_info.attributes.title = "LVKW Basic Example (C++)";
  window_info.attributes.logicalSize = {1280, 720};
  
  lvkw::Window window = ctx.createWindow(window_info);

  // 3. Get Vulkan extensions required by LVKW
  auto extensions_vec = ctx.getVkExtensions();

  AppState state;
  vulkan_renderer_init(&state.renderer, static_cast<uint32_t>(extensions_vec.size()), extensions_vec.data());
  
  // 4. Main Loop
  while (state.keep_going) {
    // Process all pending events using the C++20 shorthand
    lvkw::pollEvents(ctx,
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
        }
    );

    if (state.renderer_initialized) {
      vulkan_renderer_draw_frame(&state.renderer);
    }
  }

  // 5. Cleanup
  if (state.renderer_initialized) {
    vulkan_renderer_cleanup(&state.renderer);
  }

  return 0;
}
