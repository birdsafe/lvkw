#include "clipboard_module.hpp"
#include "imgui.h"
#include "lvkw/lvkw.hpp"

ClipboardModule::ClipboardModule() {}

void ClipboardModule::update(lvkw::Context &ctx, lvkw::Window &window) {
  (void)ctx; (void)window;
  if (!enabled_)
    return;
}

void ClipboardModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Clipboard", &enabled_)) {
    ImGui::End();
    return;
  }

  const char* target_names[] = {"Clipboard", "Primary Selection"};
  ImGui::Combo("Target", &target_index_, target_names, IM_ARRAYSIZE(target_names));
  LVKW_DataExchangeTarget target = (target_index_ == 0) ? LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD : LVKW_DATA_EXCHANGE_TARGET_PRIMARY;

  if (ImGui::CollapsingHeader("Text Operations", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputTextMultiline("##clip_input", input_buffer_, sizeof(input_buffer_), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5));
    
    if (ImGui::Button("Push Text")) {
      try {
        window.pushText(target, input_buffer_);
      } catch (const lvkw::Exception& e) {
        // Fallback for backends that don't support primary selection
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Pull Text")) {
      try {
        const char* text = window.pullText(target);
        if (text) {
          strncpy(input_buffer_, text, sizeof(input_buffer_) - 1);
          input_buffer_[sizeof(input_buffer_) - 1] = '\0';
        }
      } catch (const lvkw::Exception& e) {}
    }
  }

  if (ImGui::CollapsingHeader("MIME Data", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Refresh MIME Types")) {
      mime_types_.clear();
      try {
        std::vector<const char*> mimes = window.listBufferMimeTypes(target);
        for (const char* m : mimes) {
          mime_types_.push_back(m);
        }
      } catch (const lvkw::Exception& e) {}
    }

    ImGui::Text("Available MIME Types (%zu):", mime_types_.size());
    if (ImGui::BeginChild("MimeList", ImVec2(0, 150), true)) {
      for (const auto& mime : mime_types_) {
        ImGui::BulletText("%s", mime.c_str());
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100);
        char label[64];
        snprintf(label, sizeof(label), "Read##%s", mime.c_str());
        if (ImGui::SmallButton(label)) {
          // In a real app we'd read binary data here
          // window.getClipboardData(mime.c_str(), &data, &size);
        }
      }
    }
    ImGui::EndChild();
  }

  ImGui::End();
}
