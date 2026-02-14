// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_CXX20_HPP_INCLUDED
#define LVKW_CXX20_HPP_INCLUDED

#include "lvkw.hpp"

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
 * Scans the current event queue using a visitor or a generic lambda.
 * @tparam F The visitor or lambda type.
 * @param ctx The library context.
 * @param f The event handler.
 */
template <typename F>
void scanEvents(Context &ctx, F &&f);

/**
 * Scans the current event queue using multiple function overloads at once.
 */
template <typename F1, typename F2, typename... Fs>
void scanEvents(Context &ctx, F1 &&f1, F2 &&f2, Fs &&...fs);

/**
 * Convenience shorthand for non-blocking event polling.
 */
template <typename... Fs>
void pollEvents(Context &ctx, Fs &&...handlers);

/**
 * Convenience shorthand for non-blocking event polling with an explicit mask.
 */
template <typename... Fs>
void pollEvents(Context &ctx, LVKW_EventType mask, Fs &&...handlers);

/**
 * Convenience shorthand for blocking event waiting.
 */
template <typename Rep, typename Period, typename... Fs>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, Fs &&...handlers);

/**
 * Convenience shorthand for blocking event waiting with an explicit mask.
 */
template <typename Rep, typename Period, typename... Fs>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, LVKW_EventType mask,
                Fs &&...handlers);

}  // namespace lvkw

#include "details/lvkw_cxx20_impl.hpp"

#endif  // LVKW_CXX20_HPP_INCLUDED
