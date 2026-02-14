// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_HPP_INCLUDED
#define LVKW_HPP_INCLUDED

#include <chrono>
#include <concepts>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "lvkw.h"

namespace lvkw {

template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

/**
 * A thin wrapper that bundles a specific event payload with its window metadata.
 *
 * @note LIFETIME: Event objects and their underlying data are only valid
 * for the duration of the callback. If you need to store event data for
 * later, copy the underlying C structure.
 *
 * @tparam T The C event structure type (e.g., LVKW_KeyboardEvent).
 */
template <typename T>
struct Event {
  LVKW_Window *window;
  const T &data;

  /** Provides pointer-like access to the event data. */
  const T *operator->() const { return &data; }
  /** Implicitly converts to the underlying data structure. */
  operator const T &() const { return data; }
};

/** Specific C++ event types that include window metadata. */
using WindowReadyEvent = Event<LVKW_WindowReadyEvent>;
using WindowCloseEvent = Event<LVKW_WindowCloseEvent>;
using WindowResizedEvent = Event<LVKW_WindowResizedEvent>;
using WindowMaximizationEvent = Event<LVKW_WindowMaximizationEvent>;
using KeyboardEvent = Event<LVKW_KeyboardEvent>;
using MouseMotionEvent = Event<LVKW_MouseMotionEvent>;
using MouseButtonEvent = Event<LVKW_MouseButtonEvent>;
using MouseScrollEvent = Event<LVKW_MouseScrollEvent>;
using IdleEvent = Event<LVKW_IdleEvent>;
using MonitorConnectionEvent = Event<LVKW_MonitorConnectionEvent>;
using MonitorModeEvent = Event<LVKW_MonitorModeEvent>;
using TextInputEvent = Event<LVKW_TextInputEvent>;
using TextCompositionEvent = Event<LVKW_TextCompositionEvent>;
using FocusEvent = Event<LVKW_FocusEvent>;

/**
 * C++ wrapper for DND feedback state.
 */
struct DndFeedback {
  LVKW_DndFeedback *raw;

  /** Access or modify the DND action. */
  LVKW_DndAction &action() const { return *raw->action; }
  /** Access or modify the session-persistent user data. */
  void *&sessionUserdata() const { return *raw->session_userdata; }
};

/** C++ wrapper for DndHoverEvent. */
struct DndHoverEvent : public Event<LVKW_DndHoverEvent> {
  /** Access the feedback object. */
  DndFeedback feedback() const { return {data.feedback}; }

  /** Shorthand for feedback().action(). */
  LVKW_DndAction &action() const { return feedback().action(); }
  /** Shorthand for feedback().sessionUserdata(). */
  void *&sessionUserdata() const { return feedback().sessionUserdata(); }
};

using DndLeaveEvent = Event<LVKW_DndLeaveEvent>;
using DndDropEvent = Event<LVKW_DndDropEvent>;

#ifdef LVKW_ENABLE_CONTROLLER
using ControllerConnectionEvent = Event<LVKW_CtrlConnectionEvent>;
#endif

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

/** Thrown when an LVKW operation fails. */
class Exception : public std::runtime_error {
 public:
  /**
   * Creates a new Exception.
   * @param status The LVKW_Status that triggered the failure.
   * @param message A description of what went wrong.
   */
  Exception(LVKW_Status status, const char *message)
      : std::runtime_error(message), m_status(status) {}
  /** Returns the raw status code that caused this exception. */
  LVKW_Status status() const { return m_status; }

  /** Returns true if the operation failed. */
  bool isError() const { return m_status != LVKW_SUCCESS; }

 private:
  LVKW_Status m_status;
};

/** Thrown when a window handle is lost. */
class WindowLostException : public Exception {
 public:
  WindowLostException(const char *message) : Exception(LVKW_ERROR_WINDOW_LOST, message) {}
};

/** Thrown when the entire library context is lost. */
class ContextLostException : public Exception {
 public:
  ContextLostException(const char *message) : Exception(LVKW_ERROR_CONTEXT_LOST, message) {}
};

/**
 * Checks a status code and throws if it indicates an error.
 * @param status The status code to check.
 * @param message The message to include in the exception.
 * @throws Exception or a more specific subclass if status is not LVKW_SUCCESS.
 */
#ifdef LVKW_ENABLE_DIAGNOSTICS
inline void check(LVKW_Status status, const char *message) {
  switch (status) {
    case LVKW_SUCCESS:
      break;
    case LVKW_ERROR_WINDOW_LOST:
      throw WindowLostException(message);
    case LVKW_ERROR_CONTEXT_LOST:
      throw ContextLostException(message);
    default:
      throw Exception(status, message);
  }
}
#else
inline void check(LVKW_Status status, const char *message) {
  switch (status) {
    case LVKW_SUCCESS:
      break;
    case LVKW_ERROR_WINDOW_LOST:
      throw WindowLostException("");
    case LVKW_ERROR_CONTEXT_LOST:
      throw ContextLostException("");
    default:
      throw Exception(status, "");
  }
}
#endif

/** A handy RAII wrapper for an LVKW_Cursor. */
class Cursor {
 public:
  /** Destroys the cursor if it was created by the user. */
  ~Cursor() {
    if (is_owned()) {
      lvkw_cursor_destroy(m_cursor_handle);
    }
  }

