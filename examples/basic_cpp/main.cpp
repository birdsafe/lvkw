// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <iostream>
#include <vector>

#include "lvkw/cpp/cxx20.hpp"
#include <vulkan_renderer.h>

struct App {
  VulkanRenderer renderer;
  bool renderer_initialized = false;
  bool keep_going = true;
  
  // Use a pointer or delayed initialization for the window
  std::unique_ptr<lvkw::Window> window;

  void on_ready(lvkw::WindowReadyEvent) {
    VkSurfaceKHR surface = window->createVkSurface(renderer.instance);
    auto geometry = window->getGeometry();
    vulkan_renderer_setup_surface(&renderer, surface,
                                  static_cast<uint32_t>(geometry.pixel_size.x),
                                  static_cast<uint32_t>(geometry.pixel_size.y));
    renderer_initialized = true;
  }

  void on_close(lvkw::WindowCloseEvent) { 
    keep_going = false; 
  }

  void on_resize(lvkw::WindowResizedEvent evt) {
    if (renderer_initialized) {
      vulkan_renderer_on_resized(&renderer, 
                                static_cast<uint32_t>(evt->geometry.pixel_size.x),
                                static_cast<uint32_t>(evt->geometry.pixel_size.y));
    }
  }
};

int main() {
  App app;

  auto dispatcher = lvkw::makeDispatcher(
    [&](lvkw::WindowReadyEvent e) { app.on_ready(e); },
    [&](lvkw::WindowCloseEvent e) { app.on_close(e); },
    [&](lvkw::WindowResizedEvent e) { app.on_resize(e); }
  );

  // 1. Initialize the LVKW Context.
  LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  ctx_info.attributes.event_callback = decltype(dispatcher)::callback;
  ctx_info.attributes.event_userdata = &dispatcher;
  
  ctx_info.attributes.diagnostic_cb = [](const LVKW_DiagnosticInfo *info, void *) {
    std::cout << "LVKW [" << (int)info->diagnostic << "]: " << info->message << std::endl;
  };
  
  lvkw::Context ctx(ctx_info);

  // 2. Create a Window.
  LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  window_info.attributes.title = "LVKW Basic Example (C++20)";
  window_info.attributes.logical_size = {1280, 720};
  
  app.window = std::make_unique<lvkw::Window>(ctx.createWindow(window_info));

  // 3. Get Vulkan extensions required by LVKW
  auto extensions_vec = ctx.getVkExtensions();

  vulkan_renderer_init(&app.renderer, static_cast<uint32_t>(extensions_vec.size()), extensions_vec.data());
  
  // 4. Main Loop
  while (app.keep_going) {
    ctx.pumpEvents(0);

    if (app.renderer_initialized) {
      vulkan_renderer_draw_frame(&app.renderer);
    }
  }

  // 5. Cleanup
  if (app.renderer_initialized) {
    vulkan_renderer_cleanup(&app.renderer);
  }

  return 0;
}
