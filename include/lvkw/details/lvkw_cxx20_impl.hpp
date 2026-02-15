// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CXX20_IMPL_HPP_INCLUDED
#define LVKW_CXX20_IMPL_HPP_INCLUDED

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

template <typename Visitor>
consteval LVKW_EventType inferEventMask() {
  using V = std::remove_cvref_t<Visitor>;
  uint32_t mask = 0;
  if constexpr (std::invocable<V, WindowReadyEvent>) mask |= LVKW_EVENT_TYPE_WINDOW_READY;
  if constexpr (std::invocable<V, WindowCloseEvent>) mask |= LVKW_EVENT_TYPE_CLOSE_REQUESTED;
  if constexpr (std::invocable<V, WindowResizedEvent>) mask |= LVKW_EVENT_TYPE_WINDOW_RESIZED;
  if constexpr (std::invocable<V, WindowMaximizationEvent>) mask |= LVKW_EVENT_TYPE_WINDOW_MAXIMIZED;
  if constexpr (std::invocable<V, KeyboardEvent>) mask |= LVKW_EVENT_TYPE_KEY;
  if constexpr (std::invocable<V, MouseMotionEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_MOTION;
  if constexpr (std::invocable<V, MouseButtonEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_BUTTON;
  if constexpr (std::invocable<V, MouseScrollEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_SCROLL;
  if constexpr (std::invocable<V, IdleEvent>) mask |= LVKW_EVENT_TYPE_IDLE_STATE_CHANGED;
  if constexpr (std::invocable<V, MonitorConnectionEvent>)
    mask |= LVKW_EVENT_TYPE_MONITOR_CONNECTION;
  if constexpr (std::invocable<V, MonitorModeEvent>) mask |= LVKW_EVENT_TYPE_MONITOR_MODE;
  if constexpr (std::invocable<V, TextInputEvent>) mask |= LVKW_EVENT_TYPE_TEXT_INPUT;
  if constexpr (std::invocable<V, TextCompositionEvent>) mask |= LVKW_EVENT_TYPE_TEXT_COMPOSITION;
  if constexpr (std::invocable<V, FocusEvent>) mask |= LVKW_EVENT_TYPE_FOCUS;
  if constexpr (std::invocable<V, DndHoverEvent>) mask |= LVKW_EVENT_TYPE_DND_HOVER;
  if constexpr (std::invocable<V, DndLeaveEvent>) mask |= LVKW_EVENT_TYPE_DND_LEAVE;
  if constexpr (std::invocable<V, DndDropEvent>) mask |= LVKW_EVENT_TYPE_DND_DROP;
#ifdef LVKW_ENABLE_CONTROLLER
  if constexpr (std::invocable<V, ControllerConnectionEvent>)
    mask |= LVKW_EVENT_TYPE_CONTROLLER_CONNECTION;
#endif
  return static_cast<LVKW_EventType>(mask);
}

}  // namespace details

/**
 * Scans the current event queue using a visitor or a generic lambda.
 */
template <typename F>
void scanEvents(Context &ctx, F &&f) {
  using F_raw = std::remove_cvref_t<F>;
  if constexpr (PartialEventVisitor<F_raw>) {
    scanEvents(ctx, details::inferEventMask<F_raw>(),
               [&](LVKW_EventType type, LVKW_Window *window, const LVKW_Event &evt) {
                 details::dispatchVisitor(type, window, evt, f);
               });
  } else if constexpr (std::invocable<F_raw, LVKW_EventType, LVKW_Window *, const LVKW_Event &>) {
    scanEvents(ctx, LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
  }
}

/**
 * Scans the current event queue using multiple function overloads at once.
 */
template <typename F1, typename F2, typename... Fs>
void scanEvents(Context &ctx, F1 &&f1, F2 &&f2, Fs &&...fs) {
  scanEvents(ctx, overloads{std::forward<F1>(f1), std::forward<F2>(f2), std::forward<Fs>(fs)...});
}

/**
 * Convenience shorthand for non-blocking event polling.
 */
template <typename... Fs>
void pollEvents(Context &ctx, Fs &&...handlers) {
  syncEvents(ctx, 0);
  scanEvents(ctx, std::forward<Fs>(handlers)...);
}

/**
 * Convenience shorthand for non-blocking event polling with an explicit mask.
 */
template <typename... Fs>
void pollEvents(Context &ctx, LVKW_EventType mask, Fs &&...handlers) {
  syncEvents(ctx, 0);
  scanEvents(ctx, mask, overloads{std::forward<Fs>(handlers)...});
}

/**
 * Convenience shorthand for blocking event waiting.
 */
template <typename Rep, typename Period, typename... Fs>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, Fs &&...handlers) {
  syncEvents(ctx, static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()));
  scanEvents(ctx, std::forward<Fs>(handlers)...);
}

/**
 * Convenience shorthand for blocking event waiting with an explicit mask.
 */
template <typename Rep, typename Period, typename... Fs>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, LVKW_EventType mask,
                Fs &&...handlers) {
  syncEvents(ctx, static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()));
  scanEvents(ctx, mask, overloads{std::forward<Fs>(handlers)...});
}

}  // namespace lvkw

#endif  // LVKW_CXX20_IMPL_HPP_INCLUDED
