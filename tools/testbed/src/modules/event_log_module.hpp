#pragma once

#include "../feature_module.hpp"
#include <deque>
#include <map>
#include <string>
#include <vector>

class EventLogModule : public FeatureModule {
public:
  EventLogModule();
  virtual ~EventLogModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override { (void)ctx; (void)window; }
  void render(lvkw::Context &ctx, lvkw::Window &window) override;
  void onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) override;

  void onFrameBegin();
  void registerWindowTitle(LVKW_Window* handle, const std::string& title);

  void onEvent(LVKW_EventType type, LVKW_Window* win, const LVKW_Event& e, uint32_t mask);

  const char *getName() const override { return "Event Log"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;

  struct LogEntry {
    double timestamp;
    uint32_t frame_id;
    LVKW_EventType type;
    std::string details;
    std::string window_title;
    LVKW_Window* window_handle;
  };

  std::vector<LogEntry> logs_;
  size_t max_logs_ = 2000;
  size_t next_index_ = 0;
  uint32_t current_frame_id_ = 0;
  bool full_ = false;

  std::map<LVKW_Window*, std::string> window_titles_;

  bool auto_scroll_ = true;
  bool show_details_ = true;

  const char* typeToString(LVKW_EventType type);
};
