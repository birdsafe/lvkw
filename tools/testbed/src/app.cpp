#include "app.hpp"
#include "modules/metrics_module.hpp"
#include <cstdio>

App::App(lvkw::Window &window, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, uint32_t queue_family, VkQueue queue, VkDescriptorPool descriptor_pool) {
  primary_window_handle_ = window.get();
  event_module_ = std::make_unique<EventModule>();
  metrics_module_ = std::make_unique<MetricsModule>();
  controller_module_ = std::make_unique<ControllerModule>();
  monitor_module_ = std::make_unique<MonitorModule>();
  cursor_module_ = std::make_unique<CursorModule>();
  input_module_ = std::make_unique<InputModule>();
  event_log_module_ = std::make_unique<EventLogModule>();
  clipboard_module_ = std::make_unique<ClipboardModule>();
  dnd_module_ = std::make_unique<DndModule>();
  window_module_ = std::make_unique<WindowModule>(window, instance, physical_device, device, queue_family, queue, descriptor_pool);
}

static ImGuiKey mapLVKWKeyToImGuiKey(LVKW_Key key) {
    switch (key) {
        case LVKW_KEY_TAB: return ImGuiKey_Tab;
        case LVKW_KEY_LEFT: return ImGuiKey_LeftArrow;
        case LVKW_KEY_RIGHT: return ImGuiKey_RightArrow;
        case LVKW_KEY_UP: return ImGuiKey_UpArrow;
        case LVKW_KEY_DOWN: return ImGuiKey_DownArrow;
        case LVKW_KEY_PAGE_UP: return ImGuiKey_PageUp;
        case LVKW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case LVKW_KEY_HOME: return ImGuiKey_Home;
        case LVKW_KEY_END: return ImGuiKey_End;
        case LVKW_KEY_INSERT: return ImGuiKey_Insert;
        case LVKW_KEY_DELETE: return ImGuiKey_Delete;
        case LVKW_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case LVKW_KEY_SPACE: return ImGuiKey_Space;
        case LVKW_KEY_ENTER: return ImGuiKey_Enter;
        case LVKW_KEY_ESCAPE: return ImGuiKey_Escape;
        case LVKW_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
        case LVKW_KEY_COMMA: return ImGuiKey_Comma;
        case LVKW_KEY_MINUS: return ImGuiKey_Minus;
        case LVKW_KEY_PERIOD: return ImGuiKey_Period;
        case LVKW_KEY_SLASH: return ImGuiKey_Slash;
        case LVKW_KEY_SEMICOLON: return ImGuiKey_Semicolon;
        case LVKW_KEY_EQUAL: return ImGuiKey_Equal;
        case LVKW_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case LVKW_KEY_BACKSLASH: return ImGuiKey_Backslash;
        case LVKW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case LVKW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
        case LVKW_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
        case LVKW_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case LVKW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
        case LVKW_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case LVKW_KEY_PAUSE: return ImGuiKey_Pause;
        case LVKW_KEY_KP_0: return ImGuiKey_Keypad0;
        case LVKW_KEY_KP_1: return ImGuiKey_Keypad1;
        case LVKW_KEY_KP_2: return ImGuiKey_Keypad2;
        case LVKW_KEY_KP_3: return ImGuiKey_Keypad3;
        case LVKW_KEY_KP_4: return ImGuiKey_Keypad4;
        case LVKW_KEY_KP_5: return ImGuiKey_Keypad5;
        case LVKW_KEY_KP_6: return ImGuiKey_Keypad6;
        case LVKW_KEY_KP_7: return ImGuiKey_Keypad7;
        case LVKW_KEY_KP_8: return ImGuiKey_Keypad8;
        case LVKW_KEY_KP_9: return ImGuiKey_Keypad9;
        case LVKW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        case LVKW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case LVKW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case LVKW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case LVKW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
        case LVKW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
        case LVKW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
        case LVKW_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
        case LVKW_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case LVKW_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
        case LVKW_KEY_LEFT_META: return ImGuiKey_LeftSuper;
        case LVKW_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
        case LVKW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case LVKW_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
        case LVKW_KEY_RIGHT_META: return ImGuiKey_RightSuper;
        case LVKW_KEY_MENU: return ImGuiKey_Menu;
        case LVKW_KEY_0: return ImGuiKey_0;
        case LVKW_KEY_1: return ImGuiKey_1;
        case LVKW_KEY_2: return ImGuiKey_2;
        case LVKW_KEY_3: return ImGuiKey_3;
        case LVKW_KEY_4: return ImGuiKey_4;
        case LVKW_KEY_5: return ImGuiKey_5;
        case LVKW_KEY_6: return ImGuiKey_6;
        case LVKW_KEY_7: return ImGuiKey_7;
        case LVKW_KEY_8: return ImGuiKey_8;
        case LVKW_KEY_9: return ImGuiKey_9;
        case LVKW_KEY_A: return ImGuiKey_A;
        case LVKW_KEY_B: return ImGuiKey_B;
        case LVKW_KEY_C: return ImGuiKey_C;
        case LVKW_KEY_D: return ImGuiKey_D;
        case LVKW_KEY_E: return ImGuiKey_E;
        case LVKW_KEY_F: return ImGuiKey_F;
        case LVKW_KEY_G: return ImGuiKey_G;
        case LVKW_KEY_H: return ImGuiKey_H;
        case LVKW_KEY_I: return ImGuiKey_I;
        case LVKW_KEY_J: return ImGuiKey_J;
        case LVKW_KEY_K: return ImGuiKey_K;
        case LVKW_KEY_L: return ImGuiKey_L;
        case LVKW_KEY_M: return ImGuiKey_M;
        case LVKW_KEY_N: return ImGuiKey_N;
        case LVKW_KEY_O: return ImGuiKey_O;
        case LVKW_KEY_P: return ImGuiKey_P;
        case LVKW_KEY_Q: return ImGuiKey_Q;
        case LVKW_KEY_R: return ImGuiKey_R;
        case LVKW_KEY_S: return ImGuiKey_S;
        case LVKW_KEY_T: return ImGuiKey_T;
        case LVKW_KEY_U: return ImGuiKey_U;
        case LVKW_KEY_V: return ImGuiKey_V;
        case LVKW_KEY_W: return ImGuiKey_W;
        case LVKW_KEY_X: return ImGuiKey_X;
        case LVKW_KEY_Y: return ImGuiKey_Y;
        case LVKW_KEY_Z: return ImGuiKey_Z;
        case LVKW_KEY_F1: return ImGuiKey_F1;
        case LVKW_KEY_F2: return ImGuiKey_F2;
        case LVKW_KEY_F3: return ImGuiKey_F3;
        case LVKW_KEY_F4: return ImGuiKey_F4;
        case LVKW_KEY_F5: return ImGuiKey_F5;
        case LVKW_KEY_F6: return ImGuiKey_F6;
        case LVKW_KEY_F7: return ImGuiKey_F7;
        case LVKW_KEY_F8: return ImGuiKey_F8;
        case LVKW_KEY_F9: return ImGuiKey_F9;
        case LVKW_KEY_F10: return ImGuiKey_F10;
        case LVKW_KEY_F11: return ImGuiKey_F11;
        case LVKW_KEY_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}

void App::onEvent(LVKW_EventType type, LVKW_Window* window, const LVKW_Event& event) {
    ImGuiIO &io = ImGui::GetIO();

    if (type == LVKW_EVENT_TYPE_WINDOW_READY) {
        waiting_for_ready_ = false;
    }

    switch (type) {
      case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
        if (window == primary_window_handle_) {
            exit_requested_ = true;
        }
        break;

      case LVKW_EVENT_TYPE_MOUSE_MOTION:
        io.AddMousePosEvent(static_cast<float>(event.mouse_motion.position.x),
                            static_cast<float>(event.mouse_motion.position.y));
        break;
      
      case LVKW_EVENT_TYPE_MOUSE_BUTTON: {
        int button = -1;
        switch (event.mouse_button.button) {
          case LVKW_MOUSE_BUTTON_LEFT: button = 0; break;
          case LVKW_MOUSE_BUTTON_RIGHT: button = 1; break;
          case LVKW_MOUSE_BUTTON_MIDDLE: button = 2; break;
          default: break;
        }
        if (button != -1) {
          io.AddMouseButtonEvent(button, event.mouse_button.state == LVKW_BUTTON_STATE_PRESSED);
        }
        break;
      }

      case LVKW_EVENT_TYPE_MOUSE_SCROLL:
        io.AddMouseWheelEvent(0.0f, static_cast<float>(event.mouse_scroll.delta.y));
        break;

      case LVKW_EVENT_TYPE_WINDOW_RESIZED:
        io.DisplaySize = ImVec2(static_cast<float>(event.resized.geometry.pixel_size.x),
                                static_cast<float>(event.resized.geometry.pixel_size.y));
        break;

      case LVKW_EVENT_TYPE_KEY: {
        ImGuiKey imgui_key = mapLVKWKeyToImGuiKey(event.key.key);
        if (imgui_key != ImGuiKey_None) {
          io.AddKeyEvent(imgui_key, event.key.state == LVKW_BUTTON_STATE_PRESSED);
        }
        io.AddKeyEvent(ImGuiMod_Ctrl, (event.key.modifiers & LVKW_MODIFIER_CONTROL) != 0);
        io.AddKeyEvent(ImGuiMod_Shift, (event.key.modifiers & LVKW_MODIFIER_SHIFT) != 0);
        io.AddKeyEvent(ImGuiMod_Alt, (event.key.modifiers & LVKW_MODIFIER_ALT) != 0);
        io.AddKeyEvent(ImGuiMod_Super, (event.key.modifiers & LVKW_MODIFIER_META) != 0);
        break;
      }

      case LVKW_EVENT_TYPE_TEXT_INPUT:
        if(io.WantCaptureKeyboard && event.text_input.text)
          io.AddInputCharactersUTF8(event.text_input.text);
        break;

      default:
        break;
    }

    // Delegate to modules
    controller_module_->onEvent(type, window, event);
    monitor_module_->onEvent(type, window, event);
    cursor_module_->onEvent(type, window, event);
    input_module_->onEvent(type, window, event);
    clipboard_module_->onEvent(type, window, event);
    dnd_module_->onEvent(type, window, event);
    window_module_->onEvent(type, window, event);
    event_log_module_->onEvent(type, window, event, event_module_->getBaseMask());
}

AppStatus App::update(lvkw::Context &ctx, lvkw::Window &window, ImGuiIO &io) {
  event_log_module_->onFrameBegin();
  event_log_module_->registerWindowTitle(window.get(), "LVKW Testbed");

  uint32_t timeout_ms = 0;
  if (metrics_module_->getEnabled()) {
      uint32_t t = metrics_module_->getRequestedSyncTimeout();
      if (t > timeout_ms) timeout_ms = t;
  }

  lvkw_events_pump(ctx.get(), timeout_ms);

  // Check for exit
  if (exit_requested_ || (window.get()->flags & LVKW_WINDOW_STATE_LOST)) return AppStatus::EXIT;

  // Always update, because some listeners might want to track things in the background
  event_module_->update(ctx, window);
  metrics_module_->update(ctx, window);
  controller_module_->update(ctx, window);
  monitor_module_->update(ctx, window);
  cursor_module_->update(ctx, window);
  input_module_->update(ctx, window);
  event_log_module_->update(ctx, window);
  clipboard_module_->update(ctx, window);
  dnd_module_->update(ctx, window);
  window_module_->update(ctx, window);

  updateCursor(ctx, window, io);
  
  if (metrics_module_->consumeRecreationRequest(recreate_info_)) {
      return AppStatus::RECREATE_CONTEXT;
  }

  return AppStatus::KEEP_GOING;
}

void App::renderUi(lvkw::Context &ctx, lvkw::Window &window, const ImGuiIO &io) {
  {
    ImGui::Begin("LVKW TestBed");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    
    ImGui::Separator();
    ImGui::Text("Modules");
    
    ImGui::Checkbox(event_module_->getName(), &event_module_->getEnabled());
    ImGui::Checkbox(metrics_module_->getName(), &metrics_module_->getEnabled());
    ImGui::Checkbox(controller_module_->getName(), &controller_module_->getEnabled());
    ImGui::Checkbox(monitor_module_->getName(), &monitor_module_->getEnabled());
    ImGui::Checkbox(cursor_module_->getName(), &cursor_module_->getEnabled());
    ImGui::Checkbox(input_module_->getName(), &input_module_->getEnabled());
    ImGui::Checkbox(event_log_module_->getName(), &event_log_module_->getEnabled());
    ImGui::Checkbox(clipboard_module_->getName(), &clipboard_module_->getEnabled());
    ImGui::Checkbox(dnd_module_->getName(), &dnd_module_->getEnabled());
    ImGui::Checkbox(window_module_->getName(), &window_module_->getEnabled());

    ImGui::End();
  }

  if (event_module_->getEnabled()) event_module_->render(ctx, window);
  if (metrics_module_->getEnabled()) metrics_module_->render(ctx, window);
  if (controller_module_->getEnabled()) controller_module_->render(ctx, window);
  if (monitor_module_->getEnabled()) monitor_module_->render(ctx, window);
  if (cursor_module_->getEnabled()) cursor_module_->render(ctx, window);
  if (input_module_->getEnabled()) input_module_->render(ctx, window);
  if (event_log_module_->getEnabled()) event_log_module_->render(ctx, window);
  if (clipboard_module_->getEnabled()) clipboard_module_->render(ctx, window);
  if (dnd_module_->getEnabled()) dnd_module_->render(ctx, window);
  if (window_module_->getEnabled()) window_module_->render(ctx, window);
}

void App::onContextRecreated(lvkw::Context &ctx, lvkw::Window &window) {
  primary_window_handle_ = window.get();
  event_module_->onContextRecreated(ctx, window);
  metrics_module_->onContextRecreated(ctx, window);
  controller_module_->onContextRecreated(ctx, window);
  monitor_module_->onContextRecreated(ctx, window);
  cursor_module_->onContextRecreated(ctx, window);
  input_module_->onContextRecreated(ctx, window);
  event_log_module_->onContextRecreated(ctx, window);
  clipboard_module_->onContextRecreated(ctx, window);
  dnd_module_->onContextRecreated(ctx, window);
  window_module_->onContextRecreated(ctx, window);

  current_cursor_mode_ = LVKW_CURSOR_NORMAL;
  current_cursor_ = nullptr;
}

void App::updateCursor(lvkw::Context &ctx, lvkw::Window &window, ImGuiIO &io) {
  if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
    return;

  auto set_mode_if_needed = [&](LVKW_CursorMode mode) {
    if (current_cursor_mode_ == mode) return;
    window.setCursorMode(mode);
    current_cursor_mode_ = mode;
  };

  auto set_cursor_if_needed = [&](LVKW_Cursor *cursor) {
    if (current_cursor_ == cursor) return;
    window.setCursor(cursor);
    current_cursor_ = cursor;
  };

  LVKW_CursorMode requested_mode = cursor_module_->getRequestedMode();
  if (requested_mode != LVKW_CURSOR_NORMAL) {
    set_mode_if_needed(requested_mode);
    current_cursor_ = nullptr;
    return;
  }

  if (cursor_module_->hasActiveCursorOverride()) {
    set_mode_if_needed(LVKW_CURSOR_NORMAL);
    current_cursor_ = nullptr;
    return;
  }

  ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
  if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
    set_mode_if_needed(LVKW_CURSOR_HIDDEN);
    current_cursor_ = nullptr;
  } else {
    set_mode_if_needed(LVKW_CURSOR_NORMAL);

    LVKW_CursorShape shape = LVKW_CURSOR_SHAPE_DEFAULT;
    switch (imgui_cursor) {
    case ImGuiMouseCursor_Arrow:
      shape = LVKW_CURSOR_SHAPE_DEFAULT;
      break;
    case ImGuiMouseCursor_TextInput:
      shape = LVKW_CURSOR_SHAPE_TEXT;
      break;
    case ImGuiMouseCursor_ResizeAll:
      shape = LVKW_CURSOR_SHAPE_MOVE;
      break;
    case ImGuiMouseCursor_ResizeNS:
      shape = LVKW_CURSOR_SHAPE_NS_RESIZE;
      break;
    case ImGuiMouseCursor_ResizeEW:
      shape = LVKW_CURSOR_SHAPE_EW_RESIZE;
      break;
    case ImGuiMouseCursor_ResizeNESW:
      shape = LVKW_CURSOR_SHAPE_NESW_RESIZE;
      break;
    case ImGuiMouseCursor_ResizeNWSE:
      shape = LVKW_CURSOR_SHAPE_NWSE_RESIZE;
      break;
    case ImGuiMouseCursor_Hand:
      shape = LVKW_CURSOR_SHAPE_HAND;
      break;
    case ImGuiMouseCursor_NotAllowed:
      shape = LVKW_CURSOR_SHAPE_NOT_ALLOWED;
      break;
    }
    set_cursor_if_needed(ctx.getStandardCursor(shape));
  }
}
