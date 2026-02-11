#ifndef LVKW_HPP_INCLUDED
#define LVKW_HPP_INCLUDED

#include <chrono>
#include <concepts>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "lvkw.h"

namespace lvkw {

template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

/**
 * @brief A thin wrapper that bundles a specific event payload with its window metadata.
 * 
 * @note LIFETIME: Event objects and their underlying data are only valid 
 * for the duration of the callback. If you need to store event data for 
 * later, copy the underlying C structure.
 * 
 * @tparam T The C event structure type (e.g., LVKW_KeyboardEvent).
 */
template <typename T>
struct Event {
  LVKW_Window *window; /**< The window that generated this event. */
  const T &data;       /**< The event-specific payload. */

  /** @brief Provides pointer-like access to the event data. */
  const T *operator->() const { return &data; }
  /** @brief Implicitly converts to the underlying data structure. */
  operator const T &() const { return data; }
};

/** @brief Specific C++ event types that include window metadata. */
using WindowReadyEvent = Event<LVKW_WindowReadyEvent>;
using WindowCloseEvent = Event<LVKW_WindowCloseEvent>;
using WindowResizedEvent = Event<LVKW_WindowResizedEvent>;
using KeyboardEvent = Event<LVKW_KeyboardEvent>;
using MouseMotionEvent = Event<LVKW_MouseMotionEvent>;
using MouseButtonEvent = Event<LVKW_MouseButtonEvent>;
using MouseScrollEvent = Event<LVKW_MouseScrollEvent>;
using IdleEvent = Event<LVKW_IdleEvent>;
using MonitorConnectionEvent = Event<LVKW_MonitorConnectionEvent>;
using MonitorModeEvent = Event<LVKW_MonitorModeEvent>;
using TextInputEvent = Event<LVKW_TextInputEvent>;
using FocusEvent = Event<LVKW_FocusEvent>;

#ifdef LVKW_CONTROLLER_ENABLED
using ControllerConnectionEvent = Event<LVKW_CtrlConnectionEvent>;
#endif

template <typename T>
concept PartialEventVisitor =
    std::invocable<std::remove_cvref_t<T>, WindowReadyEvent> ||
    std::invocable<std::remove_cvref_t<T>, WindowCloseEvent> ||
    std::invocable<std::remove_cvref_t<T>, WindowResizedEvent> ||
    std::invocable<std::remove_cvref_t<T>, KeyboardEvent> || std::invocable<std::remove_cvref_t<T>, MouseMotionEvent> ||
    std::invocable<std::remove_cvref_t<T>, MouseButtonEvent> ||
    std::invocable<std::remove_cvref_t<T>, MouseScrollEvent> || std::invocable<std::remove_cvref_t<T>, IdleEvent> ||
    std::invocable<std::remove_cvref_t<T>, MonitorConnectionEvent> ||
    std::invocable<std::remove_cvref_t<T>, MonitorModeEvent> ||
    std::invocable<std::remove_cvref_t<T>, TextInputEvent> ||
    std::invocable<std::remove_cvref_t<T>, FocusEvent>
#ifdef LVKW_CONTROLLER_ENABLED
    || std::invocable<std::remove_cvref_t<T>, ControllerConnectionEvent>
#endif
    ;

/** @brief Thrown when an LVKW operation fails. */
class Exception : public std::runtime_error {
 public:
  /**
   * @brief Creates a new Exception.
   * @param status The LVKW_Status that triggered the failure.
   * @param message A description of what went wrong.
   */
  Exception(LVKW_Status status, const char *message) : std::runtime_error(message), m_status(status) {}
  /** @brief Returns the raw status code that caused this exception. */
  LVKW_Status status() const { return m_status; }

  /** @brief Returns true if the operation failed. */
  bool isError() const { return m_status != LVKW_SUCCESS; }

