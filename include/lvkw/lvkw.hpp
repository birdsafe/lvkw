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

template <typename T>
concept PartialEventVisitor =
    std::invocable<std::remove_cvref_t<T>, WindowReadyEvent> ||
    std::invocable<std::remove_cvref_t<T>, WindowCloseEvent> ||
    std::invocable<std::remove_cvref_t<T>, WindowResizedEvent> ||
    std::invocable<std::remove_cvref_t<T>, KeyboardEvent> ||
    std::invocable<std::remove_cvref_t<T>, MouseMotionEvent> ||
    std::invocable<std::remove_cvref_t<T>, MouseButtonEvent> ||
    std::invocable<std::remove_cvref_t<T>, MouseScrollEvent> ||
    std::invocable<std::remove_cvref_t<T>, IdleEvent>;

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

  /** @brief Gives you the underlying C window handle. */
  LVKW_Window *get() const { return m_window_handle; }

  /** @brief Creates a Vulkan surface for this specific window. */
  VkSurfaceKHR createVkSurface(VkInstance instance) const {
    VkSurfaceKHR surface;
    check(lvkw_wnd_createVkSurface(m_window_handle, instance, &surface), "Failed to create Vulkan surface");
    return surface;
  }

  /** @brief Returns the current dimensions of the window's framebuffer. */
  LVKW_Size getFramebufferSize() const {
    LVKW_Size size;
    check(lvkw_wnd_getFramebufferSize(m_window_handle, &size), "Failed to get framebuffer size");
    return size;
  }

  /** @brief Returns your custom window-specific user data. */
  void *getUserData() const { return m_window_handle->userdata; }

  /** @brief Sets your custom window-specific user data. */
  void setUserData(void *userdata) { m_window_handle->userdata = userdata; }

  /** @brief Updates specific attributes of this window. */
  void updateAttributes(uint32_t field_mask, const LVKW_WindowAttributes &attrs) {
    check(lvkw_wnd_updateAttributes(m_window_handle, field_mask, &attrs), "Failed to update window attributes");
  }

  /** @brief Sets the title of the window (UTF-8). */
  void setTitle(const char *title) {
    LVKW_WindowAttributes attrs = {};
    attrs.title = title;
    updateAttributes(LVKW_WND_ATTR_TITLE, attrs);
  }

  /** @brief Sets the logical size of the window. */
  void setSize(LVKW_Size size) {
    LVKW_WindowAttributes attrs = {};
    attrs.size = size;
    updateAttributes(LVKW_WND_ATTR_SIZE, attrs);
  }

  /** @brief Switches the window in or out of fullscreen mode. */
  void setFullscreen(bool enabled) {
    check(lvkw_wnd_setFullscreen(m_window_handle, enabled), "Failed to set fullscreen");
  }

  /** @brief Sets how the cursor should behave (e.g. normal or locked). */
  void setCursorMode(LVKW_CursorMode mode) {
    check(lvkw_wnd_setCursorMode(m_window_handle, mode), "Failed to set cursor mode");
  }

  /** @brief Changes the current appearance of the cursor. */
  void setCursorShape(LVKW_CursorShape shape) {
    check(lvkw_wnd_setCursorShape(m_window_handle, shape), "Failed to set cursor shape");
  }

  /** @brief Asks the system to give this window input focus. */
  void requestFocus() { check(lvkw_wnd_requestFocus(m_window_handle), "Failed to request focus"); }

 private:
  LVKW_Window *m_window_handle = nullptr;

  /** @brief Creates a new window within your library context. */
  explicit Window(LVKW_Window *handle) : m_window_handle(handle) {}

  friend class Context;
};

/** @brief A handy RAII wrapper for the LVKW_Context. */
class Context {
 public:
  /**
   * @brief A default diagnosis logger that prints straight to std::cerr.
   */
  static void defaultDiagnosisCallback(const LVKW_DiagnosisInfo *info, void * /*userdata*/) {
    std::cerr << "LVKW Diagnosis: " << info->message << " (Code: " << (int)info->diagnosis << ")" << std::endl;
  }

  /** @brief Creates a context with default settings (AUTO backend). */
  Context() {
    LVKW_ContextCreateInfo ci = {};
    ci.backend = LVKW_BACKEND_AUTO;
    check(lvkw_createContext(&ci, &m_ctx_handle), "Failed to create LVKW context");
  }

  /** @brief Creates a context using your specific creation options. */
  explicit Context(const LVKW_ContextCreateInfo &create_info) {
    LVKW_ContextCreateInfo ci = create_info;
    if (!ci.diagnosis_cb) {
      ci.diagnosis_cb = defaultDiagnosisCallback;
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

  /** @brief Moves a context from another instance. */
  Context(Context &&other) noexcept : m_ctx_handle(other.m_ctx_handle) { other.m_ctx_handle = nullptr; }

  /** @brief Transfers ownership of a context handle. */
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

  /** @brief Gives you the underlying C context handle. */
  LVKW_Context *get() const { return m_ctx_handle; }

  /** @brief Returns a list of Vulkan extensions required for this context. */
  std::vector<const char *> getVulkanInstanceExtensions() const {
    uint32_t count = 0;
    lvkw_ctx_getVkExtensions(m_ctx_handle, &count, nullptr);
    std::vector<const char *> extensions(count);
    lvkw_ctx_getVkExtensions(m_ctx_handle, &count, extensions.data());
    return extensions;
  }

  /** @brief Polls for events and passes them to your callback function. */
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

  /** @brief Waits for events with a timeout, then sends them to your callback. */
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

  /** @brief Polls for events using a visitor or a generic lambda. */
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
          default:
            break;
        }
      });
    }
    else if constexpr (std::invocable<F_raw, const LVKW_Event &>) {
      pollEvents(LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
    }
  }

  /** @brief Waits for events with a timeout, then dispatches them to a visitor. */
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
          default:
            break;
        }
      });
    }
    else if constexpr (std::invocable<F_raw, const LVKW_Event &>) {
      waitEvents(timeout, LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
    }
  }

  /** @brief Dispatches events using multiple function overloads at once. */
  template <typename F1, typename F2, typename... Fs>
  void pollEvents(F1 &&f1, F2 &&f2, Fs &&...fs) {
    pollEvents(overloads{std::forward<F1>(f1), std::forward<F2>(f2), std::forward<Fs>(fs)...});
  }

  /** @brief Configures how long to wait before sending an idle notification. */
  void setIdleTimeout(uint32_t timeout_ms) {
    check(lvkw_ctx_setIdleTimeout(m_ctx_handle, timeout_ms), "Failed to set idle timeout");
  }

  /** @brief Returns your custom global user data pointer. */
  void *getUserData() const { return m_ctx_handle->userdata; }

  /** @brief Sets your custom global user data pointer. */
  void setUserData(void *userdata) { m_ctx_handle->userdata = userdata; }

  /** @brief Creates a new window within this context. */
  Window createWindow(const LVKW_WindowCreateInfo &create_info) {
    LVKW_Window *handle;
    check(lvkw_ctx_createWindow(m_ctx_handle, &create_info, &handle), "Failed to create LVKW window");
    return Window(handle);
  }

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
    return static_cast<LVKW_EventType>(mask);
  }
};

inline bool operator==(const LVKW_Size &lhs, const LVKW_Size &rhs) noexcept {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

}  // namespace lvkw

#endif  // LVKW_HPP_INCLUDED