#pragma once

#include "../feature_module.hpp"
#include <vector>
#include <string>
#include <array>

class InputModule : public FeatureModule {
public:
  InputModule();
  virtual ~InputModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Input Tester"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

  // Mouse State
  LVKW_LogicalVec mouse_pos_ = {0.0f, 0.0f};
  LVKW_LogicalVec mouse_delta_ = {0.0f, 0.0f};
  LVKW_LogicalVec mouse_delta_raw_ = {0.0f, 0.0f};
  std::array<bool, 8> mouse_buttons_ = {false};
  float mouse_wheel_ = 0.0f;

  // Keyboard State
  std::array<bool, 512> keys_down_ = {false};
  std::string last_text_input_;
  std::string last_text_composition_;

  void renderMouseSection();
  void renderKeyboardSection();
  void renderTextSection();
};