 private:
  LVKW_Status m_status;
};

/** @brief Thrown when a window handle is lost. */
class WindowLostException : public Exception {
 public:
  WindowLostException(const char *message) : Exception(LVKW_ERROR_WINDOW_LOST, message) {}
};

/** @brief Thrown when the entire library context is lost. */
class ContextLostException : public Exception {
 public:
  ContextLostException(const char *message) : Exception(LVKW_ERROR_CONTEXT_LOST, message) {}
};

/**
 * @brief Checks a status code and throws if it indicates an error.
 * @param status The status code to check.
 * @param message The message to include in the exception.
 * @throws Exception or a more specific subclass if status is not LVKW_SUCCESS.
 */
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

/** @brief A handy RAII wrapper for an LVKW_Window. */
class Window {
 public:
  /** @brief Destroys the window and cleans up. */
  ~Window() {
    if (m_window_handle) {
      lvkw_wnd_destroy(m_window_handle);
    }
  }

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  /** @brief Moves a window from another instance. */
  Window(Window &&other) noexcept : m_window_handle(other.m_window_handle) { other.m_window_handle = nullptr; }

  /** @brief Transfers ownership of a window handle. */
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

  /** @brief Gives you the underlying C window handle.
   *  @return The raw LVKW_Window handle. */
  LVKW_Window *get() const { return m_window_handle; }

  /** @brief Creates a Vulkan surface for this specific window.
   * 
   *  PRECONDITION: The window must be ready (check isReady()).
   * 
   *  NOTE: The user is responsible for destroying the created surface via
   *  vkDestroySurfaceKHR(). The library does NOT automatically destroy the
   *  surface when the Window object is destroyed.
   * 
   *  @param instance The Vulkan instance.
   *  @return The created VkSurfaceKHR.
   *  @throws Exception if surface creation fails. */
  VkSurfaceKHR createVkSurface(VkInstance instance) const {
    VkSurfaceKHR surface;
    check(lvkw_wnd_createVkSurface(m_window_handle, instance, &surface), "Failed to create Vulkan surface");
    return surface;
  }

  /** @brief Returns the current geometry (logical and physical size) of the window.
   *  @return The window geometry.
   *  @throws Exception if the query fails. */
  LVKW_WindowGeometry getGeometry() const {
    LVKW_WindowGeometry geometry;
    check(lvkw_wnd_getGeometry(m_window_handle, &geometry), "Failed to get window geometry");
    return geometry;
  }

  /** @brief Returns true if the window handle is lost. 
   * 
   *  A lost window must be destroyed and recreated. 
   */
  bool isLost() const { return m_window_handle->flags & LVKW_WND_STATE_LOST; }

  /** @brief Returns true if the window is ready for rendering. 
   * 
   *  Do not call createVkSurface() or getGeometry() until this 
   *  returns true.
   */
  bool isReady() const { return m_window_handle->flags & LVKW_WND_STATE_READY; }

  /** @brief Returns your custom window-specific user data.
   *  @return The userdata pointer. */
  void *getUserData() const { return m_window_handle->userdata; }

  /** @brief Sets your custom window-specific user data.
   *  @param userdata The new userdata pointer. */
  void setUserData(void *userdata) { m_window_handle->userdata = userdata; }

  /** @brief Updates specific attributes of this window.
   *  @param field_mask A bitmask of LVKW_WindowAttributesField.
   *  @param attrs The new attribute values.
   *  @throws Exception if the update fails. */
  void update(uint32_t field_mask, const LVKW_WindowAttributes &attrs) {
    check(lvkw_wnd_update(m_window_handle, field_mask, &attrs), "Failed to update window attributes");
  }

  /** @brief Sets the title of the window (UTF-8).
   *  @param title The new title. */
  void setTitle(const char *title) {
    LVKW_WindowAttributes attrs = {};
    attrs.title = title;
    update(LVKW_WND_ATTR_TITLE, attrs);
  }

  /** @brief Sets the logical size of the window.
   *  @param size The new size. */
  void setSize(LVKW_LogicalVec size) {
    LVKW_WindowAttributes attrs = {};
    attrs.logicalSize = size;
    update(LVKW_WND_ATTR_LOGICAL_SIZE, attrs);
  }

