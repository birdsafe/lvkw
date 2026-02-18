#pragma once

#include "../feature_module.hpp"
#include "imgui_impl_vulkan.h"
#include <memory>
#include <string>
#include <vector>

class WindowModule : public FeatureModule {
public:
  WindowModule(lvkw::Window &primary_window, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, uint32_t queue_family, VkQueue queue, VkDescriptorPool descriptor_pool);
  virtual ~WindowModule();

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;
  void onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) override;

  void onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& event);

  const char *getName() const override { return "Windowing"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

  lvkw::Window &primary_window_;
  VkInstance instance_;
  VkPhysicalDevice physical_device_;
  VkDevice device_;
  uint32_t queue_family_;
  VkQueue queue_;
  VkDescriptorPool descriptor_pool_;

  struct SecondaryWindow {
    std::unique_ptr<lvkw::Window> window;
    ImGui_ImplVulkanH_Window wd;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    bool should_close = false;
    bool swapchain_rebuild = false;
    std::string title;
    bool is_decorated = true;
    bool is_resizable = true;
    bool mouse_passthrough = false;
    bool primary_selection = false;
    LVKW_LogicalVec min_size = {0, 0};
    LVKW_LogicalVec max_size = {0, 0};
    LVKW_Fraction aspect_ratio = {0, 0};
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
  };

  std::vector<std::unique_ptr<SecondaryWindow>> secondary_windows_;
  
  bool primary_is_decorated_ = true;
  bool primary_is_primary_selection_ = false;

  void createSecondaryWindow(lvkw::Context &ctx);
  void renderSecondaryWindow(SecondaryWindow &sw);
  void cleanupSecondaryWindow(SecondaryWindow &sw);
};
