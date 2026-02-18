#pragma once

#include "../feature_module.hpp"
#include <array>
#include <string>

class InputModule : public FeatureModule {
public:
  InputModule();
  virtual ~InputModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override { (void)ctx; (void)window; }
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  void onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& event);

  const char *getName() const override { return "Input Tester"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

  LVKW_LogicalVec mouse_pos_ = {0, 0};
  LVKW_LogicalVec mouse_delta_ = {0, 0};
  LVKW_LogicalVec mouse_delta_raw_ = {0, 0};
  float mouse_wheel_ = 0.0f;
  std::array<bool, 8> mouse_buttons_{false};

  std::array<bool, 512> keys_down_{false};
  std::string last_text_input_;
  std::string last_text_composition_;

  void renderMouseSection();
  void renderKeyboardSection();
  void renderTextSection();
};
