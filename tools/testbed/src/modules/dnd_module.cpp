#include "dnd_module.hpp"
#include "imgui.h"
#include "lvkw/lvkw.hpp"

DndModule::DndModule() {}

void DndModule::update(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  current_session_.dropped = false;

  lvkw::scanEvents(
      ctx,
      [&](const lvkw::DndHoverEvent &e) {
        if (e.window != window.get()) return;
        
        current_session_.is_active = true;
        current_session_.position = e->position;
        current_session_.paths.clear();
        for (uint16_t i = 0; i < e->path_count; ++i) {
          current_session_.paths.push_back(e->paths[i]);
        }
        
        // Provide feedback
        e.action() = selected_feedback_action_;
      },
      [&](const lvkw::DndLeaveEvent &e) {
        if (e.window != window.get()) return;
        current_session_.is_active = false;
      },
      [&](const lvkw::DndDropEvent &e) {
        if (e.window != window.get()) return;
        
        current_session_.is_active = false;
        current_session_.dropped = true;
        current_session_.position = e->position;
        
        last_dropped_paths_.clear();
        for (uint16_t i = 0; i < e->path_count; ++i) {
          last_dropped_paths_.push_back(e->paths[i]);
        }
      });
}

void DndModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  (void)ctx;
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Drag & Drop", &enabled_)) {
    ImGui::End();
    return;
  }

  ImGui::Text("Feedback Action to Provide:");
  int selected_action = static_cast<int>(selected_feedback_action_);
  ImGui::RadioButton("Copy", &selected_action, LVKW_DND_ACTION_COPY); ImGui::SameLine();
  ImGui::RadioButton("Move", &selected_action, LVKW_DND_ACTION_MOVE); ImGui::SameLine();
  ImGui::RadioButton("Link", &selected_action, LVKW_DND_ACTION_LINK); ImGui::SameLine();
  ImGui::RadioButton("None", &selected_action, LVKW_DND_ACTION_NONE);
  selected_feedback_action_ = static_cast<LVKW_DndAction>(selected_action);

  ImGui::Separator();

  // Visual Drop Zone
  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 150);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImU32 bg_color = current_session_.is_active ? IM_COL32(100, 100, 200, 255) : IM_COL32(50, 50, 50, 255);
  draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), bg_color);
  draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(200, 200, 200, 255), 0, 0, 2.0f);

  const char* text = current_session_.is_active ? "DRAGGING OVER..." : "DROP ZONE (Accepts Files)";
  ImVec2 text_size = ImGui::CalcTextSize(text);
  draw_list->AddText(ImVec2(canvas_pos.x + (canvas_size.x - text_size.x) / 2, canvas_pos.y + (canvas_size.y - text_size.y) / 2), IM_COL32(255, 255, 255, 255), text);

  if (current_session_.is_active) {
      char buf[64];
      snprintf(buf, sizeof(buf), "Pos: (%.1f, %.1f)", current_session_.position.x, current_session_.position.y);
      draw_list->AddText(ImVec2(canvas_pos.x + 10, canvas_pos.y + canvas_size.y - 20), IM_COL32(255, 255, 0, 255), buf);
  }

  ImGui::Dummy(canvas_size);

  ImGui::Separator();
  ImGui::Text("Last Dropped Paths (%zu):", last_dropped_paths_.size());
  if (ImGui::BeginChild("DroppedPaths", ImVec2(0, 0), true)) {
    for (const auto& path : last_dropped_paths_) {
      ImGui::BulletText("%s", path.c_str());
    }
  }
  ImGui::EndChild();

  ImGui::End();
}
