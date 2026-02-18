#include "metrics_module.hpp"
#include "imgui.h"
#include "lvkw/c/context.h"
#include "lvkw/c/instrumentation.h"
#include <mutex>
#include <thread>
#include <chrono>

std::deque<MetricsModule::MemOp> MetricsModule::mem_ops_;
std::mutex MetricsModule::mem_ops_mutex_;
std::deque<std::string> MetricsModule::diag_logs_;
std::mutex MetricsModule::diag_mutex_;

void MetricsModule::addMemOp(MetricsModule::MemOp op) {
    std::lock_guard<std::mutex> lock(mem_ops_mutex_);
    op.time = ImGui::GetTime();
    mem_ops_.push_back(op);
    if (mem_ops_.size() > 1000) {
        mem_ops_.pop_front();
    }
}

void* MetricsModule::logging_alloc(size_t size, void* userdata) {
    void* ptr = malloc(size);
    addMemOp({MemOp::ALLOC, ptr, size, nullptr, 0.0});
    return ptr;
}

void* MetricsModule::logging_realloc(void* ptr, size_t size, void* userdata) {
    void* new_ptr = realloc(ptr, size);
    addMemOp({MemOp::REALLOC, new_ptr, size, ptr, 0.0});
    return new_ptr;
}

void MetricsModule::logging_free(void* ptr, void* userdata) {
    addMemOp({MemOp::FREE, ptr, 0, nullptr, 0.0});
    free(ptr);
}

void MetricsModule::diagnostic_callback(const LVKW_DiagnosticInfo* info, void* userdata) {
    std::lock_guard<std::mutex> lock(diag_mutex_);
    diag_logs_.push_back(std::string(info->message));
    if (diag_logs_.size() > 500) {
        diag_logs_.pop_front();
    }
}

MetricsModule::MetricsModule() {
    LVKW_ContextCreateInfo cci = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
    attributes_ = cci.attributes;
    tuning_ = LVKW_CONTEXT_TUNING_DEFAULT;
}

void MetricsModule::update(lvkw::Context &ctx, lvkw::Window &window) {
    if (!enabled_) return;

    if (lag_ms_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(lag_ms_));
    }
}

void MetricsModule::render(lvkw::Context &ctx, lvkw::Window &window) {
    if (!enabled_) return;

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Metrics & Diagnostics", &enabled_)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("RECREATE PRIMARY CONTEXT")) {
        recreation_requested_ = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(Will happen on next frame)");

    ImGui::Separator();

    if (ImGui::BeginTabBar("MetricsTabs")) {
        if (ImGui::BeginTabItem("Control")) {
            renderMetrics(ctx);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Create Info")) {
            renderCreateInfo();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tuning")) {
            renderTuning();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Memory Log")) {
            renderMemLog();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Diagnostics Log")) {
            renderDiagLog();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void MetricsModule::renderMetrics(lvkw::Context &ctx) {
    ImGui::Text("Event Control & Performance Simulation:");
    
    ImGui::Separator();
    ImGui::Text("Lag simulator (ms)");
    ImGui::SliderInt("##Lag", &lag_ms_, 0, 500);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Simulate main-thread lag (ms) to see how push-based events behave.");
    }

    ImGui::Separator();
    ImGui::Text("Sync Timeout (s)");
    int timeout_int = (int)sync_timeout_;
    if (ImGui::InputInt("##SyncTimeout", &timeout_int)) {
        if (timeout_int < 0) timeout_int = 0;
        sync_timeout_ = (uint32_t)timeout_int;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("The app will stop updating until the timeout or some event happens");
    
    if (ImGui::Button("Sync Once")) {
        sync_request_time_ = ImGui::GetTime() + 1.0;
    }

    if (sync_request_time_ > 0.0) {
        double remaining = sync_request_time_ - ImGui::GetTime();
        if (remaining > 0)
            ImGui::Text("Sync in %.1fs...", remaining);
        else
            ImGui::Text("Waiting...");
    }

    ImGui::Separator();
    if (ImGui::Button("Post 5000 USER_0 events")) {
        postStressEvents(ctx);
    }
}

bool MetricsModule::consumeRecreationRequest(ContextRecreateInfo& out_info) {
    if (recreation_requested_) {
        recreation_requested_ = false;
        out_info.backend = selected_backend_;
        out_info.flags = creation_flags_;
        out_info.attributes = attributes_;
        out_info.tuning = tuning_;
        out_info.use_logging_allocator = use_logging_allocator_;
        return true;
    }
    return false;
}

void MetricsModule::renderCreateInfo() {
    ImGui::Text("Backend:");
    const char* backends[] = { "AUTO", "WAYLAND", "X11", "WIN32", "COCOA" };
    int current_backend = (int)selected_backend_;
    if (ImGui::Combo("##Backend", &current_backend, backends, IM_ARRAYSIZE(backends))) {
        selected_backend_ = (LVKW_BackendType)current_backend;
    }

    ImGui::Separator();
    ImGui::Text("Initial Attributes:");

    ImGui::Checkbox("Inhibit Idle", &attributes_.inhibit_idle);

    ImGui::Separator();
    ImGui::Text("Allocator:");
    ImGui::Checkbox("Use Logging Allocator", &use_logging_allocator_);
}

void MetricsModule::renderTuning() {
    if (ImGui::TreeNodeEx("Wayland", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* decor_modes[] = { "AUTO", "SSD", "CSD", "NONE" };
        int current_mode = (int)tuning_.wayland.decoration_mode;
        if (ImGui::Combo("Decoration Mode", &current_mode, decor_modes, IM_ARRAYSIZE(decor_modes))) {
            tuning_.wayland.decoration_mode = (LVKW_WaylandDecorationMode)current_mode;
        }
        ImGui::TreePop();
    }
}

void MetricsModule::renderMemLog() {
    if (ImGui::Button("Clear Log##Mem")) {
        std::lock_guard<std::mutex> lock(mem_ops_mutex_);
        mem_ops_.clear();
    }
    
    ImGui::BeginChild("MemLogScroll");
    std::lock_guard<std::mutex> lock(mem_ops_mutex_);
    for (const auto& op : mem_ops_) {
        switch (op.type) {
            case MemOp::ALLOC:
                ImGui::Text("[%06.3f] ALLOC: %zu bytes -> %p", op.time, op.size, op.ptr);
                break;
            case MemOp::REALLOC:
                ImGui::Text("[%06.3f] REALLOC: %p -> %p (%zu bytes)", op.time, op.old_ptr, op.ptr, op.size);
                break;
            case MemOp::FREE:
                ImGui::Text("[%06.3f] FREE: %p", op.time, op.ptr);
                break;
        }
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

void MetricsModule::renderDiagLog() {
    if (ImGui::Button("Clear Log##Diag")) {
        std::lock_guard<std::mutex> lock(diag_mutex_);
        diag_logs_.clear();
    }
    
    ImGui::BeginChild("DiagLogScroll");
    std::lock_guard<std::mutex> lock(diag_mutex_);
    for (const auto& msg : diag_logs_) {
        ImGui::TextUnformatted(msg.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

void MetricsModule::postStressEvents(lvkw::Context &ctx) {
    for (int i = 0; i < 5000; i++) {
        lvkw::postEvent(ctx, LVKW_EVENT_TYPE_USER_0);
    }
}

uint32_t MetricsModule::getRequestedSyncTimeout() {
  if (sync_request_time_ > 0.0 && ImGui::GetTime() >= sync_request_time_) {
    sync_request_time_ = -1.0;
    return sync_timeout_ * 1000;
  }
  return 0;
}
