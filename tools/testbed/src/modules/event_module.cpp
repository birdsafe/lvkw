#include "event_module.hpp"
#include "imgui.h"
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <vector>
#include <thread>
#include <chrono>
#include "lvkw/lvkw-context.h" // For lvkw_ctx_update
#include "lvkw/lvkw-metrics.h"

// Helper for short names
static const char *getShortName(LVKW_EventType type) {
  switch (type) {
  case LVKW_EVENT_TYPE_CLOSE_REQUESTED: return "Close";
  case LVKW_EVENT_TYPE_WINDOW_RESIZED: return "Resize";
  case LVKW_EVENT_TYPE_KEY: return "Key";
  case LVKW_EVENT_TYPE_WINDOW_READY: return "Ready";
  case LVKW_EVENT_TYPE_MOUSE_MOTION: return "Motion";
  case LVKW_EVENT_TYPE_MOUSE_BUTTON: return "Button";
  case LVKW_EVENT_TYPE_MOUSE_SCROLL: return "Scroll";
  case LVKW_EVENT_TYPE_IDLE_STATE_CHANGED: return "Idle";
  case LVKW_EVENT_TYPE_MONITOR_CONNECTION: return "MonConn";
  case LVKW_EVENT_TYPE_MONITOR_MODE: return "MonMode";
  case LVKW_EVENT_TYPE_TEXT_INPUT: return "Text";
  case LVKW_EVENT_TYPE_FOCUS: return "Focus";
  case LVKW_EVENT_TYPE_WINDOW_MAXIMIZED: return "Maximize";
  case LVKW_EVENT_TYPE_DND_HOVER: return "DnD Hov";
  case LVKW_EVENT_TYPE_DND_LEAVE: return "DnD Leav";
  case LVKW_EVENT_TYPE_DND_DROP: return "DnD Drop";
  case LVKW_EVENT_TYPE_TEXT_COMPOSITION: return "Compose";
#ifdef LVKW_ENABLE_CONTROLLER
  case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION: return "CtrlConn";
#endif
  case LVKW_EVENT_TYPE_USER_0: return "User0";
  case LVKW_EVENT_TYPE_USER_1: return "User1";
  case LVKW_EVENT_TYPE_USER_2: return "User2";
  case LVKW_EVENT_TYPE_USER_3: return "User3";
  default: return "?";
  }
}

EventModule::EventModule() {
  event_toggles_ = {
      {LVKW_EVENT_TYPE_CLOSE_REQUESTED, true},
      {LVKW_EVENT_TYPE_WINDOW_RESIZED, true},
      {LVKW_EVENT_TYPE_KEY, true},
      {LVKW_EVENT_TYPE_WINDOW_READY, true},
      {LVKW_EVENT_TYPE_MOUSE_MOTION, true},
      {LVKW_EVENT_TYPE_MOUSE_BUTTON, true},
      {LVKW_EVENT_TYPE_MOUSE_SCROLL, true},
      {LVKW_EVENT_TYPE_IDLE_STATE_CHANGED, true},
      {LVKW_EVENT_TYPE_MONITOR_CONNECTION, true},
      {LVKW_EVENT_TYPE_MONITOR_MODE, true},
      {LVKW_EVENT_TYPE_TEXT_INPUT, true},
      {LVKW_EVENT_TYPE_FOCUS, true},
      {LVKW_EVENT_TYPE_WINDOW_MAXIMIZED, true},
      {LVKW_EVENT_TYPE_DND_HOVER, true},
      {LVKW_EVENT_TYPE_DND_LEAVE, true},
      {LVKW_EVENT_TYPE_DND_DROP, true},
      {LVKW_EVENT_TYPE_TEXT_COMPOSITION, true},
#ifdef LVKW_ENABLE_CONTROLLER
      {LVKW_EVENT_TYPE_CONTROLLER_CONNECTION, true},
#endif
      {LVKW_EVENT_TYPE_USER_0, true},
      {LVKW_EVENT_TYPE_USER_1, true},
      {LVKW_EVENT_TYPE_USER_2, true},
      {LVKW_EVENT_TYPE_USER_3, true},
  };

  for (const auto &pair : event_toggles_) {
    if (pair.second) {
      base_mask_ |= pair.first;
    }
  }
}

void EventModule::updateEventMask(lvkw::Context &ctx) {
  base_mask_ = 0;
  for (const auto &pair : event_toggles_) {
    if (pair.second) {
      base_mask_ |= pair.first;
    }
  }

  uint32_t mask = base_mask_;
  if (!strict_filtering_) {
    mask |= LVKW_EVENT_TYPE_MOUSE_MOTION | LVKW_EVENT_TYPE_MOUSE_BUTTON |
            LVKW_EVENT_TYPE_MOUSE_SCROLL;
  }

  LVKW_ContextAttributes attrs = {};
  attrs.event_mask = static_cast<LVKW_EventType>(mask);
  lvkw::check(lvkw_ctx_update(ctx.get(), LVKW_CTX_ATTR_EVENT_MASK, &attrs),
              "Failed to update event mask");
}

void EventModule::update(lvkw::Context &ctx, lvkw::Window &window) {
  (void)ctx; (void)window;
}

  

struct EventGroup {
    const char* name;
    std::vector<LVKW_EventType> events;
};

void EventModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;
    
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

  ImGui::Begin("Event System", &enabled_);

  if (ImGui::CollapsingHeader("Event Mask Control", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool changed = false;
    if (ImGui::Button("Enable All")) {
      for (auto &pair : event_toggles_)
        pair.second = true;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Disable All")) {
      for (auto &pair : event_toggles_)
        pair.second = false;
      changed = true;
    }

    if (ImGui::Checkbox("Strict Filtering", &strict_filtering_)) {
      updateEventMask(ctx);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("WARNING! \nToggling this makes the event log 100%% truthful, but you might soft-lock yourself.\nBy default, mouse events still flow through so that ImGui stays functional, and we just don't list them in the log. ");
    }

    static const std::vector<EventGroup> groups = {
        {"Window", {
            LVKW_EVENT_TYPE_CLOSE_REQUESTED,
            LVKW_EVENT_TYPE_WINDOW_RESIZED,
            LVKW_EVENT_TYPE_WINDOW_READY,
            LVKW_EVENT_TYPE_WINDOW_MAXIMIZED,
            LVKW_EVENT_TYPE_FOCUS
        }},
        {"Keyboard", {
            LVKW_EVENT_TYPE_KEY,
            LVKW_EVENT_TYPE_TEXT_INPUT,
            LVKW_EVENT_TYPE_TEXT_COMPOSITION
        }},
        {"Mouse", {
            LVKW_EVENT_TYPE_MOUSE_MOTION,
            LVKW_EVENT_TYPE_MOUSE_BUTTON,
            LVKW_EVENT_TYPE_MOUSE_SCROLL
        }},
        {"Monitor & Sys", {
            LVKW_EVENT_TYPE_MONITOR_CONNECTION,
            LVKW_EVENT_TYPE_MONITOR_MODE,
            LVKW_EVENT_TYPE_IDLE_STATE_CHANGED,
#ifdef LVKW_ENABLE_CONTROLLER
            LVKW_EVENT_TYPE_CONTROLLER_CONNECTION
#endif
        }},
        {"Drag & Drop", {
            LVKW_EVENT_TYPE_DND_HOVER,
            LVKW_EVENT_TYPE_DND_LEAVE,
            LVKW_EVENT_TYPE_DND_DROP
        }},
        {"User", {
            LVKW_EVENT_TYPE_USER_0,
            LVKW_EVENT_TYPE_USER_1,
            LVKW_EVENT_TYPE_USER_2,
            LVKW_EVENT_TYPE_USER_3
        }}
    };

    if (ImGui::BeginTable("mask_groups", (int)groups.size(), ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame)) {
        for (const auto& group : groups) {
            ImGui::TableNextColumn();
            
            // Calculate group state
            bool all_on = true;
            for (auto type : group.events) {
                if (!event_toggles_[type]) {
                    all_on = false;
                    break;
                }
            }
            
            bool group_toggle = all_on;
            if (ImGui::Checkbox(group.name, &group_toggle)) {
                for (auto type : group.events) {
                    event_toggles_[type] = group_toggle;
                }
                changed = true;
            }
            
            ImGui::Separator();
            for (auto type : group.events) {
                // event_toggles_ key is uint32_t, LVKW_EventType casts implicitly
                if (ImGui::Checkbox(getShortName(type), &event_toggles_[type])) {
                    changed = true;
                }
            }
        }
        
        ImGui::EndTable();
    }

    if (changed) {
      updateEventMask(ctx);
    }
  }

  if (ImGui::CollapsingHeader("Live Monitor", ImGuiTreeNodeFlags_DefaultOpen)) {
      renderAnimation();
  }

  if (ImGui::CollapsingHeader("User Events", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Post User Event 0")) {
      lvkw::postEvent(ctx, LVKW_EVENT_TYPE_USER_0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Post User Event 1")) {
      lvkw::postEvent(ctx, LVKW_EVENT_TYPE_USER_1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Post User Event 2")) {
      lvkw::postEvent(ctx, LVKW_EVENT_TYPE_USER_2);
    }
    ImGui::SameLine();
    if (ImGui::Button("Post User Event 3")) {
      lvkw::postEvent(ctx, LVKW_EVENT_TYPE_USER_3);
    }
  }

  ImGui::End();
}

void EventModule::renderAnimation() {
    if (ImGui::BeginChild("Animation", ImVec2(0, 200), true)) {
      ImDrawList *draw_list = ImGui::GetWindowDrawList();
      ImVec2 p = ImGui::GetCursorScreenPos();
      ImVec2 size = ImGui::GetContentRegionAvail();

      // Update position
      anim_x_ += anim_dx_;
      anim_y_ += anim_dy_;

      // Bounce
      if (anim_x_ < 0) {
        anim_x_ = 0;
        anim_dx_ *= -1;
      }
      if (anim_x_ > size.x - 50) {
        anim_x_ = size.x - 50;
        anim_dx_ *= -1;
      }
      if (anim_y_ < 0) {
        anim_y_ = 0;
        anim_dy_ *= -1;
      }
      if (anim_y_ > size.y - 20) {
        anim_y_ = size.y - 20;
        anim_dy_ *= -1;
      }

      char buf[32];
      snprintf(buf, sizeof(buf), "LVKW");
      draw_list->AddText(ImVec2(p.x + anim_x_, p.y + anim_y_),
                         IM_COL32(255, 255, 0, 255), buf);
    }
    ImGui::EndChild();
}

uint32_t EventModule::getRequestedSyncTimeout() {
  if (sync_request_time_ > 0.0 && ImGui::GetTime() >= sync_request_time_) {
    sync_request_time_ = -1.0;
    return sync_timeout_ * 1000;
  }
  return 0;
}
