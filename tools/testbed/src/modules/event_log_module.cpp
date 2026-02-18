#include "event_log_module.hpp"
#include "imgui.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <map>

EventLogModule::EventLogModule() {
    logs_.resize(max_logs_);
}

void EventLogModule::onFrameBegin() {
    current_frame_id_++;
}

void EventLogModule::registerWindowTitle(LVKW_Window* handle, const std::string& title) {
    window_titles_[handle] = title;
}

static std::string formatModifiers(LVKW_ModifierFlags mods) {
    std::string s;
    if (mods & LVKW_MODIFIER_SHIFT) s += "Shift ";
    if (mods & LVKW_MODIFIER_CONTROL) s += "Ctrl ";
    if (mods & LVKW_MODIFIER_ALT) s += "Alt ";
    if (mods & LVKW_MODIFIER_META) s += "Super ";
    if (mods & LVKW_MODIFIER_CAPS_LOCK) s += "Caps ";
    if (mods & LVKW_MODIFIER_NUM_LOCK) s += "Num ";
    return s;
}

void EventLogModule::onEvent(LVKW_EventType type, LVKW_Window* win, const LVKW_Event& e, uint32_t mask) {
    if (!enabled_) return;
    if (!(type & mask)) return;

    std::stringstream ss;
    switch (type) {
        case LVKW_EVENT_TYPE_WINDOW_RESIZED:
            ss << "Size: " << e.resized.geometry.logical_size.x << "x" << e.resized.geometry.logical_size.y 
                << " (Pix: " << e.resized.geometry.pixel_size.x << "x" << e.resized.geometry.pixel_size.y << ")";
            break;
        case LVKW_EVENT_TYPE_KEY:
            ss << "Key: " << (int)e.key.key << " State: " << (e.key.state == LVKW_BUTTON_STATE_PRESSED ? "PR" : "RE")
                << " Mods: " << formatModifiers(e.key.modifiers);
            break;
        case LVKW_EVENT_TYPE_MOUSE_MOTION:
            ss << "Pos: (" << e.mouse_motion.position.x << "," << e.mouse_motion.position.y << ")"
                << " Delta: (" << e.mouse_motion.delta.x << "," << e.mouse_motion.delta.y << ")"
                << " Raw: (" << e.mouse_motion.raw_delta.x << "," << e.mouse_motion.raw_delta.y << ")";
            break;
        case LVKW_EVENT_TYPE_MOUSE_BUTTON:
            ss << "Button: " << (int)e.mouse_button.button << " State: " << (e.mouse_button.state == LVKW_BUTTON_STATE_PRESSED ? "PR" : "RE")
                << " Mods: " << formatModifiers(e.mouse_button.modifiers);
            break;
        case LVKW_EVENT_TYPE_MOUSE_SCROLL:
            ss << "Delta: (" << e.mouse_scroll.delta.x << "," << e.mouse_scroll.delta.y << ")";
            break;
        case LVKW_EVENT_TYPE_TEXT_INPUT:
            ss << "Text: '" << (e.text_input.text ? e.text_input.text : "") << "' Len: " << e.text_input.length;
            break;
        case LVKW_EVENT_TYPE_TEXT_COMPOSITION:
            ss << "Preedit: '" << (e.text_composition.text ? e.text_composition.text : "") << "' Cursor: " << e.text_composition.cursor_index;
            break;
        case LVKW_EVENT_TYPE_FOCUS:
            ss << "Focused: " << (e.focus.focused ? "YES" : "NO");
            break;
        case LVKW_EVENT_TYPE_WINDOW_MAXIMIZED:
            ss << "Maximized: " << (e.maximized.maximized ? "YES" : "NO");
            break;
        case LVKW_EVENT_TYPE_MONITOR_CONNECTION:
            ss << "Monitor: " << (void*)e.monitor_connection.monitor_ref << " Connected: " << (e.monitor_connection.connected ? "YES" : "NO");
            break;
        case LVKW_EVENT_TYPE_DND_HOVER:
            ss << "Pos: (" << e.dnd_hover.position.x << "," << e.dnd_hover.position.y << ") Paths: " << e.dnd_hover.path_count;
            break;
        case LVKW_EVENT_TYPE_DND_DROP:
            ss << "Pos: (" << e.dnd_drop.position.x << "," << e.dnd_drop.position.y << ") Paths: " << e.dnd_drop.path_count;
            break;
#ifdef LVKW_ENABLE_CONTROLLER
        case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION:
            ss << "Controller: " << (void*)e.controller_connection.controller_ref
                << " Connected: " << (e.controller_connection.connected ? "YES" : "NO");
            break;
#endif
        default:
            ss << "No detailed payload parser";
            break;
    }

    LogEntry entry;
    entry.timestamp = ImGui::GetTime();
    entry.frame_id = current_frame_id_;
    entry.type = type;
    entry.details = ss.str();
    entry.window_handle = win;
    
    if (win) {
        auto it = window_titles_.find(win);
        if (it != window_titles_.end()) {
            entry.window_title = it->second;
        } else {
            char buf[32];
            snprintf(buf, sizeof(buf), "Wnd:%p", (void*)win);
            entry.window_title = buf;
        }
    } else {
        entry.window_title = "Global";
    }

    logs_[next_index_] = std::move(entry);
    next_index_ = (next_index_ + 1) % max_logs_;
    if (next_index_ == 0) full_ = true;
}

