#pragma once

#include "../feature_module.hpp"
#include <map>
#include <string>
#include <vector>

class ControllerModule : public FeatureModule {
public:
  ControllerModule();
  virtual ~ControllerModule();

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;
  void onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) override;

  void onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& event);

  const char *getName() const override { return "Controllers"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

#ifdef LVKW_ENABLE_CONTROLLER
  struct ControllerState {
    LVKW_Controller *controller;
    LVKW_CtrlInfo info;
    std::vector<float> haptic_levels;
  };

  std::map<LVKW_Controller *, ControllerState> controllers_;
  void renderController(LVKW_Controller *handle, ControllerState &state);
#endif

  bool first_update_ = true;
};