  Cursor(const Cursor &) = delete;
  Cursor &operator=(const Cursor &) = delete;

  /** Moves a cursor from another instance. */
  Cursor(Cursor &&other) noexcept : m_cursor_handle(other.m_cursor_handle) {
    other.m_cursor_handle = nullptr;
  }

  /** Transfers ownership of a cursor handle. */
  Cursor &operator=(Cursor &&other) noexcept {
    if (this != &other) {
      if (is_owned()) {
        lvkw_cursor_destroy(m_cursor_handle);
      }
      m_cursor_handle = other.m_cursor_handle;
      other.m_cursor_handle = nullptr;
    }
    return *this;
  }

  /** Gives you the underlying C cursor handle.
   *  @return The raw LVKW_Cursor handle. */
  LVKW_Cursor *get() const { return m_cursor_handle; }

 private:
  LVKW_Cursor *m_cursor_handle = nullptr;

  Cursor(LVKW_Cursor *handle) : m_cursor_handle(handle) {}

  bool is_owned() const {
    return m_cursor_handle && !(m_cursor_handle->flags & LVKW_CURSOR_FLAG_SYSTEM);
  }
  friend class Context;
};

/** A handy RAII wrapper for an LVKW_Window. */
class Window {
 public:
  /** Destroys the window and cleans up. */
  ~Window() {
    if (m_window_handle) {
      lvkw_wnd_destroy(m_window_handle);
    }
  }

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  /** Moves a window from another instance. */
  Window(Window &&other) noexcept : m_window_handle(other.m_window_handle) {
    other.m_window_handle = nullptr;
  }

  /** Transfers ownership of a window handle. */
  Window &operator=(Window &&other) noexcept {
    if (this != &other) {
      if (m_window_handle) {
        lvkw_wnd_destroy(m_window_handle);
      }
      m_window_handle = other.m_window_handle;
      other.m_window_handle = nullptr;
    }
    return *this;
  }

  /** Gives you the underlying C window handle.
   *  @return The raw LVKW_Window handle. */
  LVKW_Window *get() const { return m_window_handle; }

  /** Creates a Vulkan surface for this specific window.
   *
   *  PRECONDITION: The window must be ready (check isReady()).
   *
   *  @param instance The Vulkan instance.
   *  @return The created VkSurfaceKHR.
   *  @throws Exception if surface creation fails. */
  VkSurfaceKHR createVkSurface(VkInstance instance) const {
    VkSurfaceKHR surface;
    check(lvkw_wnd_createVkSurface(m_window_handle, instance, &surface),
          "Failed to create Vulkan surface");
    return surface;
  }

  /** Returns the current geometry (logical and physical size) of the window.
   *  @return The window geometry.
   *  @throws Exception if the query fails. */
  LVKW_WindowGeometry getGeometry() const {
    LVKW_WindowGeometry geometry;
    check(lvkw_wnd_getGeometry(m_window_handle, &geometry), "Failed to get window geometry");
    return geometry;
  }

  /** Returns true if the window handle is lost.
   *
   *  A lost window must be destroyed and recreated.
   */
  bool isLost() const { return m_window_handle->flags & LVKW_WND_STATE_LOST; }

  /** Returns true if the window is ready for rendering.
   *
   *  Do not call createVkSurface() or getGeometry() until this
   *  returns true.
   */
  bool isReady() const { return m_window_handle->flags & LVKW_WND_STATE_READY; }

  /** Returns true if the window currently has input focus. */
  bool isFocused() const { return m_window_handle->flags & LVKW_WND_STATE_FOCUSED; }

  /** Returns true if the window is currently maximized. */
  bool isMaximized() const { return m_window_handle->flags & LVKW_WND_STATE_MAXIMIZED; }

  /** Returns your custom window-specific user data.
   *  @return The userdata pointer. */
  void *getUserData() const { return m_window_handle->userdata; }

