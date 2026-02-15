#pragma once

#include "../feature_module.hpp"
#include <string>
#include <vector>

class DndModule : public FeatureModule {
public:
  DndModule();
  virtual ~DndModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Drag & Drop"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

  struct DropSession {
    LVKW_LogicalVec position;
    std::vector<std::string> paths;
    LVKW_DndAction action;
    bool is_active = false;
    bool dropped = false;
  } current_session_;

  LVKW_DndAction selected_feedback_action_ = LVKW_DND_ACTION_COPY;
  
  std::vector<std::string> last_dropped_paths_;
};
