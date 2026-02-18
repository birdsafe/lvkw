#pragma once

#include "../feature_module.hpp"
#include "lvkw/lvkw.hpp"
#include <memory>
#include <string>
#include <vector>

class CursorModule : public FeatureModule {
public:
  CursorModule();
  virtual ~CursorModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;
  void onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) override;

  void onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& event);

  const char *getName() const override { return "Cursors"; }
  bool &getEnabled() override { return enabled_; }

  LVKW_CursorMode getRequestedMode() const { return current_mode_; }
  bool hasActiveCursorOverride() const { return cursor_override_active_; }

private:
  bool enabled_ = false;
  LVKW_CursorMode current_mode_ = LVKW_CURSOR_NORMAL;
  int selected_shape_idx_ = 0;
  bool cursor_override_active_ = false;

  struct PendingCursor {
    LVKW_Cursor *cursor = nullptr;
    LVKW_CursorMode mode = LVKW_CURSOR_NORMAL;
    bool cursor_changed = false;
    bool mode_changed = false;
  } pending_cursor_;

  std::unique_ptr<lvkw::Cursor> custom_cursor_;
  void createCustomCursor(lvkw::Context &ctx);
};
