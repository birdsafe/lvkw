#pragma once

#include "../feature_module.hpp"
#include <vector>
#include <string>

class MonitorModule : public FeatureModule {
public:
  MonitorModule();
  virtual ~MonitorModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Monitors"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

  struct MonitorInfo {
    LVKW_MonitorRef* ref;
    std::string name;
    LVKW_LogicalVec position;
    LVKW_LogicalVec size;
    LVKW_PixelVec physical_size;
    LVKW_Scalar scale;
    std::vector<LVKW_VideoMode> modes;
  };

  std::vector<MonitorInfo> monitors_;
  bool needs_refresh_ = true;
  void refreshMonitors(lvkw::Context &ctx);
};
