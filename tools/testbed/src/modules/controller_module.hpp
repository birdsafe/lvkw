#pragma once

#include "../feature_module.hpp"
#include <map>
#include <memory>
#include <vector>

class ControllerModule : public FeatureModule {
public:
  ControllerModule();
  virtual ~ControllerModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Controllers"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;
  bool first_update_ = true;

  struct ControllerState {
      std::unique_ptr<lvkw::Controller> controller;
      LVKW_CtrlInfo info;
      std::vector<float> haptic_levels;
  };

  std::map<LVKW_CtrlId, ControllerState> controllers_;

  void renderController(LVKW_CtrlId id, ControllerState &state);
};
