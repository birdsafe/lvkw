// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CPP_CXX20_HPP_INCLUDED
#define LVKW_CPP_CXX20_HPP_INCLUDED

#include "lvkw/lvkw.hpp"

#include <concepts>
#include <type_traits>

namespace lvkw {

/** Utility for creating a visitor from multiple lambdas. */
template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overloads(Ts...) -> overloads<Ts...>;

template <typename T>
concept PartialEventVisitor = std::invocable<std::remove_cvref_t<T>, WindowReadyEvent> ||
                              std::invocable<std::remove_cvref_t<T>, WindowCloseEvent> ||
                              std::invocable<std::remove_cvref_t<T>, WindowResizedEvent> ||
                              std::invocable<std::remove_cvref_t<T>, WindowMaximizationEvent> ||
                              std::invocable<std::remove_cvref_t<T>, KeyboardEvent> ||
                              std::invocable<std::remove_cvref_t<T>, MouseMotionEvent> ||
                              std::invocable<std::remove_cvref_t<T>, MouseButtonEvent> ||
                              std::invocable<std::remove_cvref_t<T>, MouseScrollEvent> ||
                              std::invocable<std::remove_cvref_t<T>, IdleEvent> ||
                              std::invocable<std::remove_cvref_t<T>, MonitorConnectionEvent> ||
                              std::invocable<std::remove_cvref_t<T>, MonitorModeEvent> ||
                              std::invocable<std::remove_cvref_t<T>, TextInputEvent> ||
                              std::invocable<std::remove_cvref_t<T>, TextCompositionEvent> ||
                              std::invocable<std::remove_cvref_t<T>, FocusEvent> ||
                              std::invocable<std::remove_cvref_t<T>, DndHoverEvent> ||
                              std::invocable<std::remove_cvref_t<T>, DndLeaveEvent> ||
                              std::invocable<std::remove_cvref_t<T>, DndDropEvent>
#ifdef LVKW_ENABLE_CONTROLLER
                              || std::invocable<std::remove_cvref_t<T>, ControllerConnectionEvent>
#endif
    ;

/**
 * A helper class that can be used as event_userdata to dispatch events to visitors.
 */
template <typename Visitor>
class EventDispatcher {
 public:
  explicit EventDispatcher(Visitor &&visitor) : m_visitor(std::forward<Visitor>(visitor)) {}

  static void callback(LVKW_EventType type, LVKW_Window *window, const LVKW_Event *event,
                       void *userdata);

 private:
  Visitor m_visitor;
};

/**
 * Creates an EventDispatcher from one or more handlers.
 */
template <typename... Fs>
auto makeDispatcher(Fs &&...handlers) {
  if constexpr (sizeof...(Fs) == 1) {
    // We need to expand Fs here to get the single type, but there's only one.
    // Using a helper or just overloads is safer.
    return EventDispatcher<overloads<std::remove_cvref_t<Fs>...>>(
        overloads{std::forward<Fs>(handlers)...});
  } else {
    return EventDispatcher<overloads<std::remove_cvref_t<Fs>...>>(
        overloads{std::forward<Fs>(handlers)...});
  }
}

}  // namespace lvkw

#include "lvkw/details/lvkw_cxx20_impl.hpp"

#endif  // LVKW_CPP_CXX20_HPP_INCLUDED