void EventLogModule::render(lvkw::Context &ctx, lvkw::Window &window) {
    if (!enabled_) return;

    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Event Log", &enabled_)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) {
        logs_.clear();
        logs_.resize(max_logs_);
        next_index_ = 0;
        full_ = false;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);
    ImGui::SameLine();
    ImGui::Checkbox("Details", &show_details_);

    ImGui::Separator();

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("EventLogTable", show_details_ ? 5 : 4, flags, ImVec2(0, 0))) {
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Frame", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Window", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        if (show_details_) ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch);
        
        ImGui::TableSetupScrollFreeze(0, 1); // Freeze the header row
        ImGui::TableHeadersRow();

        size_t count = full_ ? max_logs_ : next_index_;
        size_t start = full_ ? next_index_ : 0;

        for (size_t i = 0; i < count; ++i) {
            const auto& entry = logs_[(start + i) % max_logs_];
            ImGui::TableNextRow();

            // Alternating colors based on frame ID
            ImU32 row_bg_color = (entry.frame_id % 2 == 0) ? IM_COL32(50, 50, 60, 255) : IM_COL32(40, 40, 50, 255);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, row_bg_color);

            ImGui::TableNextColumn();
            ImGui::Text("%.3f", entry.timestamp);
            ImGui::TableNextColumn();
            ImGui::Text("0x%06X", entry.frame_id);
            ImGui::TableNextColumn();
            ImGui::Text("%s", typeToString(entry.type));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(entry.window_title.c_str());
            if (show_details_) {
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(entry.details.c_str());
            }
        }

        if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndTable();
    }

    ImGui::End();
}

void EventLogModule::onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) {
    (void)ctx;
    (void)window;
    logs_.clear();
    logs_.resize(max_logs_);
    next_index_ = 0;
    full_ = false;
    window_titles_.clear();
}

const char* EventLogModule::typeToString(LVKW_EventType type) {
    switch (type) {
        case LVKW_EVENT_TYPE_CLOSE_REQUESTED: return "CLOSE_REQUESTED";
        case LVKW_EVENT_TYPE_WINDOW_RESIZED: return "WINDOW_RESIZED";
        case LVKW_EVENT_TYPE_KEY: return "KEY";
        case LVKW_EVENT_TYPE_WINDOW_READY: return "WINDOW_READY";
        case LVKW_EVENT_TYPE_MOUSE_MOTION: return "MOUSE_MOTION";
        case LVKW_EVENT_TYPE_MOUSE_BUTTON: return "MOUSE_BUTTON";
        case LVKW_EVENT_TYPE_MOUSE_SCROLL: return "MOUSE_SCROLL";
        case LVKW_EVENT_TYPE_IDLE_STATE_CHANGED: return "IDLE_STATE_CHANGED";
        case LVKW_EVENT_TYPE_MONITOR_CONNECTION: return "MONITOR_CONNECTION";
        case LVKW_EVENT_TYPE_MONITOR_MODE: return "MONITOR_MODE";
        case LVKW_EVENT_TYPE_TEXT_INPUT: return "TEXT_INPUT";
        case LVKW_EVENT_TYPE_FOCUS: return "FOCUS";
        case LVKW_EVENT_TYPE_WINDOW_MAXIMIZED: return "WINDOW_MAXIMIZED";
        case LVKW_EVENT_TYPE_DND_HOVER: return "DND_HOVER";
        case LVKW_EVENT_TYPE_DND_LEAVE: return "DND_LEAVE";
        case LVKW_EVENT_TYPE_DND_DROP: return "DND_DROP";
        case LVKW_EVENT_TYPE_TEXT_COMPOSITION: return "TEXT_COMPOSITION";
#ifdef LVKW_ENABLE_CONTROLLER
        case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION: return "CONTROLLER_CONNECTION";
#endif
        case LVKW_EVENT_TYPE_USER_0: return "USER_0";
        case LVKW_EVENT_TYPE_USER_1: return "USER_1";
        case LVKW_EVENT_TYPE_USER_2: return "USER_2";
        case LVKW_EVENT_TYPE_USER_3: return "USER_3";
        default: return "UNKNOWN";
    }
}
