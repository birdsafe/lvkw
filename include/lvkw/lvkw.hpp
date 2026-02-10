#ifndef LVKW_HPP_INCLUDED
#define LVKW_HPP_INCLUDED

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

template <typename T>
concept PartialEventVisitor =
    std::invocable<T, const LVKW_WindowReadyEvent &> || std::invocable<T, const LVKW_WindowCloseEvent &> ||
    std::invocable<T, const LVKW_WindowResizedEvent &> || std::invocable<T, const LVKW_KeyboardEvent &> ||
    std::invocable<T, const LVKW_MouseMotionEvent &> || std::invocable<T, const LVKW_MouseButtonEvent &> ||
    std::invocable<T, const LVKW_MouseScrollEvent &> || std::invocable<T, const LVKW_IdleEvent &>;

/** @brief Exception thrown by C++ wrappers on LVKW results indicating failure.
 */
class Exception : public std::runtime_error {
 public:
  /**
   * @brief Constructs an Exception object.
   * @param result The LVKW_Result bitmask associated with the failure.
   * @param message The error message string.
   */
  Exception(LVKW_Result result, const char *message) : std::runtime_error(message), m_result(result) {}
  LVKW_Result result() const { return m_result; }

  bool isWindowLost() const { return m_result & LVKW_RESULT_WINDOW_LOST_BIT; }
  bool isContextLost() const { return m_result & LVKW_RESULT_CONTEXT_LOST_BIT; }

 private:
  LVKW_Result m_result;
};

/**
 * @brief Checks the result of an LVKW operation and throws an exception if it
 * failed.
 * @param result The LVKW_Result bitmask to check.
 * @param message The message to use if an exception is thrown.
 * @throws Exception if the result has the LVKW_RESULT_ERROR_BIT set.
 */
inline void check(LVKW_Result result, const char *message) {
  if (result & LVKW_RESULT_ERROR_BIT) {
    throw Exception(result, message);
  }
}

/** @brief RAII wrapper for LVKW_Context. */
class Context {
 public:
  /**
   * @brief Default diagnosis callback that prints to std::cerr.
   */
  static void defaultDiagnosisCallback(const LVKW_DiagnosisInfo *info, void * /*user_data*/) {
    std::cerr << "LVKW Diagnosis: " << info->message << " (Code: " << (int)info->diagnosis << ")" << std::endl;
  }

  Context() {
    LVKW_ContextCreateInfo ci = {};
    ci.backend = LVKW_BACKEND_AUTO;
    check(lvkw_context_create(&ci, &m_ctx_handle), "Failed to create LVKW context");
  }

  explicit Context(const LVKW_ContextCreateInfo &create_info) {
    LVKW_ContextCreateInfo ci = create_info;
    if (!ci.diagnosis_callback) {
      ci.diagnosis_callback = defaultDiagnosisCallback;
    }
    check(lvkw_context_create(&ci, &m_ctx_handle), "Failed to create LVKW context");
  }

  ~Context() {
    if (m_ctx_handle) {
      lvkw_context_destroy(m_ctx_handle);
    }
  }

  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  Context(Context &&other) noexcept : m_ctx_handle(other.m_ctx_handle) { other.m_ctx_handle = nullptr; }

  Context &operator=(Context &&other) noexcept {
    if (this != &other) {
      if (m_ctx_handle) {
        lvkw_context_destroy(m_ctx_handle);
      }
      m_ctx_handle = other.m_ctx_handle;
      other.m_ctx_handle = nullptr;
    }
    return *this;
  }

  LVKW_Context *get() const { return m_ctx_handle; }

  /** @brief Gets required Vulkan instance extensions.
   * @return A vector of C-style strings, each representing a required Vulkan
   * instance extension.
   */
  std::vector<const char *> getVulkanInstanceExtensions() const {
    uint32_t count = 0;
    lvkw_context_getVulkanInstanceExtensions(m_ctx_handle, &count, nullptr);
    std::vector<const char *> extensions(count);
    lvkw_context_getVulkanInstanceExtensions(m_ctx_handle, &count, extensions.data());
    return extensions;
  }

  /** @brief Polls events using a callback. */
  template <typename F>
    requires std::invocable<F, const LVKW_Event &>
  void pollEvents(LVKW_EventType event_mask, F &&callback) {
    check(lvkw_context_pollEvents(
              m_ctx_handle, event_mask,
              [](const LVKW_Event *evt, void *userdata) {
                auto &cb = *static_cast<std::remove_reference_t<F> *>(userdata);
                cb(*evt);
              },
              &callback),
          "Failed to poll events");
  }