  /** @brief Switches the window in or out of fullscreen mode.
   *  @param enabled True to enable fullscreen. */
  void setFullscreen(bool enabled) {
    LVKW_WindowAttributes attrs = {};
    attrs.fullscreen = enabled;
    update(LVKW_WND_ATTR_FULLSCREEN, attrs);
  }

  /** @brief Sets how the cursor should behave (e.g. normal or locked).
   *  @param mode The new cursor mode. */
  void setCursorMode(LVKW_CursorMode mode) {
    LVKW_WindowAttributes attrs = {};
    attrs.cursor_mode = mode;
    update(LVKW_WND_ATTR_CURSOR_MODE, attrs);
  }

  /** @brief Changes the current appearance of the cursor.
   *  @param shape The new cursor shape. */
  void setCursorShape(LVKW_CursorShape shape) {
    LVKW_WindowAttributes attrs = {};
    attrs.cursor_shape = shape;
    update(LVKW_WND_ATTR_CURSOR_SHAPE, attrs);
  }

  /** @brief Asks the system to give this window input focus.
   *  @throws Exception if the request fails. */
  void requestFocus() { check(lvkw_wnd_requestFocus(m_window_handle), "Failed to request focus"); }

  /** @brief Sets the system clipboard content to a UTF-8 string.
   *  @param text The null-terminated UTF-8 string to copy.
   *  @throws Exception if the operation fails. */
  void setClipboardText(const char *text) {
    check(lvkw_wnd_setClipboardText(m_window_handle, text), "Failed to set clipboard text");
  }

  /** @brief Retrieves the current system clipboard content as a UTF-8 string.
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

 private:
  LVKW_Window *m_window_handle = nullptr;

  /** @brief Creates a new window within your library context. */
  explicit Window(LVKW_Window *handle) : m_window_handle(handle) {}

  friend class Context;
};

#ifdef LVKW_CONTROLLER_ENABLED
/** @brief A handy RAII wrapper for an LVKW_Controller. */
class Controller {
 public:
  /** @brief Destroys the controller and cleans up. */
  ~Controller() {
    if (m_controller_handle) {
      lvkw_ctrl_destroy(m_controller_handle);
    }
  }

  Controller(const Controller &) = delete;
  Controller &operator=(const Controller &) = delete;

  /** @brief Moves a controller from another instance. */
  Controller(Controller &&other) noexcept : m_controller_handle(other.m_controller_handle) {
    other.m_controller_handle = nullptr;
  }

  /** @brief Transfers ownership of a controller handle. */
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

  /** @brief Gives you the underlying C controller handle.
   *  @return The raw LVKW_Controller handle. */
  LVKW_Controller *get() const { return m_controller_handle; }

  /** @brief Provides pointer-like access to the controller structure. */
  const LVKW_Controller *operator->() const { return m_controller_handle; }

  /** @brief Returns detailed information about this controller.
   *  @return The controller info.
   *  @throws Exception if the query fails. */
  LVKW_CtrlInfo getInfo() const {
    LVKW_CtrlInfo info;
    check(lvkw_ctrl_getInfo(m_controller_handle, &info), "Failed to get controller info");
    return info;
  }

  /** @brief Returns true if the controller is lost (unplugged). */
  bool isLost() const { return m_controller_handle->flags & LVKW_WND_STATE_LOST; }

 private:
  LVKW_Controller *m_controller_handle = nullptr;

  explicit Controller(LVKW_Controller *handle) : m_controller_handle(handle) {}

  friend class Context;
};
#endif

/** @brief A handy RAII wrapper for the LVKW_Context. 
 * 
 *  @note THREAD SAFETY: All calls to a context and its associated windows 
 *  must originate from the thread that created the context.
 */
class Context {
 public:
  /**
   * @brief A default diagnosis logger that prints straight to std::cerr.
   */
  static void defaultDiagnosisCallback(const LVKW_DiagnosisInfo *info, void * /*userdata*/) {
    std::cerr << "LVKW Diagnosis: " << info->message << " (Code: " << (int)info->diagnosis << ")" << std::endl;
  }

