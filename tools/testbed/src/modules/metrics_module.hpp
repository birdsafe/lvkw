#pragma once

#include "../feature_module.hpp"
#include <vector>
#include <string>
#include <deque>
#include <mutex>

struct ContextRecreateInfo {
    LVKW_BackendType backend;
    uint32_t flags;
    LVKW_ContextAttributes attributes;
    LVKW_ContextTuning tuning;
    bool use_logging_allocator;
};

class MetricsModule : public FeatureModule {
public:
  MetricsModule();
  virtual ~MetricsModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Metrics & Diag"; }
  bool &getEnabled() override { return enabled_; }
  uint32_t getRequestedSyncTimeout() override;

  bool consumeRecreationRequest(ContextRecreateInfo& out_info);

  static void diagnostic_callback(const LVKW_DiagnosticInfo* info, void* userdata);
  static void* logging_alloc(size_t size, void* userdata);
  static void* logging_realloc(void* ptr, size_t size, void* userdata);
  static void logging_free(void* ptr, void* userdata);

  struct MemOp {
      enum Type { ALLOC, REALLOC, FREE } type;
      void* ptr;
      size_t size;
      void* old_ptr;
      double time;
  };

private:
  bool enabled_ = false;

  // Metrics state
  int lag_ms_ = 0;
  uint32_t sync_timeout_ = 0;
  double sync_request_time_ = -1.0;

  // Creation Parameters (UI state)
  LVKW_BackendType selected_backend_ = LVKW_BACKEND_AUTO;
  uint32_t creation_flags_ = LVKW_CONTEXT_FLAG_NONE;
  LVKW_ContextAttributes attributes_ = {};
  LVKW_ContextTuning tuning_ = LVKW_CONTEXT_TUNING_DEFAULT;
  bool use_logging_allocator_ = false;

  bool recreation_requested_ = false;

  // Memory Operation Log
  static std::deque<MemOp> mem_ops_;
  static std::mutex mem_ops_mutex_;
  static void addMemOp(MemOp op);

  // Diagnostics Log
  static std::deque<std::string> diag_logs_;
  static std::mutex diag_mutex_;

  // UI helpers
  void renderMetrics(lvkw::Context &ctx);
  void renderCreateInfo();
  void renderTuning();
  void renderMemLog();
  void renderDiagLog();

  void postStressEvents(lvkw::Context &ctx);
};
