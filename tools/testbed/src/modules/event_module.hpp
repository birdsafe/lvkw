#pragma once

#include "../feature_module.hpp"
#include "lvkw/lvkw.hpp"
#include <map>
#include <vector>

class EventModule : public FeatureModule {
public:
  EventModule();
  virtual ~EventModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  void onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& event);

  const char *getName() const override { return "Event System"; }
  bool &getEnabled() override { return enabled_; }

  uint32_t getBaseMask() const { return base_mask_; }
  uint32_t getRequestedSyncTimeout() override;

private:
  bool enabled_ = true;
  uint32_t base_mask_ = 0;
  bool strict_filtering_ = false;

  std::map<LVKW_EventType, bool> event_toggles_;

  // Animation state
  float anim_x_ = 0;
  float anim_y_ = 0;
  float anim_dx_ = 2;
  float anim_dy_ = 2;

  // Sync state
  uint32_t sync_timeout_ = 5;
  double sync_request_time_ = -1.0;

  void updateEventMask(lvkw::Context &ctx);
  void renderAnimation();
};
