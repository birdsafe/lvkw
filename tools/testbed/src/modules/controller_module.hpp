#pragma once

#include "../feature_module.hpp"
#include <map>
#include <memory>
#include <vector>

class ControllerModule : public FeatureModule {
public:
  ControllerModule();
  virtual ~ControllerModule();

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Controllers"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;
  bool first_update_ = true;

#ifdef LVKW_ENABLE_CONTROLLER
  struct ControllerState {
      LVKW_Controller *controller = nullptr;
      LVKW_CtrlInfo info;
      std::vector<float> haptic_levels;
  };

  std::map<LVKW_Controller *, ControllerState> controllers_;

  void renderController(LVKW_Controller *handle, ControllerState &state);
#endif
};
