#include "controller_module.hpp"
#include "imgui.h"
#include <algorithm>

ControllerModule::ControllerModule() {}

void ControllerModule::update(lvkw::Context &ctx, lvkw::Window &window) {
#ifdef LVKW_ENABLE_CONTROLLER
  if (first_update_) {
    first_update_ = false;
    auto ids = ctx.listControllers();
    for (auto id : ids) {
      try {
        auto controller = std::make_unique<lvkw::Controller>(ctx.createController(id));
        LVKW_CtrlInfo info = controller->getInfo();
        ControllerState state;
        state.controller = std::move(controller);
        state.info = info;
        state.haptic_levels.resize(state.controller->get()->haptic_count, 0.0f);
        controllers_[id] = std::move(state);
      } catch (...) {}
    }
  }

  lvkw::scanEvents(ctx, LVKW_EVENT_TYPE_CONTROLLER_CONNECTION,
                   [&](LVKW_EventType type, LVKW_Window *w, const LVKW_Event &e) {
                     if (e.controller_connection.connected) {
                       if (controllers_.find(e.controller_connection.id) != controllers_.end()) return;
                       try {
                         auto controller = std::make_unique<lvkw::Controller>(
                             ctx.createController(e.controller_connection.id));
                         LVKW_CtrlInfo info = controller->getInfo();
                         
                         ControllerState state;
                         state.controller = std::move(controller);
                         state.info = info;
                         state.haptic_levels.resize(state.controller->get()->haptic_count, 0.0f);
                         
                         controllers_[e.controller_connection.id] = std::move(state);
                       } catch (...) {}
                     } else {
                       controllers_.erase(e.controller_connection.id);
                     }
                   });

  // Cleanup lost controllers
  for (auto it = controllers_.begin(); it != controllers_.end();) {
    if (it->second.controller->isLost()) {
      it = controllers_.erase(it);
    } else {
      ++it;
    }
  }
#endif
}

void ControllerModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Controllers", &enabled_)) {
    ImGui::End();
    return;
  }

#ifndef LVKW_ENABLE_CONTROLLER
  ImGui::TextColored(ImVec4(1, 0, 0, 1), "LVKW_ENABLE_CONTROLLER is NOT defined!");
#else
  if (controllers_.empty()) {
    ImGui::Text("No controllers connected.");
  } else {
    if (ImGui::Button("Close All")) {
        controllers_.clear();
    }
    
    for (auto &pair : controllers_) {
      renderController(pair.first, pair.second);
    }
  }
#endif

  ImGui::End();
}

#ifdef LVKW_ENABLE_CONTROLLER
void ControllerModule::renderController(LVKW_CtrlId id, ControllerState &state) {
  char label[128];
  snprintf(label, sizeof(label), "%s (ID: %u)###ctrl_%u", state.info.name, id, id);

  if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("VID: 0x%04X PID: 0x%04X Ver: %u", state.info.vendor_id,
                state.info.product_id, state.info.version);
    ImGui::Text("Standardized: %s", state.info.is_standardized ? "Yes" : "No");

    auto *c = state.controller->get();

    if (ImGui::BeginTable("CtrlTable", 2, ImGuiTableFlags_BordersInnerV)) {
      ImGui::TableNextColumn();
      ImGui::Text("Axes (%u)", c->analog_count);
      for (uint32_t i = 0; i < c->analog_count; ++i) {
        float val = (float)c->analogs[i].value;
        ImGui::PushID(i);
        ImGui::ProgressBar((val + 1.0f) * 0.5f, ImVec2(-FLT_MIN, 0),
                           c->analog_channels[i].name);
        ImGui::PopID();
      }

      ImGui::TableNextColumn();
      ImGui::Text("Buttons (%u)", c->button_count);
      for (uint32_t i = 0; i < c->button_count; ++i) {
        bool pressed = (c->buttons[i] == LVKW_BUTTON_STATE_PRESSED);
        ImGui::Selectable(c->button_channels[i].name, pressed, 0, ImVec2(0, 0));
      }
      ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Text("Haptics / Rumble (%u channels)", c->haptic_count);
    if (c->haptic_count > 0) {
      bool changed = false;
      for (uint32_t i = 0; i < c->haptic_count; ++i) {
          ImGui::PushID(i);
          changed |= ImGui::SliderFloat(c->haptic_channels[i].name, &state.haptic_levels[i], 0.0f, 1.0f);
          ImGui::PopID();
      }

      if (changed) {
          std::vector<LVKW_Scalar> levels(state.haptic_levels.begin(), state.haptic_levels.end());
          state.controller->setHapticLevels(0, (uint32_t)levels.size(), levels.data());
      }
    } else {
        ImGui::TextDisabled("No haptic channels available.");
    }
    
    if (ImGui::Button("Stop Rumble")) {
        std::fill(state.haptic_levels.begin(), state.haptic_levels.end(), 0.0f);
        std::vector<LVKW_Scalar> levels(state.haptic_levels.size(), 0.0);
        state.controller->setHapticLevels(0, (uint32_t)levels.size(), levels.data());
    }
  }
}
#endif