  /** @brief Creates a context with default settings (AUTO backend).
   *  @throws Exception if creation fails. */
  Context() {
    LVKW_ContextCreateInfo ci = {};
    ci.backend = LVKW_BACKEND_AUTO;
    ci.attributes.diagnosis_cb = defaultDiagnosisCallback;
    check(lvkw_createContext(&ci, &m_ctx_handle), "Failed to create LVKW context");
  }

  /** @brief Creates a context using your specific creation options.
   *  @param create_info Creation parameters.
   *  @throws Exception if creation fails. */
  explicit Context(const LVKW_ContextCreateInfo &create_info) {
    LVKW_ContextCreateInfo ci = create_info;
    if (!ci.attributes.diagnosis_cb) {
      ci.attributes.diagnosis_cb = defaultDiagnosisCallback;
    }
    check(lvkw_createContext(&ci, &m_ctx_handle), "Failed to create LVKW context");
  }

  /** @brief Cleans up and destroys the context. */
  ~Context() {
    if (m_ctx_handle) {
      lvkw_ctx_destroy(m_ctx_handle);
    }
  }

  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  /** @brief Moves a context from another instance.
   *  @param other The context to move from. */
  Context(Context &&other) noexcept : m_ctx_handle(other.m_ctx_handle) { other.m_ctx_handle = nullptr; }

  /** @brief Transfers ownership of a context handle.
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

  /** @brief Gives you the underlying C context handle.
   *  @return The raw LVKW_Context handle. */
  LVKW_Context *get() const { return m_ctx_handle; }

  /** @brief Returns a list of Vulkan extensions required for this context.
   *  @return A vector of extension names. */
  std::vector<const char *> getVkExtensions() const {
    uint32_t count = 0;
    const char *const *extensions = lvkw_ctx_getVkExtensions(m_ctx_handle, &count);
    if (!extensions || count == 0) return {};
    return std::vector<const char *>(extensions, extensions + count);
  }

  /** @brief Polls for events and passes them to your callback function.
   * 
   *  @note LIFETIME: The event data passed to the callback is only valid 
   *  for the duration of the callback.
   * 
   *  @tparam F The callback type.
   *  @param event_mask Bitmask of events to poll.
   *  @param callback Function to call for each event.
   *  @throws Exception if polling fails. */
  template <typename F>
    requires std::invocable<F, const LVKW_Event &>
  void pollEvents(LVKW_EventType event_mask, F &&callback) {
    check(lvkw_ctx_pollEvents(
              m_ctx_handle, event_mask,
              [](const LVKW_Event *evt, void *userdata) {
                auto &cb = *static_cast<std::remove_reference_t<F> *>(userdata);
                cb(*evt);
              },
              &callback),
          "Failed to poll events");
  }

