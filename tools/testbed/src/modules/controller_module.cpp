#include "controller_module.hpp"
#include "imgui.h"
#include <algorithm>
#include <cstdio>

ControllerModule::ControllerModule() {}
ControllerModule::~ControllerModule() {
#ifdef LVKW_ENABLE_CONTROLLER
  for (auto &pair : controllers_) {
    if (pair.second.controller) {
      lvkw_input_destroyController(pair.second.controller);
    }
  }
  controllers_.clear();
#endif
}

void ControllerModule::update(lvkw::Context &ctx, lvkw::Window &window) {
#ifdef LVKW_ENABLE_CONTROLLER
  if (first_update_) {
    first_update_ = false;
    auto refs = ctx.listControllers();
    for (auto *ref : refs) {
      try {
        LVKW_Controller *controller = nullptr;
        if (!ref || lvkw_input_createController(ref, &controller) != LVKW_SUCCESS ||
            controllers_.find(controller) != controllers_.end()) {
          continue;
        }
        LVKW_CtrlInfo info = {};
        lvkw_input_getControllerInfo(controller, &info);
        ControllerState state;
        state.controller = controller;
        state.info = info;
        state.haptic_levels.resize(controller->haptic_count, 0.0f);
        controllers_[controller] = std::move(state);
      } catch (...) {}
    }
  }

  lvkw::scanEvents(ctx, LVKW_EVENT_TYPE_CONTROLLER_CONNECTION,
                   [&](LVKW_EventType type, LVKW_Window *w, const LVKW_Event &e) {
                     LVKW_ControllerRef *ref = e.controller_connection.controller_ref;
                     if (!ref) return;
                     if (e.controller_connection.connected) {
                       if (controllers_.find((LVKW_Controller *)ref) != controllers_.end()) return;
                       LVKW_Controller *handle = nullptr;
                       if (lvkw_input_createController(ref, &handle) != LVKW_SUCCESS) return;
                       try {
                         LVKW_CtrlInfo info = {};
                         lvkw_input_getControllerInfo(handle, &info);
                         
                         ControllerState state;
                         state.controller = handle;
                         state.info = info;
                         state.haptic_levels.resize(state.controller->haptic_count, 0.0f);
                         
                         controllers_[handle] = std::move(state);
                       } catch (...) {}
                     } else {
                       LVKW_Controller *handle = (LVKW_Controller *)ref;
                       auto it = controllers_.find(handle);
                       if (it != controllers_.end()) {
                         lvkw_input_destroyController(it->second.controller);
                         controllers_.erase(it);
                       }
                     }
                   });

  // Cleanup lost controllers
  for (auto it = controllers_.begin(); it != controllers_.end();) {
    if (it->second.controller->flags & LVKW_CONTROLLER_STATE_LOST) {
      lvkw_input_destroyController(it->second.controller);
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
        for (auto &pair : controllers_) {
          lvkw_input_destroyController(pair.second.controller);
        }
        controllers_.clear();
    }
    
    for (auto &pair : controllers_) {
      renderController(pair.first, pair.second);
    }
  }
#endif

  ImGui::End();
}

void ControllerModule::onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) {
  (void)ctx;
  (void)window;
#ifdef LVKW_ENABLE_CONTROLLER
  for (auto &pair : controllers_) {
    if (pair.second.controller) {
      lvkw_input_destroyController(pair.second.controller);
    }
  }
  controllers_.clear();
#endif
  first_update_ = true;
}

#ifdef LVKW_ENABLE_CONTROLLER
void ControllerModule::renderController(LVKW_Controller *handle, ControllerState &state) {
  char label[128];
  snprintf(label, sizeof(label), "%s###ctrl_%p", state.info.name, (void *)handle);

  if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("VID: 0x%04X PID: 0x%04X Ver: %u", state.info.vendor_id,
                state.info.product_id, state.info.version);
    ImGui::Text("Standardized: %s", state.info.is_standardized ? "Yes" : "No");

    auto *c = state.controller;

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
          lvkw_input_setControllerHapticLevels(state.controller, 0, (uint32_t)levels.size(),
                                               levels.data());
      }
    } else {
        ImGui::TextDisabled("No haptic channels available.");
    }
    
    if (ImGui::Button("Stop Rumble")) {
        std::fill(state.haptic_levels.begin(), state.haptic_levels.end(), 0.0f);
        std::vector<LVKW_Scalar> levels(state.haptic_levels.size(), 0.0);
        lvkw_input_setControllerHapticLevels(state.controller, 0, (uint32_t)levels.size(),
                                             levels.data());
    }
  }
}
#endif
