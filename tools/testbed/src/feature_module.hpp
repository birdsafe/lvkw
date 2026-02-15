#pragma once

#include "lvkw/lvkw.hpp"

class FeatureModule {
public:
  virtual ~FeatureModule() = default;

  // Called every frame. Handle logic here.
  virtual void update(lvkw::Context &ctx, lvkw::Window &window) = 0;

  // Called every frame. Draw specific ImGui window(s) here.
  virtual void render(lvkw::Context &ctx, lvkw::Window &window) = 0;

  // For the Table of Contents
  virtual const char *getName() const = 0;
  virtual bool &getEnabled() = 0;

  // Sync Timeout Control
  virtual uint32_t getRequestedSyncTimeout() { return 0; }
};
