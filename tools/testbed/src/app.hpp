#pragma once

#include "imgui.h"
#include "lvkw/lvkw.hpp"
#include "feature_module.hpp"
#include "modules/event_module.hpp"
#include "modules/metrics_module.hpp"
#include "modules/controller_module.hpp"
#include "modules/monitor_module.hpp"
#include "modules/cursor_module.hpp"
#include "modules/window_module.hpp"
#include "modules/input_module.hpp"
#include "modules/event_log_module.hpp"
#include "modules/clipboard_module.hpp"
#include "modules/dnd_module.hpp"
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

enum class AppStatus {
  KEEP_GOING,
  EXIT,
  RECREATE_CONTEXT
};

class App {
public:
  App(lvkw::Window &window, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, uint32_t queue_family, VkQueue queue, VkDescriptorPool descriptor_pool);
  AppStatus update(lvkw::Context &ctx, lvkw::Window &window, ImGuiIO &io);
  void renderUi(lvkw::Context &ctx, lvkw::Window &window, const ImGuiIO &io);

  ImVec4 getClearColor() const { return clear_color_; }
  const ContextRecreateInfo& getRecreateInfo() const { return recreate_info_; }

private:
  ImVec4 clear_color_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  
  std::unique_ptr<EventModule> event_module_;
  std::unique_ptr<MetricsModule> metrics_module_;
  std::unique_ptr<ControllerModule> controller_module_;
  std::unique_ptr<MonitorModule> monitor_module_;
  std::unique_ptr<CursorModule> cursor_module_;
  std::unique_ptr<WindowModule> window_module_;
  std::unique_ptr<InputModule> input_module_;
  std::unique_ptr<EventLogModule> event_log_module_;
  std::unique_ptr<ClipboardModule> clipboard_module_;
  std::unique_ptr<DndModule> dnd_module_;

  ContextRecreateInfo recreate_info_;

  LVKW_CursorMode current_cursor_mode_ = LVKW_CURSOR_NORMAL;
  void updateCursor(lvkw::Context &ctx, lvkw::Window &window, ImGuiIO &io);
};
