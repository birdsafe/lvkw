#include "dnd_module.hpp"
#include "imgui.h"
#include "lvkw/lvkw.hpp"

DndModule::DndModule() {}

void DndModule::onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& evt) {
  if (!enabled_) return;

  switch (type) {
    case LVKW_EVENT_TYPE_DND_HOVER: {
      current_session_.is_active = true;
      current_session_.position = evt.dnd_hover.position;
      current_session_.paths.clear();
      for (uint32_t i = 0; i < evt.dnd_hover.path_count; ++i) {
        current_session_.paths.push_back(evt.dnd_hover.paths[i]);
      }
      
      // Recommended action isn't explicitly in hover event anymore, but we can set our feedback
      if (evt.dnd_hover.feedback && evt.dnd_hover.feedback->action) {
          *evt.dnd_hover.feedback->action = selected_feedback_action_;
      }
      break;
    }
    case LVKW_EVENT_TYPE_DND_LEAVE: {
      current_session_.is_active = false;
      current_session_.paths.clear();
      break;
    }
    case LVKW_EVENT_TYPE_DND_DROP: {
      current_session_.is_active = false;
      last_dropped_paths_.clear();
      for (uint32_t i = 0; i < evt.dnd_drop.path_count; ++i) {
        last_dropped_paths_.push_back(evt.dnd_drop.paths[i]);
      }
      break;
    }
    default: break;
  }
}

void DndModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Drag & Drop", &enabled_)) {
    ImGui::End();
    return;
  }

  if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
      const char* actions[] = {"Copy", "Move", "Link", "None"};
      int current_action = 0;
      if (selected_feedback_action_ == LVKW_DND_ACTION_MOVE) current_action = 1;
      else if (selected_feedback_action_ == LVKW_DND_ACTION_LINK) current_action = 2;
      else if (selected_feedback_action_ == LVKW_DND_ACTION_NONE) current_action = 3;

      if (ImGui::Combo("Feedback Action", &current_action, actions, IM_ARRAYSIZE(actions))) {
          if (current_action == 0) selected_feedback_action_ = LVKW_DND_ACTION_COPY;
          else if (current_action == 1) selected_feedback_action_ = LVKW_DND_ACTION_MOVE;
          else if (current_action == 2) selected_feedback_action_ = LVKW_DND_ACTION_LINK;
          else selected_feedback_action_ = LVKW_DND_ACTION_NONE;
      }
  }

  if (ImGui::CollapsingHeader("Active Session", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (current_session_.is_active) {
      ImGui::Text("Hovering at (%.1f, %.1f)", current_session_.position.x, current_session_.position.y);
      ImGui::Text("Paths (%zu):", current_session_.paths.size());
      for (const auto& path : current_session_.paths) {
        ImGui::BulletText("%s", path.c_str());
      }
    } else {
      ImGui::TextDisabled("No active DnD session.");
    }
  }

  if (ImGui::CollapsingHeader("Last Drop", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (last_dropped_paths_.empty()) {
      ImGui::TextDisabled("No files dropped yet.");
    } else {
      ImGui::Text("Last drop contained %zu paths:", last_dropped_paths_.size());
      for (const auto& path : last_dropped_paths_) {
        ImGui::BulletText("%s", path.c_str());
      }
      if (ImGui::Button("Clear History")) {
        last_dropped_paths_.clear();
      }
    }
  }

  ImGui::End();
}