  /** @brief Polls events using a visitor. */
  template <typename F>
  void pollEvents(F &&f) {
    if constexpr (PartialEventVisitor<F>) {
      pollEvents(inferEventMask<std::remove_cvref_t<F>>(), [&](const LVKW_Event &evt) {
        switch (evt.type) {
          case LVKW_EVENT_TYPE_WINDOW_READY:
            if constexpr (std::invocable<F, const LVKW_WindowReadyEvent &>) f(evt.window_ready);
            break;
          case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
            if constexpr (std::invocable<F, const LVKW_WindowCloseEvent &>) f(evt.close_requested);
            break;
          case LVKW_EVENT_TYPE_WINDOW_RESIZED:
            if constexpr (std::invocable<F, const LVKW_WindowResizedEvent &>) f(evt.resized);
            break;
          case LVKW_EVENT_TYPE_KEY:
            if constexpr (std::invocable<F, const LVKW_KeyboardEvent &>) f(evt.key);
            break;
          case LVKW_EVENT_TYPE_MOUSE_MOTION:
            if constexpr (std::invocable<F, const LVKW_MouseMotionEvent &>) f(evt.mouse_motion);
            break;
          case LVKW_EVENT_TYPE_MOUSE_BUTTON:
            if constexpr (std::invocable<F, const LVKW_MouseButtonEvent &>) f(evt.mouse_button);
            break;
          case LVKW_EVENT_TYPE_MOUSE_SCROLL:
            if constexpr (std::invocable<F, const LVKW_MouseScrollEvent &>) f(evt.mouse_scroll);
            break;
          case LVKW_EVENT_TYPE_IDLE_NOTIFICATION:
            if constexpr (std::invocable<F, const LVKW_IdleEvent &>) f(evt.idle);
            break;
          default:
            break;
        }
      });
    }
    else if constexpr (std::invocable<F, const LVKW_Event &>) {
      pollEvents(LVKW_EVENT_TYPE_ALL, std::forward<F>(f));
    }
  }

  template <typename F1, typename F2, typename... Fs>
  void pollEvents(F1 &&f1, F2 &&f2, Fs &&...fs) {
    pollEvents(overloads{std::forward<F1>(f1), std::forward<F2>(f2), std::forward<Fs>(fs)...});
  }

  void setIdleTimeout(uint32_t timeout_ms) {
    check(lvkw_context_setIdleTimeout(m_ctx_handle, timeout_ms), "Failed to set idle timeout");
  }

  void *getUserData() const { return lvkw_context_getUserData(m_ctx_handle); }

 private:
  LVKW_Context *m_ctx_handle = nullptr;

  template <typename Visitor>
  static constexpr LVKW_EventType inferEventMask() {
    uint32_t mask = 0;
    if constexpr (std::invocable<Visitor, const LVKW_WindowReadyEvent &>) mask |= LVKW_EVENT_TYPE_WINDOW_READY;
    if constexpr (std::invocable<Visitor, const LVKW_WindowCloseEvent &>) mask |= LVKW_EVENT_TYPE_CLOSE_REQUESTED;
    if constexpr (std::invocable<Visitor, const LVKW_WindowResizedEvent &>) mask |= LVKW_EVENT_TYPE_WINDOW_RESIZED;
    if constexpr (std::invocable<Visitor, const LVKW_KeyboardEvent &>) mask |= LVKW_EVENT_TYPE_KEY;
    if constexpr (std::invocable<Visitor, const LVKW_MouseMotionEvent &>) mask |= LVKW_EVENT_TYPE_MOUSE_MOTION;
    if constexpr (std::invocable<Visitor, const LVKW_MouseButtonEvent &>) mask |= LVKW_EVENT_TYPE_MOUSE_BUTTON;
    if constexpr (std::invocable<Visitor, const LVKW_MouseScrollEvent &>) mask |= LVKW_EVENT_TYPE_MOUSE_SCROLL;
    if constexpr (std::invocable<Visitor, const LVKW_IdleEvent &>) mask |= LVKW_EVENT_TYPE_IDLE_NOTIFICATION;
    return static_cast<LVKW_EventType>(mask);
  }
};

/** @brief RAII wrapper for LVKW_Window. */
class Window {
 public:
  Window(Context &ctx, const LVKW_WindowCreateInfo &create_info) {
    check(lvkw_window_create(ctx.get(), &create_info, &m_window_handle), "Failed to create LVKW window");
  }

  ~Window() {
    if (m_window_handle) {
      lvkw_window_destroy(m_window_handle);
    }
  }

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  Window(Window &&other) noexcept : m_window_handle(other.m_window_handle) { other.m_window_handle = nullptr; }

  Window &operator=(Window &&other) noexcept {
    if (this != &other) {
      if (m_window_handle) {
        lvkw_window_destroy(m_window_handle);
      }
      m_window_handle = other.m_window_handle;
      other.m_window_handle = nullptr;
    }
    return *this;
  }

  LVKW_Window *get() const { return m_window_handle; }

  /** @brief Creates a Vulkan surface. */
  VkSurfaceKHR createVkSurface(VkInstance instance) const {
    VkSurfaceKHR surface;
    check(lvkw_window_createVkSurface(m_window_handle, instance, &surface), "Failed to create Vulkan surface");
    return surface;
  }

  /** @brief Gets the current framebuffer size. */
  LVKW_Size getFramebufferSize() const {
    LVKW_Size size;
    check(lvkw_window_getFramebufferSize(m_window_handle, &size), "Failed to get framebuffer size");
    return size;
  }

  void *getUserData() const { return lvkw_window_getUserData(m_window_handle); }

  void setFullscreen(bool enabled) {
    check(lvkw_window_setFullscreen(m_window_handle, enabled), "Failed to set fullscreen");
  }

  void setCursorMode(LVKW_CursorMode mode) {
    check(lvkw_window_setCursorMode(m_window_handle, mode), "Failed to set cursor mode");
  }

  void setCursorShape(LVKW_CursorShape shape) {
    check(lvkw_window_setCursorShape(m_window_handle, shape), "Failed to set cursor shape");
  }

  void requestFocus() { check(lvkw_window_requestFocus(m_window_handle), "Failed to request focus"); }

 private:
  LVKW_Window *m_window_handle = nullptr;
};

inline bool operator==(const LVKW_Size &lhs, const LVKW_Size &rhs) noexcept {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

}  // namespace lvkw

#endif  // LVKW_HPP_INCLUDED