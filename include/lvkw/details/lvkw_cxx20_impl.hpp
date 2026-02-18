// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CXX20_IMPL_HPP_INCLUDED
#define LVKW_CXX20_IMPL_HPP_INCLUDED

#include <utility>

namespace lvkw {

namespace details {

template <typename Visitor>
void dispatchVisitor(LVKW_EventType type, LVKW_Window *window, const LVKW_Event &evt, Visitor &&f) {
  using F_raw = std::remove_cvref_t<Visitor>;
  switch (type) {
    case LVKW_EVENT_TYPE_WINDOW_READY:
      if constexpr (std::invocable<F_raw, WindowReadyEvent>)
        f(WindowReadyEvent{window, evt.window_ready});
      break;
    case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
      if constexpr (std::invocable<F_raw, WindowCloseEvent>)
        f(WindowCloseEvent{window, evt.close_requested});
      break;
    case LVKW_EVENT_TYPE_WINDOW_RESIZED:
      if constexpr (std::invocable<F_raw, WindowResizedEvent>)
        f(WindowResizedEvent{window, evt.resized});
      break;
    case LVKW_EVENT_TYPE_WINDOW_MAXIMIZED:
      if constexpr (std::invocable<F_raw, WindowMaximizationEvent>)
        f(WindowMaximizationEvent{window, evt.maximized});
      break;
    case LVKW_EVENT_TYPE_KEY:
      if constexpr (std::invocable<F_raw, KeyboardEvent>) f(KeyboardEvent{window, evt.key});
      break;
    case LVKW_EVENT_TYPE_MOUSE_MOTION:
      if constexpr (std::invocable<F_raw, MouseMotionEvent>)
        f(MouseMotionEvent{window, evt.mouse_motion});
      break;
    case LVKW_EVENT_TYPE_MOUSE_BUTTON:
      if constexpr (std::invocable<F_raw, MouseButtonEvent>)
        f(MouseButtonEvent{window, evt.mouse_button});
      break;
    case LVKW_EVENT_TYPE_MOUSE_SCROLL:
      if constexpr (std::invocable<F_raw, MouseScrollEvent>)
        f(MouseScrollEvent{window, evt.mouse_scroll});
      break;
    case LVKW_EVENT_TYPE_IDLE_STATE_CHANGED:
      if constexpr (std::invocable<F_raw, IdleEvent>) f(IdleEvent{window, evt.idle});
      break;
    case LVKW_EVENT_TYPE_MONITOR_CONNECTION:
      if constexpr (std::invocable<F_raw, MonitorConnectionEvent>)
        f(MonitorConnectionEvent{window, evt.monitor_connection});
      break;
    case LVKW_EVENT_TYPE_MONITOR_MODE:
      if constexpr (std::invocable<F_raw, MonitorModeEvent>)
        f(MonitorModeEvent{window, evt.monitor_mode});
      break;
    case LVKW_EVENT_TYPE_TEXT_INPUT:
      if constexpr (std::invocable<F_raw, TextInputEvent>) f(TextInputEvent{window, evt.text_input});
      break;
    case LVKW_EVENT_TYPE_TEXT_COMPOSITION:
      if constexpr (std::invocable<F_raw, TextCompositionEvent>)
        f(TextCompositionEvent{window, evt.text_composition});
      break;
    case LVKW_EVENT_TYPE_FOCUS:
      if constexpr (std::invocable<F_raw, FocusEvent>) f(FocusEvent{window, evt.focus});
      break;
    case LVKW_EVENT_TYPE_DND_HOVER:
      if constexpr (std::invocable<F_raw, DndHoverEvent>) f(DndHoverEvent{window, evt.dnd_hover});
      break;
    case LVKW_EVENT_TYPE_DND_LEAVE:
      if constexpr (std::invocable<F_raw, DndLeaveEvent>) f(DndLeaveEvent{window, evt.dnd_leave});
      break;
    case LVKW_EVENT_TYPE_DND_DROP:
      if constexpr (std::invocable<F_raw, DndDropEvent>) f(DndDropEvent{window, evt.dnd_drop});
      break;
#ifdef LVKW_ENABLE_CONTROLLER
    case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION:
      if constexpr (std::invocable<F_raw, ControllerConnectionEvent>)
        f(ControllerConnectionEvent{window, evt.controller_connection});
      break;
#endif
    default:
      break;
  }
}

}  // namespace details

template <typename Visitor>
void EventDispatcher<Visitor>::callback(LVKW_EventType type, LVKW_Window *window,
                                       const LVKW_Event *event, void *userdata) {
  auto *self = static_cast<EventDispatcher<Visitor> *>(userdata);
  if constexpr (PartialEventVisitor<Visitor>) {
    details::dispatchVisitor(type, window, *event, self->m_visitor);
  } else if constexpr (std::invocable<Visitor, LVKW_EventType, LVKW_Window *, const LVKW_Event &>) {
    self->m_visitor(type, window, *event);
  }
}

}  // namespace lvkw

#endif  // LVKW_CXX20_IMPL_HPP_INCLUDED