  /** Sets your custom window-specific user data.
   *  @param userdata The new userdata pointer. */
  void setUserData(void *userdata) { m_window_handle->userdata = userdata; }

  /** Updates specific attributes of this window.
   *  @param field_mask A bitmask of LVKW_WindowAttributesField.
   *  @param attrs The new attribute values.
   *  @throws Exception if the update fails. */
  void update(uint32_t field_mask, const LVKW_WindowAttributes &attrs) {
    check(lvkw_wnd_update(m_window_handle, field_mask, &attrs),
          "Failed to update window attributes");
  }

  /** Sets the title of the window (UTF-8).
   *  @param title The new title. */
  void setTitle(const char *title) {
    LVKW_WindowAttributes attrs = {};
    attrs.title = title;
    update(LVKW_WND_ATTR_TITLE, attrs);
  }

  /** Sets the logical size of the window.
   *  @param size The new size. */
  void setSize(LVKW_LogicalVec size) {
    LVKW_WindowAttributes attrs = {};
    attrs.logicalSize = size;
    update(LVKW_WND_ATTR_LOGICAL_SIZE, attrs);
  }

  /** Switches the window in or out of fullscreen mode.
   *  @param enabled True to enable fullscreen. */
  void setFullscreen(bool enabled) {
    LVKW_WindowAttributes attrs = {};
    attrs.fullscreen = enabled;
    update(LVKW_WND_ATTR_FULLSCREEN, attrs);
  }

  /** Switches the window in or out of maximized mode.
   *  @param enabled True to maximize. */
  void setMaximized(bool enabled) {
    LVKW_WindowAttributes attrs = {};
    attrs.maximized = enabled;
    update(LVKW_WND_ATTR_MAXIMIZED, attrs);
  }

  /** Sets how the cursor should behave (e.g. normal or locked).
   *  @param mode The new cursor mode. */
  void setCursorMode(LVKW_CursorMode mode) {
    LVKW_WindowAttributes attrs = {};
    attrs.cursor_mode = mode;
    update(LVKW_WND_ATTR_CURSOR_MODE, attrs);
  }

  /** Changes the current hardware cursor.
   *  @param cursor The new cursor. NULL for system default. */
  void setCursor(LVKW_Cursor *cursor) {
    LVKW_WindowAttributes attrs = {};
    attrs.cursor = cursor;
    update(LVKW_WND_ATTR_CURSOR, attrs);
  }

  /** Changes the current hardware cursor.
   *  @param cursor The RAII cursor object. */
  void setCursor(const Cursor &cursor) { setCursor(cursor.get()); }

  /** Sets the minimum logical size of the window.
   *  @param minSize The new minimum size. {0,0} for no limit. */
  void setMinSize(LVKW_LogicalVec minSize) {
    LVKW_WindowAttributes attrs = {};
    attrs.minSize = minSize;
    update(LVKW_WND_ATTR_MIN_SIZE, attrs);
  }

  /** Sets the maximum logical size of the window.
   *  @param maxSize The new maximum size. {0,0} for no limit. */
  void setMaxSize(LVKW_LogicalVec maxSize) {
    LVKW_WindowAttributes attrs = {};
    attrs.maxSize = maxSize;
    update(LVKW_WND_ATTR_MAX_SIZE, attrs);
  }

  /** Sets the aspect ratio of the window.
   *  @param aspectRatio The new aspect ratio. {0,0} for no limit. */
  void setAspectRatio(LVKW_Ratio aspectRatio) {
    LVKW_WindowAttributes attrs = {};
    attrs.aspect_ratio = aspectRatio;
    update(LVKW_WND_ATTR_ASPECT_RATIO, attrs);
  }

  /** Toggles whether the window is resizable by the user.
   *  @param resizable True to allow resizing. */
  void setResizable(bool resizable) {
    LVKW_WindowAttributes attrs = {};
    attrs.resizable = resizable;
    update(LVKW_WND_ATTR_RESIZABLE, attrs);
  }

  /** Toggles whether the window has OS decorations.
   *  @param decorated True to show decorations. */
  void setDecorated(bool decorated) {
    LVKW_WindowAttributes attrs = {};
    attrs.decorated = decorated;
    update(LVKW_WND_ATTR_DECORATED, attrs);
  }

  /** Toggles whether mouse events pass through the window.
   *  @param passthrough True to enable passthrough. */
  void setMousePassthrough(bool passthrough) {
    LVKW_WindowAttributes attrs = {};
    attrs.mouse_passthrough = passthrough;
    update(LVKW_WND_ATTR_MOUSE_PASSTHROUGH, attrs);
  }

