#include "input_module.hpp"
#include "imgui.h"
#include <iostream>

InputModule::InputModule() {}

void InputModule::onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& e) {
  if (!enabled_) return;

  switch (type) {
    case LVKW_EVENT_TYPE_MOUSE_MOTION:
      mouse_pos_ = e.mouse_motion.position;
      mouse_delta_ = e.mouse_motion.delta;
      mouse_delta_raw_ = e.mouse_motion.raw_delta;
      break;

    case LVKW_EVENT_TYPE_MOUSE_BUTTON:
      if (e.mouse_button.button < (int)mouse_buttons_.size()) {
        mouse_buttons_[e.mouse_button.button] = (e.mouse_button.state == LVKW_BUTTON_STATE_PRESSED);
      }
      break;

    case LVKW_EVENT_TYPE_MOUSE_SCROLL:
      mouse_wheel_ += (float)e.mouse_scroll.delta.y;
      break;

    case LVKW_EVENT_TYPE_KEY:
      if (e.key.key < (int)keys_down_.size()) {
        keys_down_[e.key.key] = (e.key.state == LVKW_BUTTON_STATE_PRESSED);
      }
      break;

    case LVKW_EVENT_TYPE_TEXT_INPUT:
      if (e.text_input.text) {
        last_text_input_ = e.text_input.text;
      }
      break;

    case LVKW_EVENT_TYPE_TEXT_COMPOSITION:
      if (e.text_composition.text) {
        last_text_composition_ = e.text_composition.text;
      }
      break;

    default: break;
  }
}

void InputModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Input Tester", &enabled_)) {
    ImGui::End();
    return;
  }

  if (ImGui::BeginTabBar("InputTabs")) {
    if (ImGui::BeginTabItem("Mouse")) {
      renderMouseSection();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Keyboard")) {
      renderKeyboardSection();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Text & IME")) {
      renderTextSection();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }

  ImGui::End();
}

void InputModule::renderMouseSection() {
  ImGui::Text("Position: (%.2f, %.2f)", mouse_pos_.x, mouse_pos_.y);
  ImGui::Text("Delta: (%.2f, %.2f)", mouse_delta_.x, mouse_delta_.y);
  ImGui::Text("Raw Delta: (%.2f, %.2f)", mouse_delta_raw_.x, mouse_delta_raw_.y);
  ImGui::Text("Scroll Accum: %.2f", mouse_wheel_);

  ImGui::Separator();
  ImGui::Text("Buttons:");
  for (int i = 0; i < (int)mouse_buttons_.size(); ++i) {
    if (i > 0) ImGui::SameLine();
    bool active = mouse_buttons_[i];
    if (active) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    }
    char label[16];
    snprintf(label, sizeof(label), "B%d", i);
    ImGui::Button(label);
    if (active) {
      ImGui::PopStyleColor();
    }
  }

  // Draw a small coordinate plane to visualize delta
  ImGui::Separator();
  ImGui::Text("Movement Visualization (Relative)");
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size = ImVec2(200, 200);
  draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255));
  ImVec2 center = ImVec2(canvas_pos.x + canvas_size.x / 2, canvas_pos.y + canvas_size.y / 2);
  
  draw_list->AddLine(ImVec2(canvas_pos.x, center.y), ImVec2(canvas_pos.x + canvas_size.x, center.y), IM_COL32(100, 100, 100, 255));
  draw_list->AddLine(ImVec2(center.x, canvas_pos.y), ImVec2(center.x, canvas_pos.y + canvas_size.y), IM_COL32(100, 100, 100, 255));

  ImVec2 delta_vec = ImVec2(center.x + (float)mouse_delta_.x * 5.0f, center.y + (float)mouse_delta_.y * 5.0f);
  draw_list->AddLine(center, delta_vec, IM_COL32(255, 255, 0, 255), 2.0f);
  draw_list->AddCircleFilled(delta_vec, 3.0f, IM_COL32(255, 255, 0, 255));

  ImVec2 raw_delta_vec = ImVec2(center.x + (float)mouse_delta_raw_.x * 5.0f, center.y + (float)mouse_delta_raw_.y * 5.0f);
  draw_list->AddLine(center, raw_delta_vec, IM_COL32(0, 255, 255, 255), 1.0f);
  draw_list->AddCircleFilled(raw_delta_vec, 2.0f, IM_COL32(0, 255, 255, 255));

  ImGui::Dummy(canvas_size);
  ImGui::TextColored(ImVec4(1, 1, 0, 1), "Yellow: Delta");
  ImGui::TextColored(ImVec4(0, 1, 1, 1), "Cyan: Raw Delta");
}

