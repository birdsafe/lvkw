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
  void update(lvkw::Context &ctx, lvkw::Window &window, LVKW_EventType mask);
  void render(lvkw::Context &ctx, lvkw::Window &window) override;
  void onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) override;

  void onFrameBegin();
  void registerWindowTitle(LVKW_Window* handle, const std::string& title);

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

  // Filters
  uint32_t filter_mask_ = LVKW_EVENT_TYPE_ALL;

  void log(LVKW_EventType type, LVKW_Window* window, const std::string& details);
  const char* typeToString(LVKW_EventType type);
};
