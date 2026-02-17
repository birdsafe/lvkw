#include "monitor_module.hpp"
#include "imgui.h"
#include "lvkw/lvkw.hpp"

MonitorModule::MonitorModule() {}

void MonitorModule::update(lvkw::Context &ctx, lvkw::Window &window) {
  (void)ctx; (void)window;
  if (!enabled_)
    return;
}

void MonitorModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Monitors", &enabled_)) {
    ImGui::End();
    return;
  }

  if (ImGui::Button("Refresh Monitors")) {
    refreshMonitors(ctx);
  }

  ImGui::Separator();
  ImGui::Text("Connected Monitors: %zu", monitors_.size());

  for (const auto& mon : monitors_) {
    char label[128];
    snprintf(label, sizeof(label), "%s###mon_%p", mon.name.c_str(), (void*)mon.ref);
    if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("Position:  %.1f, %.1f", mon.position.x, mon.position.y);
      ImGui::Text("Size (Log): %.1f x %.1f", mon.size.x, mon.size.y);
      ImGui::Text("Size (Phys): %d x %d", mon.physical_size.x, mon.physical_size.y);
      ImGui::Text("Scale:     %.2f", mon.scale);

      if (ImGui::TreeNode("Supported Video Modes")) {
        if (ImGui::BeginTable("ModesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
          ImGui::TableSetupColumn("Resolution");
          ImGui::TableSetupColumn("Refresh Rate");
          ImGui::TableHeadersRow();

          for (const auto& mode : mon.modes) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d x %d", mode.size.x, mode.size.y);
            ImGui::TableNextColumn();
            ImGui::Text("%.2f Hz", (float)mode.refresh_rate_mhz / 1000.0f);
          }
          ImGui::EndTable();
        }
        ImGui::TreePop();
      }
    }
  }

  ImGui::End();
}

void MonitorModule::refreshMonitors(lvkw::Context &ctx) {
  monitors_.clear();
  try {
    std::vector<LVKW_MonitorRef*> refs = ctx.getMonitors();
    for (auto ref : refs) {
      LVKW_Monitor* h = ctx.createMonitor(ref);
      MonitorInfo info;
      info.ref = ref;
      info.name = h->name ? h->name : "Unknown Monitor";
      info.position = h->logical_position;
      info.size = h->logical_size;
      info.physical_size = {(int32_t)h->physical_size.x, (int32_t)h->physical_size.y};
      info.scale = h->scale;
      
      try {
        info.modes = ctx.getMonitorModes(h);
      } catch (...) {}

      lvkw_display_destroyMonitor(h);
      monitors_.push_back(info);
    }
  } catch (...) {}
}
