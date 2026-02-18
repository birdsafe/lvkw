#include "cursor_module.hpp"
#include "imgui.h"
#include "lvkw/lvkw.h"
#include <vector>

struct ShapeEntry {
    const char* name;
    LVKW_CursorShape shape;
};

static const ShapeEntry standard_shapes[] = {
    {"Default/Arrow", LVKW_CURSOR_SHAPE_DEFAULT},
    {"Help", LVKW_CURSOR_SHAPE_HELP},
    {"Hand", LVKW_CURSOR_SHAPE_HAND},
    {"Wait/Busy", LVKW_CURSOR_SHAPE_WAIT},
    {"Crosshair", LVKW_CURSOR_SHAPE_CROSSHAIR},
    {"Text/IBeam", LVKW_CURSOR_SHAPE_TEXT},
    {"Move", LVKW_CURSOR_SHAPE_MOVE},
    {"Not Allowed", LVKW_CURSOR_SHAPE_NOT_ALLOWED},
    {"Resize EW", LVKW_CURSOR_SHAPE_EW_RESIZE},
    {"Resize NS", LVKW_CURSOR_SHAPE_NS_RESIZE},
    {"Resize NESW", LVKW_CURSOR_SHAPE_NESW_RESIZE},
    {"Resize NWSE", LVKW_CURSOR_SHAPE_NWSE_RESIZE},
};

CursorModule::CursorModule() {}

void CursorModule::onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& evt) {
    if (!enabled_) return;

    if (type == LVKW_EVENT_TYPE_KEY) {
        if (evt.key.key == LVKW_KEY_ESCAPE && evt.key.state == LVKW_BUTTON_STATE_PRESSED) {
            if (current_mode_ != LVKW_CURSOR_NORMAL) {
                current_mode_ = LVKW_CURSOR_NORMAL;
                pending_cursor_.mode = LVKW_CURSOR_NORMAL;
                pending_cursor_.mode_changed = true;
            }
        }
    }
}

void CursorModule::update(lvkw::Context &ctx, lvkw::Window &window) {
    if (!enabled_) return;

    if (pending_cursor_.cursor_changed) {
        window.setCursor(pending_cursor_.cursor);
        pending_cursor_.cursor_changed = false;
    }

    if (pending_cursor_.mode_changed) {
        window.setCursorMode(pending_cursor_.mode);
        pending_cursor_.mode_changed = false;
    }
}

void CursorModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Cursors", &enabled_)) {
    ImGui::End();
    return;
  }

  if (ImGui::CollapsingHeader("Cursor Mode", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (ImGui::RadioButton("Normal", current_mode_ == LVKW_CURSOR_NORMAL)) {
          current_mode_ = LVKW_CURSOR_NORMAL;
          pending_cursor_.mode = LVKW_CURSOR_NORMAL;
          pending_cursor_.mode_changed = true;
      }
      if (ImGui::RadioButton("Hidden", current_mode_ == LVKW_CURSOR_HIDDEN)) {
          current_mode_ = LVKW_CURSOR_HIDDEN;
          pending_cursor_.mode = LVKW_CURSOR_HIDDEN;
          pending_cursor_.mode_changed = true;
      }
      if (ImGui::RadioButton("Locked/Captured", current_mode_ == LVKW_CURSOR_LOCKED)) {
          current_mode_ = LVKW_CURSOR_LOCKED;
          pending_cursor_.mode = LVKW_CURSOR_LOCKED;
          pending_cursor_.mode_changed = true;
      }
      if (current_mode_ != LVKW_CURSOR_NORMAL) {
          ImGui::SameLine();
          ImGui::TextDisabled("(Press ESC to unlock)");
      }
  }

  if (ImGui::CollapsingHeader("Standard Shapes", ImGuiTreeNodeFlags_DefaultOpen)) {
      for (int i = 0; i < (int)IM_ARRAYSIZE(standard_shapes); ++i) {
          if (ImGui::Selectable(standard_shapes[i].name, selected_shape_idx_ == i)) {
              selected_shape_idx_ = i;
              pending_cursor_.cursor = ctx.getStandardCursor(standard_shapes[i].shape);
              pending_cursor_.cursor_changed = true;
              cursor_override_active_ = true;
          }
      }
  }

  if (ImGui::CollapsingHeader("Custom Cursor", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (ImGui::Button("Create/Apply Yellow Square")) {
          createCustomCursor(ctx);
          if (custom_cursor_) {
              pending_cursor_.cursor = custom_cursor_->get();
              pending_cursor_.cursor_changed = true;
              cursor_override_active_ = true;
          }
      }
      if (custom_cursor_) {
          ImGui::SameLine();
          if (ImGui::Button("Destroy Custom")) {
              custom_cursor_.reset();
              // Revert to standard
              pending_cursor_.cursor = ctx.getStandardCursor(standard_shapes[selected_shape_idx_].shape);
              pending_cursor_.cursor_changed = true;
              cursor_override_active_ = true;
          }
      }
  }

  ImGui::End();
}

void CursorModule::onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) {
    (void)ctx;
    (void)window;
    custom_cursor_.reset();
    pending_cursor_ = {};
    current_mode_ = LVKW_CURSOR_NORMAL;
    selected_shape_idx_ = 0;
    cursor_override_active_ = false;
}

void CursorModule::createCustomCursor(lvkw::Context &ctx) {
    const int w = 32;
    const int h = 32;
    std::vector<uint32_t> pixels(w * h);
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (x == 0 || x == w - 1 || y == 0 || y == h - 1) {
                pixels[y * w + x] = 0xFF000000; // Black border (Alpha=255, B=0, G=0, R=0)
            } else {
                pixels[y * w + x] = 0xFF00FFFF; // Yellow fill (Alpha=255, B=0, G=255, R=255)
            }
        }
    }

    LVKW_CursorCreateInfo cci;
    cci.size = {w, h};
    cci.hot_spot = {w / 2, h / 2};
    cci.pixels = pixels.data();

    try {
        custom_cursor_ = std::make_unique<lvkw::Cursor>(ctx.createCursor(cci));
    } catch (...) {
        // Handle error
    }
}
