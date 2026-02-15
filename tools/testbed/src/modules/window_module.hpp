#pragma once

#include "../feature_module.hpp"
#include <vector>
#include <memory>
#include <string>
#include <vulkan/vulkan.h>
#include "imgui_impl_vulkan.h"

struct SecondaryWindow {
    std::unique_ptr<lvkw::Window> window;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    ImGui_ImplVulkanH_Window wd = {};
    ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
    bool should_close = false;
    bool swapchain_rebuild = false;
    bool is_decorated = true;
    bool is_resizable = true;
    bool mouse_passthrough = false;
    LVKW_LogicalVec min_size = {0, 0};
    LVKW_LogicalVec max_size = {0, 0};
    LVKW_Fraction aspect_ratio = {0, 0};
    std::string title;
};

class WindowModule : public FeatureModule {
public:
  WindowModule(lvkw::Window &primary_window, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, uint32_t queue_family, VkQueue queue, VkDescriptorPool descriptor_pool);
  virtual ~WindowModule();

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Windowing"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

  lvkw::Window &primary_window_;
  bool primary_is_decorated_ = true; // Tracked because lvkw doesn't expose it yet

  VkInstance instance_;
  VkPhysicalDevice physical_device_;
  VkDevice device_;
  uint32_t queue_family_;
  VkQueue queue_;
  VkDescriptorPool descriptor_pool_;

  std::vector<std::unique_ptr<SecondaryWindow>> secondary_windows_;

  void createSecondaryWindow(lvkw::Context &ctx);
  void renderSecondaryWindow(SecondaryWindow &sw);
  void cleanupSecondaryWindow(SecondaryWindow &sw);
};