void InputModule::renderKeyboardSection() {
  ImGui::Text("Active Keys:");
  ImGui::BeginChild("ActiveKeys", ImVec2(0, 100), true);
  for (int i = 0; i < (int)keys_down_.size(); ++i) {
    if (keys_down_[i]) {
      ImGui::Text("Key %d pressed", i);
    }
  }
  ImGui::EndChild();

  ImGui::Separator();
  ImGui::Text("Virtual Keyboard");

  auto draw_key = [&](LVKW_Key key, const char* label, float width = 40.0f) {
    bool active = (key >= 0 && key < (int)keys_down_.size()) && keys_down_[key];
    if (active) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    }
    ImGui::Button(label, ImVec2(width, 40));
    if (active) {
      ImGui::PopStyleColor();
    }
    ImGui::SameLine();
  };

  // Function Keys
  draw_key(LVKW_KEY_ESCAPE, "Esc");
  ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
  draw_key(LVKW_KEY_F1, "F1");
  draw_key(LVKW_KEY_F2, "F2");
  draw_key(LVKW_KEY_F3, "F3");
  draw_key(LVKW_KEY_F4, "F4");
  ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine();
  draw_key(LVKW_KEY_F5, "F5");
  draw_key(LVKW_KEY_F6, "F6");
  draw_key(LVKW_KEY_F7, "F7");
  draw_key(LVKW_KEY_F8, "F8");
  ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine();
  draw_key(LVKW_KEY_F9, "F9");
  draw_key(LVKW_KEY_F10, "F10");
  draw_key(LVKW_KEY_F11, "F11");
  draw_key(LVKW_KEY_F12, "F12");
  ImGui::NewLine();

  // Numbers Row
  draw_key(LVKW_KEY_GRAVE_ACCENT, "`");
  draw_key(LVKW_KEY_1, "1");
  draw_key(LVKW_KEY_2, "2");
  draw_key(LVKW_KEY_3, "3");
  draw_key(LVKW_KEY_4, "4");
  draw_key(LVKW_KEY_5, "5");
  draw_key(LVKW_KEY_6, "6");
  draw_key(LVKW_KEY_7, "7");
  draw_key(LVKW_KEY_8, "8");
  draw_key(LVKW_KEY_9, "9");
  draw_key(LVKW_KEY_0, "0");
  draw_key(LVKW_KEY_MINUS, "-");
  draw_key(LVKW_KEY_EQUAL, "=");
  draw_key(LVKW_KEY_BACKSPACE, "Backspace", 80.0f);
  ImGui::NewLine();

  // Row 1
  draw_key(LVKW_KEY_TAB, "Tab", 60.0f);
  draw_key(LVKW_KEY_Q, "Q");
  draw_key(LVKW_KEY_W, "W");
  draw_key(LVKW_KEY_E, "E");
  draw_key(LVKW_KEY_R, "R");
  draw_key(LVKW_KEY_T, "T");
  draw_key(LVKW_KEY_Y, "Y");
  draw_key(LVKW_KEY_U, "U");
  draw_key(LVKW_KEY_I, "I");
  draw_key(LVKW_KEY_O, "O");
  draw_key(LVKW_KEY_P, "P");
  draw_key(LVKW_KEY_LEFT_BRACKET, "[");
  draw_key(LVKW_KEY_RIGHT_BRACKET, "]");
  draw_key(LVKW_KEY_BACKSLASH, "\\", 60.0f);
  ImGui::NewLine();

  // Row 2
  draw_key(LVKW_KEY_CAPS_LOCK, "Caps", 70.0f);
  draw_key(LVKW_KEY_A, "A");
  draw_key(LVKW_KEY_S, "S");
  draw_key(LVKW_KEY_D, "D");
  draw_key(LVKW_KEY_F, "F");
  draw_key(LVKW_KEY_G, "G");
  draw_key(LVKW_KEY_H, "H");
  draw_key(LVKW_KEY_J, "J");
  draw_key(LVKW_KEY_K, "K");
  draw_key(LVKW_KEY_L, "L");
  draw_key(LVKW_KEY_SEMICOLON, ";");
  draw_key(LVKW_KEY_APOSTROPHE, "'");
  draw_key(LVKW_KEY_ENTER, "Enter", 90.0f);
  ImGui::NewLine();

  // Row 3
  draw_key(LVKW_KEY_LEFT_SHIFT, "Shift", 90.0f);
  draw_key(LVKW_KEY_Z, "Z");
  draw_key(LVKW_KEY_X, "X");
  draw_key(LVKW_KEY_C, "C");
  draw_key(LVKW_KEY_V, "V");
  draw_key(LVKW_KEY_B, "B");
  draw_key(LVKW_KEY_N, "N");
  draw_key(LVKW_KEY_M, "M");
  draw_key(LVKW_KEY_COMMA, ",");
  draw_key(LVKW_KEY_PERIOD, ".");
  draw_key(LVKW_KEY_SLASH, "/");
  draw_key(LVKW_KEY_RIGHT_SHIFT, "Shift", 110.0f);
  ImGui::NewLine();

  // Row 4
  draw_key(LVKW_KEY_LEFT_CONTROL, "Ctrl", 60.0f);
  draw_key(LVKW_KEY_LEFT_META, "Super", 60.0f);
  draw_key(LVKW_KEY_LEFT_ALT, "Alt", 60.0f);
  draw_key(LVKW_KEY_SPACE, "Space", 240.0f);
  draw_key(LVKW_KEY_RIGHT_ALT, "Alt", 60.0f);
  draw_key(LVKW_KEY_RIGHT_META, "Super", 60.0f);
  draw_key(LVKW_KEY_MENU, "Menu", 60.0f);
  draw_key(LVKW_KEY_RIGHT_CONTROL, "Ctrl", 60.0f);
  
  ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
  draw_key(LVKW_KEY_LEFT, "<");
  draw_key(LVKW_KEY_UP, "^");
  draw_key(LVKW_KEY_DOWN, "v");
  draw_key(LVKW_KEY_RIGHT, ">");
  ImGui::NewLine();
}

void InputModule::renderTextSection() {
  ImGui::Text("Last Text Input: %s", last_text_input_.c_str());
  ImGui::Text("Last Composition: %s", last_text_composition_.c_str());

  ImGui::Separator();
  ImGui::Text("Test Input Box:");
  static char buffer[128] = "";
  ImGui::InputText("##test_input", buffer, sizeof(buffer));
  
  ImGui::TextDisabled("Focus this box to test IME/Text events forwarding.");
}
