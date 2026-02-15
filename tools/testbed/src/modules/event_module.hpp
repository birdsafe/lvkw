#pragma once

#include "../feature_module.hpp"
#include <deque>
#include <map>
#include <string>

class EventModule : public FeatureModule {
public:
  EventModule();
  virtual ~EventModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Event System"; }
  bool &getEnabled() override { return enabled_; }
  uint32_t getRequestedSyncTimeout() override;

  LVKW_EventType getBaseMask() const { return static_cast<LVKW_EventType>(base_mask_); }

private:
  bool enabled_ = true;
  uint32_t sync_timeout_ = 0;
  double sync_request_time_ = -1.0;

  // Event Mask Control
  std::map<uint32_t, bool> event_toggles_;
  bool strict_filtering_ = false;
  uint32_t base_mask_ = 0;
  void updateEventMask(lvkw::Context &ctx);

  // Animation
  void renderAnimation();
  float anim_x_ = 0.0f, anim_y_ = 0.0f;
  float anim_dx_ = 2.0f, anim_dy_ = 2.0f;

  // Drag and Drop
  LVKW_DndAction dnd_action_target_ = LVKW_DND_ACTION_COPY;
};
