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

  if (ImGui::CollapsingHeader("Text Operations", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputTextMultiline("##clip_input", input_buffer_, sizeof(input_buffer_), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5));
    
    if (ImGui::Button("Copy to Clipboard")) {
      window.setClipboardText(input_buffer_);
    }
    ImGui::SameLine();
    if (ImGui::Button("Paste from Clipboard")) {
      try {
        const char* text = window.getClipboardText();
        if (text) {
          strncpy(input_buffer_, text, sizeof(input_buffer_) - 1);
          input_buffer_[sizeof(input_buffer_) - 1] = '\0';
        }
      } catch (...) {}
    }
  }

  if (ImGui::CollapsingHeader("MIME Data", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Refresh MIME Types")) {
      mime_types_.clear();
      try {
        std::vector<const char*> mimes = window.getClipboardMimeTypes();
        for (const char* m : mimes) {
          mime_types_.push_back(m);
        }
      } catch (...) {}
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
