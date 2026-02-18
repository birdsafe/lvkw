#include "monitor_module.hpp"
#include "imgui.h"
#include "lvkw/lvkw.hpp"

MonitorModule::MonitorModule() {}

void MonitorModule::onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& evt) {
    if (!enabled_) return;

    if (type == LVKW_EVENT_TYPE_MONITOR_CONNECTION || type == LVKW_EVENT_TYPE_MONITOR_MODE) {
        needs_refresh_ = true;
    }
}

void MonitorModule::update(lvkw::Context &ctx, lvkw::Window &window) {
    if (!enabled_) return;

    if (needs_refresh_) {
        refreshMonitors(ctx);
        needs_refresh_ = false;
    }
}

void MonitorModule::refreshMonitors(lvkw::Context &ctx) {
    monitors_.clear();
    auto refs = ctx.getMonitors();
    
    for (auto* ref : refs) {
        MonitorInfo info;
        info.ref = ref;
        
        // Use a temporary handle to get properties
        auto* handle = ctx.createMonitor(ref);
        if (handle) {
            info.name = handle->name ? handle->name : "Unknown";
            info.position = handle->logical_position;
            info.size = handle->logical_size;
            info.physical_size = {(uint32_t)handle->physical_size.x, (uint32_t)handle->physical_size.y};
            info.scale = handle->scale;
            
            info.modes = ctx.getMonitorModes(handle);
            lvkw_display_destroyMonitor(handle);
        }
        
        monitors_.push_back(std::move(info));
    }
}

void MonitorModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Monitors", &enabled_)) {
    ImGui::End();
    return;
  }

  if (ImGui::Button("Refresh Now")) {
    needs_refresh_ = true;
  }

  ImGui::Separator();

  if (monitors_.empty()) {
    ImGui::Text("No monitors detected.");
  } else {
    for (size_t i = 0; i < monitors_.size(); ++i) {
      const auto& m = monitors_[i];
      char label[128];
      snprintf(label, sizeof(label), "%zu: %s###mon_%zu", i, m.name.c_str(), i);

      if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Position: (%.1f, %.1f)", m.position.x, m.position.y);
        ImGui::Text("Size: %.1f x %.1f logical (%.1f x %.1f mm)", m.size.x, m.size.y, (float)m.physical_size.x, (float)m.physical_size.y);
        ImGui::Text("Scale: %.2f", m.scale);

        if (ImGui::TreeNode("Available Modes")) {
            if (ImGui::BeginTable("ModesTable", 2, ImGuiTableFlags_Borders)) {
                ImGui::TableSetupColumn("Resolution");
                ImGui::TableSetupColumn("Refresh Rate");
                ImGui::TableHeadersRow();

                for (const auto& mode : m.modes) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%u x %u", mode.size.x, mode.size.y);
                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f Hz", (float)mode.refresh_rate_mhz / 1000.0f);
                }
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
      }
    }
  }

  ImGui::End();
}