  /** @brief Waits for events with a timeout, then sends them to your callback.
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
    requires std::invocable<F, const LVKW_Event &>
  void waitEvents(Duration timeout, LVKW_EventType event_mask, F &&callback) {
    auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
    check(lvkw_ctx_waitEvents(
              m_ctx_handle, (uint32_t)timeout_ms, event_mask,
              [](const LVKW_Event *evt, void *userdata) {
                auto &cb = *static_cast<std::remove_reference_t<F> *>(userdata);
                cb(*evt);
              },
              &callback),
          "Failed to wait for events");
  }

  /** @brief Polls for events using a visitor or a generic lambda.
   *  @tparam F The visitor or lambda type.
   *  @param f The event handler. */
  template <typename F>
  void pollEvents(F &&f) {
    using F_raw = std::remove_cvref_t<F>;
    if constexpr (PartialEventVisitor<F_raw>) {
      pollEvents(inferEventMask<F_raw>(), [&](const LVKW_Event &evt) {
        switch (evt.type) {
          case LVKW_EVENT_TYPE_WINDOW_READY:
            if constexpr (std::invocable<F_raw, WindowReadyEvent>) f(WindowReadyEvent{evt.window, evt.window_ready});
            break;
          case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
            if constexpr (std::invocable<F_raw, WindowCloseEvent>) f(WindowCloseEvent{evt.window, evt.close_requested});
            break;
          case LVKW_EVENT_TYPE_WINDOW_RESIZED:
            if constexpr (std::invocable<F_raw, WindowResizedEvent>) f(WindowResizedEvent{evt.window, evt.resized});
            break;
          case LVKW_EVENT_TYPE_KEY:
            if constexpr (std::invocable<F_raw, KeyboardEvent>) f(KeyboardEvent{evt.window, evt.key});
            break;
          case LVKW_EVENT_TYPE_MOUSE_MOTION:
            if constexpr (std::invocable<F_raw, MouseMotionEvent>) f(MouseMotionEvent{evt.window, evt.mouse_motion});
            break;
          case LVKW_EVENT_TYPE_MOUSE_BUTTON:
            if constexpr (std::invocable<F_raw, MouseButtonEvent>) f(MouseButtonEvent{evt.window, evt.mouse_button});
            break;
          case LVKW_EVENT_TYPE_MOUSE_SCROLL:
            if constexpr (std::invocable<F_raw, MouseScrollEvent>) f(MouseScrollEvent{evt.window, evt.mouse_scroll});
            break;
          case LVKW_EVENT_TYPE_IDLE_NOTIFICATION:
            if constexpr (std::invocable<F_raw, IdleEvent>) f(IdleEvent{evt.window, evt.idle});
            break;
          case LVKW_EVENT_TYPE_MONITOR_CONNECTION:
            if constexpr (std::invocable<F_raw, MonitorConnectionEvent>) f(MonitorConnectionEvent{evt.window, evt.monitor_connection});
            break;
          case LVKW_EVENT_TYPE_MONITOR_MODE:
            if constexpr (std::invocable<F_raw, MonitorModeEvent>) f(MonitorModeEvent{evt.window, evt.monitor_mode});
            break;
          case LVKW_EVENT_TYPE_TEXT_INPUT:
            if constexpr (std::invocable<F_raw, TextInputEvent>) f(TextInputEvent{evt.window, evt.text_input});
            break;
          case LVKW_EVENT_TYPE_FOCUS:
            if constexpr (std::invocable<F_raw, FocusEvent>) f(FocusEvent{evt.window, evt.focus});
            break;
#ifdef LVKW_CONTROLLER_ENABLED
          case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION:
            if constexpr (std::invocable<F_raw, ControllerConnectionEvent>) f(ControllerConnectionEvent{evt.window, evt.controller_connection});
            break;
#endif
          default:
            break;
        }
      });
    }
    else if constexpr (std::invocable<F_raw, const LVKW_Event &>) {
      pollEvents(LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
    }
  }

  /** @brief Waits for events with a timeout, then dispatches them to a visitor.
   *  @tparam F The visitor or lambda type.
   *  @tparam Duration A std::chrono duration type.
   *  @param timeout Maximum time to wait.
   *  @param f The event handler. */
  template <typename F, typename Duration>
  void waitEvents(Duration timeout, F &&f) {
    using F_raw = std::remove_cvref_t<F>;
    if constexpr (PartialEventVisitor<F_raw>) {
      waitEvents(timeout, inferEventMask<F_raw>(), [&](const LVKW_Event &evt) {
        switch (evt.type) {
          case LVKW_EVENT_TYPE_WINDOW_READY:
            if constexpr (std::invocable<F_raw, WindowReadyEvent>) f(WindowReadyEvent{evt.window, evt.window_ready});
            break;
          case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
            if constexpr (std::invocable<F_raw, WindowCloseEvent>) f(WindowCloseEvent{evt.window, evt.close_requested});
            break;
          case LVKW_EVENT_TYPE_WINDOW_RESIZED:
            if constexpr (std::invocable<F_raw, WindowResizedEvent>) f(WindowResizedEvent{evt.window, evt.resized});
            break;
          case LVKW_EVENT_TYPE_KEY:
            if constexpr (std::invocable<F_raw, KeyboardEvent>) f(KeyboardEvent{evt.window, evt.key});
            break;
          case LVKW_EVENT_TYPE_MOUSE_MOTION:
            if constexpr (std::invocable<F_raw, MouseMotionEvent>) f(MouseMotionEvent{evt.window, evt.mouse_motion});
            break;
          case LVKW_EVENT_TYPE_MOUSE_BUTTON:
            if constexpr (std::invocable<F_raw, MouseButtonEvent>) f(MouseButtonEvent{evt.window, evt.mouse_button});
            break;
          case LVKW_EVENT_TYPE_MOUSE_SCROLL:
            if constexpr (std::invocable<F_raw, MouseScrollEvent>) f(MouseScrollEvent{evt.window, evt.mouse_scroll});
            break;
          case LVKW_EVENT_TYPE_IDLE_NOTIFICATION:
            if constexpr (std::invocable<F_raw, IdleEvent>) f(IdleEvent{evt.window, evt.idle});
            break;
          case LVKW_EVENT_TYPE_MONITOR_CONNECTION:
            if constexpr (std::invocable<F_raw, MonitorConnectionEvent>) f(MonitorConnectionEvent{evt.window, evt.monitor_connection});
            break;
          case LVKW_EVENT_TYPE_MONITOR_MODE:
            if constexpr (std::invocable<F_raw, MonitorModeEvent>) f(MonitorModeEvent{evt.window, evt.monitor_mode});
            break;
          case LVKW_EVENT_TYPE_TEXT_INPUT:
            if constexpr (std::invocable<F_raw, TextInputEvent>) f(TextInputEvent{evt.window, evt.text_input});
            break;
          case LVKW_EVENT_TYPE_FOCUS:
            if constexpr (std::invocable<F_raw, FocusEvent>) f(FocusEvent{evt.window, evt.focus});
            break;
#ifdef LVKW_CONTROLLER_ENABLED
          case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION:
            if constexpr (std::invocable<F_raw, ControllerConnectionEvent>) f(ControllerConnectionEvent{evt.window, evt.controller_connection});
            break;
#endif
          default:
            break;
        }
      });
    }
    else if constexpr (std::invocable<F_raw, const LVKW_Event &>) {
      waitEvents(timeout, LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
    }
  }

  /** @brief Dispatches events using multiple function overloads at once.
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

  /** @brief Configures how long to wait before sending an idle notification.
   *  @param timeout_ms The threshold in milliseconds. */
  void setIdleTimeout(uint32_t timeout_ms) {
    LVKW_ContextAttributes attrs = {};
    attrs.idle_timeout_ms = timeout_ms;
    check(lvkw_ctx_update(m_ctx_handle, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs), "Failed to update context attributes");
  }

  /** @brief Prevents the system from going idle or sleeping.
   *  @param enabled True to inhibit idle. */
  void setIdleInhibition(bool enabled) {
    LVKW_ContextAttributes attrs = {};
    attrs.inhibit_idle = enabled;
    check(lvkw_ctx_update(m_ctx_handle, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs), "Failed to update context attributes");
  }

  /** @brief Sets the diagnosis callback for this context.
   *  @param callback The diagnosis callback function.
   *  @param userdata User data for the callback. */
  void setDiagnosisCallback(LVKW_DiagnosisCallback callback, void *userdata) {
    check(lvkw_ctx_setDiagnosisCallback(m_ctx_handle, callback, userdata), "Failed to set diagnosis callback");
  }

  /** @brief Returns true if the context handle is lost. 
   * 
   *  A lost context must be destroyed. All associated windows are also lost.
   */
  bool isLost() const { return m_ctx_handle->flags & LVKW_CTX_STATE_LOST; }

  /** @brief Returns your custom global user data pointer.
   *  @return The global userdata pointer. */
  void *getUserData() const { return m_ctx_handle->userdata; }

  /** @brief Sets your custom global user data pointer.
   *  @param userdata The new global userdata pointer. */
  void setUserData(void *userdata) { m_ctx_handle->userdata = userdata; }

  /** @brief Returns a list of available monitors.
   *  @return A vector of monitor info snapshots.
   *  @throws Exception if enumeration fails. */
  std::vector<LVKW_MonitorInfo> getMonitors() const {
    uint32_t count = 0;
    check(lvkw_ctx_getMonitors(m_ctx_handle, nullptr, &count), "Failed to get monitor count");
    std::vector<LVKW_MonitorInfo> monitors(count);
    check(lvkw_ctx_getMonitors(m_ctx_handle, monitors.data(), &count), "Failed to get monitors");
    return monitors;
  }

  /** @brief Returns the available video modes for a specific monitor.
   *  @param monitor The monitor to query.
   *  @return A vector of video modes.
   *  @throws Exception if enumeration fails. */
  std::vector<LVKW_VideoMode> getMonitorModes(LVKW_MonitorId monitor) const {
    uint32_t count = 0;
    check(lvkw_ctx_getMonitorModes(m_ctx_handle, monitor, nullptr, &count), "Failed to get mode count");
    std::vector<LVKW_VideoMode> modes(count);
    check(lvkw_ctx_getMonitorModes(m_ctx_handle, monitor, modes.data(), &count), "Failed to get modes");
    return modes;
  }

  /** @brief Creates a new window within this context.
   *  @param create_info Window creation parameters.
   *  @return The created Window object.
   *  @throws Exception if creation fails. */
  Window createWindow(const LVKW_WindowCreateInfo &create_info) {
    LVKW_Window *handle;
    check(lvkw_ctx_createWindow(m_ctx_handle, &create_info, &handle), "Failed to create LVKW window");
    return Window(handle);
  }

#ifdef LVKW_CONTROLLER_ENABLED
  /** @brief Opens a controller for use.
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

  template <typename Visitor>
  static constexpr LVKW_EventType inferEventMask() {
    using V = std::remove_cvref_t<Visitor>;
    uint32_t mask = 0;
    if constexpr (std::invocable<V, WindowReadyEvent>) mask |= LVKW_EVENT_TYPE_WINDOW_READY;
    if constexpr (std::invocable<V, WindowCloseEvent>) mask |= LVKW_EVENT_TYPE_CLOSE_REQUESTED;
    if constexpr (std::invocable<V, WindowResizedEvent>) mask |= LVKW_EVENT_TYPE_WINDOW_RESIZED;
    if constexpr (std::invocable<V, KeyboardEvent>) mask |= LVKW_EVENT_TYPE_KEY;
    if constexpr (std::invocable<V, MouseMotionEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_MOTION;
    if constexpr (std::invocable<V, MouseButtonEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_BUTTON;
    if constexpr (std::invocable<V, MouseScrollEvent>) mask |= LVKW_EVENT_TYPE_MOUSE_SCROLL;
    if constexpr (std::invocable<V, IdleEvent>) mask |= LVKW_EVENT_TYPE_IDLE_NOTIFICATION;
    if constexpr (std::invocable<V, MonitorConnectionEvent>) mask |= LVKW_EVENT_TYPE_MONITOR_CONNECTION;
    if constexpr (std::invocable<V, MonitorModeEvent>) mask |= LVKW_EVENT_TYPE_MONITOR_MODE;
    if constexpr (std::invocable<V, TextInputEvent>) mask |= LVKW_EVENT_TYPE_TEXT_INPUT;
    if constexpr (std::invocable<V, FocusEvent>) mask |= LVKW_EVENT_TYPE_FOCUS;
#ifdef LVKW_CONTROLLER_ENABLED
    if constexpr (std::invocable<V, ControllerConnectionEvent>) mask |= LVKW_EVENT_TYPE_CONTROLLER_CONNECTION;
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

}  // namespace lvkw

#endif  // LVKW_HPP_INCLUDED