  /** Asks the system to give this window input focus.
   *  @throws Exception if the request fails. */
  void requestFocus() { check(lvkw_wnd_requestFocus(m_window_handle), "Failed to request focus"); }

  /** Sets the system clipboard content to a UTF-8 string.
   *  @param text The null-terminated UTF-8 string to copy.
   *  @throws Exception if the operation fails. */
  void setClipboardText(const char *text) {
    check(lvkw_wnd_setClipboardText(m_window_handle, text), "Failed to set clipboard text");
  }

  /** Retrieves the current system clipboard content as a UTF-8 string.
   *
   *  @note LIFETIME: The returned pointer is managed by the library and remains valid
   *  until the next call to getClipboardText on any window belonging to the
   *  same context, or until the context is destroyed.
   *
   *  @return The clipboard text.
   *  @throws Exception if the operation fails. */
  const char *getClipboardText() const {
    const char *text;
    check(lvkw_wnd_getClipboardText(m_window_handle, &text), "Failed to get clipboard text");
    return text;
  }

  /** Sets the system clipboard with multiple data formats (MIME types).
   *  @param data Span of clipboard data items.
   *  @throws Exception if the operation fails. */
  void setClipboardData(std::span<const LVKW_ClipboardData> data) {
    check(
        lvkw_wnd_setClipboardData(m_window_handle, data.data(), static_cast<uint32_t>(data.size())),
        "Failed to set clipboard data");
  }

  /** Retrieves specific MIME type data from the clipboard.
   *
   *  @note LIFETIME: Managed by the library.
   *
   *  @param mime_type The desired MIME type.
   *  @return A span covering the retrieved data.
   *  @throws Exception if the operation fails. */
  std::span<const uint8_t> getClipboardData(const char *mime_type) const {
    const void *data;
    size_t size;
    check(lvkw_wnd_getClipboardData(m_window_handle, mime_type, &data, &size),
          "Failed to get clipboard data");
    return {static_cast<const uint8_t *>(data), size};
  }

  /** Enumerates all MIME types currently available on the clipboard.
   *  @return A vector of MIME type strings.
   *  @throws Exception if enumeration fails. */
  std::vector<const char *> getClipboardMimeTypes() const {
    uint32_t count = 0;
    check(lvkw_wnd_getClipboardMimeTypes(m_window_handle, nullptr, &count),
          "Failed to get MIME type count");
    if (count == 0) return {};
    const char **mime_types_ptr = nullptr;
    check(lvkw_wnd_getClipboardMimeTypes(m_window_handle, &mime_types_ptr, &count),
          "Failed to get MIME types");
    if (!mime_types_ptr || count == 0) return {};
    return std::vector<const char *>(mime_types_ptr, mime_types_ptr + count);
  }

 private:
  LVKW_Window *m_window_handle = nullptr;

  /** Creates a new window within your library context. */
  explicit Window(LVKW_Window *handle) : m_window_handle(handle) {}

  friend class Context;
};

#ifdef LVKW_ENABLE_CONTROLLER
/** A handy RAII wrapper for an LVKW_Controller. */
class Controller {
 public:
  /** Destroys the controller and cleans up. */
  ~Controller() {
    if (m_controller_handle) {
      lvkw_ctrl_destroy(m_controller_handle);
    }
  }

  Controller(const Controller &) = delete;
  Controller &operator=(const Controller &) = delete;

  /** Moves a controller from another instance. */
  Controller(Controller &&other) noexcept : m_controller_handle(other.m_controller_handle) {
    other.m_controller_handle = nullptr;
  }

  /** Transfers ownership of a controller handle. */
  Controller &operator=(Controller &&other) noexcept {
    if (this != &other) {
      if (m_controller_handle) {
        lvkw_ctrl_destroy(m_controller_handle);
      }
      m_controller_handle = other.m_controller_handle;
      other.m_controller_handle = nullptr;
    }
    return *this;
  }

  /** Gives you the underlying C controller handle.
   *  @return The raw LVKW_Controller handle. */
  LVKW_Controller *get() const { return m_controller_handle; }

  /** Provides pointer-like access to the controller structure. */
  const LVKW_Controller *operator->() const { return m_controller_handle; }

  /** Returns detailed information about this controller.
   *  @return The controller info.
   *  @throws Exception if the query fails. */
  LVKW_CtrlInfo getInfo() const {
    LVKW_CtrlInfo info;
    check(lvkw_ctrl_getInfo(m_controller_handle, &info), "Failed to get controller info");
    return info;
  }

  /** Returns true if the controller is lost (unplugged). */
  bool isLost() const { return m_controller_handle->flags & LVKW_WND_STATE_LOST; }

