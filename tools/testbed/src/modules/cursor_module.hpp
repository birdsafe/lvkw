#pragma once

#include "../feature_module.hpp"
#include <vector>
#include <memory>

class CursorModule : public FeatureModule {
public:
  CursorModule();
  virtual ~CursorModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Cursors"; }
  bool &getEnabled() override { return enabled_; }

  LVKW_CursorMode getRequestedMode() const { return enabled_ ? current_mode_ : LVKW_CURSOR_NORMAL; }

private:
  bool enabled_ = false;

  LVKW_CursorMode current_mode_ = LVKW_CURSOR_NORMAL;
  int selected_shape_idx_ = 0;
  
  std::unique_ptr<lvkw::Cursor> custom_cursor_;

  void createCustomCursor(lvkw::Context &ctx);
};