  /** Sets the haptic intensities for a range of channels.
   *  @param first_haptic Index of the first haptic channel to update.
   *  @param intensities Span or array of normalized values [0.0, 1.0]. */
  void setHapticLevels(uint32_t first_haptic, std::span<const LVKW_real_t> intensities) {
    check(lvkw_ctrl_setHapticLevels(m_controller_handle, first_haptic,
                                    static_cast<uint32_t>(intensities.size()), intensities.data()),
          "Failed to set controller haptic levels");
  }

  /** Convenience method for setting standard dual-motor rumble.
   *  @param low_freq Intensity for the large motor [0.0, 1.0].
   *  @param high_freq Intensity for the small motor [0.0, 1.0]. */
  void setRumble(LVKW_real_t low_freq, LVKW_real_t high_freq) {
    const LVKW_real_t levels[] = {low_freq, high_freq};
    setHapticLevels(LVKW_CTRL_HAPTIC_LOW_FREQ, levels);
  }

 private:
  LVKW_Controller *m_controller_handle = nullptr;

  explicit Controller(LVKW_Controller *handle) : m_controller_handle(handle) {}

  friend class Context;
};
#endif

/** A handy RAII wrapper for the LVKW_Context.
 *
 *  @note THREAD SAFETY: All calls to a context and its associated windows
 *  must originate from the thread that created the context.
 */
class Context {
 public:
  /**
   * A default diagnostics logger that prints straight to std::cerr.
   */
  static void defaultDiagnosticCallback(const LVKW_DiagnosticInfo *info, void * /*userdata*/) {
    std::cerr << "LVKW Diagnostic: " << info->message << " (Code: " << (int)info->diagnostic << ")"
              << std::endl;
  }

  /** Creates a context with default settings (AUTO backend).
   *  @throws Exception if creation fails. */
  Context() {
    LVKW_ContextCreateInfo ci = {};
    ci.backend = LVKW_BACKEND_AUTO;
    ci.attributes.diagnostic_cb = defaultDiagnosticCallback;
    check(lvkw_createContext(&ci, &m_ctx_handle), "Failed to create LVKW context");
  }

  /** Creates a context using your specific creation options.
   *  @param create_info Creation parameters.
   *  @throws Exception if creation fails. */
  explicit Context(const LVKW_ContextCreateInfo &create_info) {
    LVKW_ContextCreateInfo ci = create_info;
    if (!ci.attributes.diagnostic_cb) {
      ci.attributes.diagnostic_cb = defaultDiagnosticCallback;
    }
    check(lvkw_createContext(&ci, &m_ctx_handle), "Failed to create LVKW context");
  }

  /** Cleans up and destroys the context. */
  ~Context() {
    if (m_ctx_handle) {
      lvkw_ctx_destroy(m_ctx_handle);
    }
  }

  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  /** Moves a context from another instance.
   *  @param other The context to move from. */
  Context(Context &&other) noexcept : m_ctx_handle(other.m_ctx_handle) {
    other.m_ctx_handle = nullptr;
  }

  /** Transfers ownership of a context handle.
   *  @param other The context to move from.
   *  @return A reference to this context. */
  Context &operator=(Context &&other) noexcept {
    if (this != &other) {
      if (m_ctx_handle) {
        lvkw_ctx_destroy(m_ctx_handle);
      }
      m_ctx_handle = other.m_ctx_handle;
      other.m_ctx_handle = nullptr;
    }
    return *this;
  }

  /** Gives you the underlying C context handle.
   *  @return The raw LVKW_Context handle. */
  LVKW_Context *get() const { return m_ctx_handle; }

  /** Returns a list of Vulkan extensions required for this context.
   *  @return A vector of extension names. */
  std::vector<const char *> getVkExtensions() const {
    uint32_t count = 0;
    const char *const *extensions = nullptr;
    check(lvkw_ctx_getVkExtensions(m_ctx_handle, &count, &extensions),
          "Failed to get Vulkan extensions");
    if (!extensions || count == 0) return {};
    return std::vector<const char *>(extensions, extensions + count);
  }

  /** Polls for events and passes them to your callback function.
   *
   *  @note LIFETIME: The event data passed to the callback is only valid
   *  for the duration of the callback.
   *
   *  @tparam F The callback type.
   *  @param event_mask Bitmask of events to poll.
   *  @param callback Function to call for each event.
   *  @throws Exception if polling fails. */
  template <typename F>
    requires std::invocable<F, LVKW_EventType, LVKW_Window *, const LVKW_Event &>
  void pollEvents(LVKW_EventType event_mask, F &&callback) {
    check(lvkw_ctx_pollEvents(
              m_ctx_handle, event_mask,
              [](LVKW_EventType type, LVKW_Window *window, const LVKW_Event *evt, void *userdata) {
                auto &cb = *static_cast<std::remove_reference_t<F> *>(userdata);
                cb(type, window, *evt);
              },
              &callback),
          "Failed to poll events");
  }

  /** Waits for events with a timeout, then sends them to your callback.
   *
   *  @note LIFETIME: The event data passed to the callback is only valid
   *  for the duration of the callback.
   *
   *  @tparam F The callback type.
   *  @tparam Duration A std::chrono duration type.
   *  @param timeout Maximum time to wait.
   *  @param event_mask Bitmask of events to poll.
   *  @param callback Function to call for each event.
   *  @throws Exception if waiting fails. */
  template <typename F, typename Duration>
    requires std::invocable<F, LVKW_EventType, LVKW_Window *, const LVKW_Event &>
  void waitEvents(Duration timeout, LVKW_EventType event_mask, F &&callback) {
    auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
    check(lvkw_ctx_waitEvents(
              m_ctx_handle, (uint32_t)timeout_ms, event_mask,
              [](LVKW_EventType type, LVKW_Window *window, const LVKW_Event *evt, void *userdata) {
                auto &cb = *static_cast<std::remove_reference_t<F> *>(userdata);
                cb(type, window, *evt);
              },
              &callback),
          "Failed to wait for events");
  }

  /** Polls for events using a visitor or a generic lambda.
   *  @tparam F The visitor or lambda type.
   *  @param f The event handler. */
  template <typename F>
  void pollEvents(F &&f) {
    using F_raw = std::remove_cvref_t<F>;
    if constexpr (PartialEventVisitor<F_raw>) {
      pollEvents(inferEventMask<F_raw>(),
                 [&](LVKW_EventType type, LVKW_Window *window, const LVKW_Event &evt) {
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
                       if constexpr (std::invocable<F_raw, KeyboardEvent>)
                         f(KeyboardEvent{window, evt.key});
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
                     case LVKW_EVENT_TYPE_IDLE_NOTIFICATION:
                       if constexpr (std::invocable<F_raw, IdleEvent>)
                         f(IdleEvent{window, evt.idle});
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
                       if constexpr (std::invocable<F_raw, TextInputEvent>)
                         f(TextInputEvent{window, evt.text_input});
                       break;
                     case LVKW_EVENT_TYPE_TEXT_COMPOSITION:
                       if constexpr (std::invocable<F_raw, TextCompositionEvent>)
                         f(TextCompositionEvent{window, evt.text_composition});
                       break;
                     case LVKW_EVENT_TYPE_FOCUS:
                       if constexpr (std::invocable<F_raw, FocusEvent>)
                         f(FocusEvent{window, evt.focus});
                       break;
                     case LVKW_EVENT_TYPE_DND_HOVER:
                       if constexpr (std::invocable<F_raw, DndHoverEvent>)
                         f(DndHoverEvent{window, evt.dnd_hover});
                       break;
                     case LVKW_EVENT_TYPE_DND_LEAVE:
                       if constexpr (std::invocable<F_raw, DndLeaveEvent>)
                         f(DndLeaveEvent{window, evt.dnd_leave});
                       break;
                     case LVKW_EVENT_TYPE_DND_DROP:
                       if constexpr (std::invocable<F_raw, DndDropEvent>)
                         f(DndDropEvent{window, evt.dnd_drop});
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
                 });
    }
    else if constexpr (std::invocable<F_raw, LVKW_EventType, LVKW_Window *, const LVKW_Event &>) {
      pollEvents(LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
    }
  }

  /** Waits for events with a timeout, then dispatches them to a visitor.
   *  @tparam F The visitor or lambda type.
   *  @tparam Duration A std::chrono duration type.
   *  @param timeout Maximum time to wait.
   *  @param f The event handler. */
  template <typename F, typename Duration>
  void waitEvents(Duration timeout, F &&f) {
    using F_raw = std::remove_cvref_t<F>;
    if (m_ctx_handle == nullptr) return;

    if constexpr (PartialEventVisitor<F_raw>) {
      waitEvents(timeout, inferEventMask<F_raw>(),
                 [&](LVKW_EventType type, LVKW_Window *window, const LVKW_Event &evt) {
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
                       if constexpr (std::invocable<F_raw, KeyboardEvent>)
                         f(KeyboardEvent{window, evt.key});
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
                     case LVKW_EVENT_TYPE_IDLE_NOTIFICATION:
                       if constexpr (std::invocable<F_raw, IdleEvent>)
                         f(IdleEvent{window, evt.idle});
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
                       if constexpr (std::invocable<F_raw, TextInputEvent>)
                         f(TextInputEvent{window, evt.text_input});
                       break;
                     case LVKW_EVENT_TYPE_TEXT_COMPOSITION:
                       if constexpr (std::invocable<F_raw, TextCompositionEvent>)
                         f(TextCompositionEvent{window, evt.text_composition});
                       break;
                     case LVKW_EVENT_TYPE_FOCUS:
                       if constexpr (std::invocable<F_raw, FocusEvent>)
                         f(FocusEvent{window, evt.focus});
                       break;
                     case LVKW_EVENT_TYPE_DND_HOVER:
                       if constexpr (std::invocable<F_raw, DndHoverEvent>)
                         f(DndHoverEvent{window, evt.dnd_hover});
                       break;
                     case LVKW_EVENT_TYPE_DND_LEAVE:
                       if constexpr (std::invocable<F_raw, DndLeaveEvent>)
                         f(DndLeaveEvent{window, evt.dnd_leave});
                       break;
                     case LVKW_EVENT_TYPE_DND_DROP:
                       if constexpr (std::invocable<F_raw, DndDropEvent>)
                         f(DndDropEvent{window, evt.dnd_drop});
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
                 });
    }
    else if constexpr (std::invocable<F_raw, LVKW_EventType, LVKW_Window *, const LVKW_Event &>) {
      waitEvents(timeout, LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
    }
  }

  /** Dispatches events using multiple function overloads at once.
   *  @tparam F1 First handler type.
   *  @tparam F2 Second handler type.
   *  @tparam Fs Remaining handler types.
   *  @param f1 First handler.
   *  @param f2 Second handler.
   *  @param fs Remaining handlers. */
  template <typename F1, typename F2, typename... Fs>
  void pollEvents(F1 &&f1, F2 &&f2, Fs &&...fs) {
    pollEvents(overloads{std::forward<F1>(f1), std::forward<F2>(f2), std::forward<Fs>(fs)...});
  }

  /** Configures how long to wait before sending an idle notification.
   *  @param timeout_ms The threshold in milliseconds. */
  void setIdleTimeout(uint32_t timeout_ms) {
    LVKW_ContextAttributes attrs = {};
    attrs.idle_timeout_ms = timeout_ms;
    check(lvkw_ctx_update(m_ctx_handle, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs),
          "Failed to update context attributes");
  }

  /** Prevents the system from going idle or sleeping.
   *  @param enabled True to inhibit idle. */
  void setIdleInhibition(bool enabled) {
    LVKW_ContextAttributes attrs = {};
    attrs.inhibit_idle = enabled;
    check(lvkw_ctx_update(m_ctx_handle, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs),
          "Failed to update context attributes");
  }

  /** Sets the diagnostics callback for this context.
   *  @param callback The diagnostics callback function.
   *  @param userdata User data for the callback. */
  void setDiagnosticCallback(LVKW_DiagnosticCallback callback, void *userdata) {
    check(lvkw_ctx_setDiagnosticCallback(m_ctx_handle, callback, userdata),
          "Failed to set diagnostics callback");
  }

  /** Returns true if the context handle is lost.
   *
   *  A lost context must be destroyed. All associated windows are also lost.
   */
  bool isLost() const { return m_ctx_handle->flags & LVKW_CTX_STATE_LOST; }

  /** Returns your custom global user data pointer.
   *  @return The global userdata pointer. */
  void *getUserData() const { return m_ctx_handle->userdata; }

  /** Sets your custom global user data pointer.
   *  @param userdata The new global userdata pointer. */
  void setUserData(void *userdata) { m_ctx_handle->userdata = userdata; }

  /** Returns a list of available monitors.
   *  @return A vector of monitor handles.
   *  @throws Exception if enumeration fails. */
  std::vector<LVKW_Monitor *> getMonitors() const {
    uint32_t count = 0;
    check(lvkw_ctx_getMonitors(m_ctx_handle, nullptr, &count), "Failed to get monitor count");
    std::vector<LVKW_Monitor *> monitors(count);
    check(lvkw_ctx_getMonitors(m_ctx_handle, monitors.data(), &count), "Failed to get monitors");
    return monitors;
  }

  /** Returns the available video modes for a specific monitor.
   *  @param monitor The monitor to query.
   *  @return A vector of video modes.
   *  @throws Exception if enumeration fails. */
  std::vector<LVKW_VideoMode> getMonitorModes(const LVKW_Monitor *monitor) const {
    uint32_t count = 0;
    check(lvkw_ctx_getMonitorModes(m_ctx_handle, monitor, nullptr, &count),
          "Failed to get mode count");
    std::vector<LVKW_VideoMode> modes(count);
    check(lvkw_ctx_getMonitorModes(m_ctx_handle, monitor, modes.data(), &count),
          "Failed to get modes");
    return modes;
  }

  /** Creates a new window within this context.
   *  @param create_info Window creation parameters.
   *  @return The created Window object.
   *  @throws Exception if creation fails. */
  Window createWindow(const LVKW_WindowCreateInfo &create_info) {
    LVKW_Window *handle;
    check(lvkw_ctx_createWindow(m_ctx_handle, &create_info, &handle),
          "Failed to create LVKW window");
    return Window(handle);
  }

  /** Retrieves a handle to a standard system cursor.
   *  @param shape The desired cursor shape.
   *  @return A raw handle to the standard cursor. */
  LVKW_Cursor *getStandardCursor(LVKW_CursorShape shape) const {
    return lvkw_ctx_getStandardCursor(m_ctx_handle, shape);
  }

  /** Creates a custom hardware cursor from pixels.
   *  @param create_info Configuration for the new cursor.
   *  @return The created RAII Cursor object. */
  Cursor createCursor(const LVKW_CursorCreateInfo &create_info) {
    LVKW_Cursor *handle;
    check(lvkw_ctx_createCursor(m_ctx_handle, &create_info, &handle),
          "Failed to create custom cursor");
    return Cursor(handle);
  }

  /** Retrieves a specific category of telemetry data.
   *  @tparam T The telemetry struct type (e.g., LVKW_EventTelemetry).
   *  @param reset If true, counters/watermarks will be reset after retrieval.
   *  @return The telemetry snapshot.
   *  @throws Exception if the query fails or if the category is unsupported. */
  template <typename T>
  T getTelemetry(bool reset = false) const {
    T data;
    check(lvkw_ctx_getTelemetry(m_ctx_handle, getCategory<T>(), &data, reset),
          "Failed to get telemetry");
    return data;
  }

#ifdef LVKW_ENABLE_CONTROLLER
  /** Opens a controller for use.
   *  @param id The controller ID from a connection event.
   *  @return The created Controller object.
   *  @throws Exception if creation fails. */
  Controller createController(LVKW_CtrlId id) {
    LVKW_Controller *handle;
    check(lvkw_ctrl_create(m_ctx_handle, id, &handle), "Failed to create LVKW controller");
    return Controller(handle);
  }
#endif

 private:
  LVKW_Context *m_ctx_handle = nullptr;

  template <typename T>
  static constexpr LVKW_TelemetryCategory getCategory() {
    if constexpr (std::is_same_v<T, LVKW_EventTelemetry>) return LVKW_TELEMETRY_CATEGORY_EVENTS;
    return LVKW_TELEMETRY_CATEGORY_NONE;
  }

  template <typename Visitor>
  static constexpr LVKW_EventType inferEventMask() {
    using V = std::remove_cvref_t<Visitor>;
    uint32_t mask = 0;
    if constexpr (std::invocable<V, WindowReadyEvent>) mask |= LVKW_EVENT_TYPE_WINDOW_READY;
    if constexpr (std::invocable<V, WindowCloseEvent>) mask |= LVKW_EVENT_TYPE_CLOSE_REQUESTED;
    if constexpr (std::invocable<V, WindowResizedEvent>) mask |= LVKW_EVENT_TYPE_WINDOW_RESIZED;
    if constexpr (std::invocable<V, WindowMaximizationEvent>)
      mask |= LVKW_EVENT_TYPE_WINDOW_MAXIMIZED;
    if constexpr (std::invocable<V, KeyboardEvent>) mask |= LVKW_EVENT_TYPE_KEY;
    if constexpr (std::invocable<V, MouseMotionEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_MOTION;
    if constexpr (std::invocable<V, MouseButtonEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_BUTTON;
    if constexpr (std::invocable<V, MouseScrollEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_SCROLL;
    if constexpr (std::invocable<V, IdleEvent>) mask |= LVKW_EVENT_TYPE_IDLE_NOTIFICATION;
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
};

inline bool operator==(const LVKW_PixelVec &lhs, const LVKW_PixelVec &rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator==(const LVKW_LogicalVec &lhs, const LVKW_LogicalVec &rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator==(const LVKW_Ratio &lhs, const LVKW_Ratio &rhs) noexcept {
  return lhs.numer == rhs.numer && lhs.denom == rhs.denom;
}

}  // namespace lvkw

#endif  // LVKW_HPP_INCLUDED
