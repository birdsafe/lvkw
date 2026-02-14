// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_HPP_INCLUDED
#define LVKW_HPP_INCLUDED

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "lvkw.h"

namespace lvkw {

class Context;
class Cursor;
class Window;

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
typedef Event<LVKW_WindowReadyEvent> WindowReadyEvent;
typedef Event<LVKW_WindowCloseEvent> WindowCloseEvent;
typedef Event<LVKW_WindowResizedEvent> WindowResizedEvent;
typedef Event<LVKW_WindowMaximizationEvent> WindowMaximizationEvent;
typedef Event<LVKW_KeyboardEvent> KeyboardEvent;
typedef Event<LVKW_MouseMotionEvent> MouseMotionEvent;
typedef Event<LVKW_MouseButtonEvent> MouseButtonEvent;
typedef Event<LVKW_MouseScrollEvent> MouseScrollEvent;
typedef Event<LVKW_IdleEvent> IdleEvent;
typedef Event<LVKW_MonitorConnectionEvent> MonitorConnectionEvent;
typedef Event<LVKW_MonitorModeEvent> MonitorModeEvent;
typedef Event<LVKW_TextInputEvent> TextInputEvent;
typedef Event<LVKW_TextCompositionEvent> TextCompositionEvent;
typedef Event<LVKW_FocusEvent> FocusEvent;

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
  DndFeedback feedback() const;

  /** Shorthand for feedback().action(). */
  LVKW_DndAction &action() const;
  /** Shorthand for feedback().sessionUserdata(). */
  void *&sessionUserdata() const;
};

typedef Event<LVKW_DndLeaveEvent> DndLeaveEvent;
typedef Event<LVKW_DndDropEvent> DndDropEvent;

#ifdef LVKW_ENABLE_CONTROLLER
typedef Event<LVKW_CtrlConnectionEvent> ControllerConnectionEvent;
#endif

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
void check(LVKW_Status status, const char *message);

/** A handy RAII wrapper for an LVKW_Cursor. */
class Cursor {
 public:
  /** Destroys the cursor if it was created by the user. */
  ~Cursor();

  Cursor(const Cursor &) = delete;
  Cursor &operator=(const Cursor &) = delete;

  /** Moves a cursor from another instance. */
  Cursor(Cursor &&other) noexcept;

  /** Transfers ownership of a cursor handle. */
  Cursor &operator=(Cursor &&other) noexcept;

  /** Gives you the underlying C cursor handle.
   *  @return The raw LVKW_Cursor handle. */
  LVKW_Cursor *get() const;

 private:
  LVKW_Cursor *m_cursor_handle = nullptr;

  Cursor(LVKW_Cursor *handle) : m_cursor_handle(handle) {}

  bool is_owned() const;
  friend class Context;
};

/** A handy RAII wrapper for an LVKW_Window. */
class Window {
 public:
  /** Destroys the window and cleans up. */
  ~Window();

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  /** Moves a window from another instance. */
  Window(Window &&other) noexcept;

  /** Transfers ownership of a window handle. */
  Window &operator=(Window &&other) noexcept;

  /** Gives you the underlying C window handle.
   *  @return The raw LVKW_Window handle. */
  LVKW_Window *get() const;

  /** Creates a Vulkan surface for this specific window.
   *
   *  PRECONDITION: The window must be ready (check isReady()).
   *
   *  @param instance The Vulkan instance.
   *  @return The created VkSurfaceKHR.
   *  @throws Exception if surface creation fails. */
  VkSurfaceKHR createVkSurface(VkInstance instance) const;

  /** Returns the current geometry (logical and physical size) of the window.
   *  @return The window geometry.
   *  @throws Exception if the query fails. */
  LVKW_WindowGeometry getGeometry() const;

  /** Returns true if the window handle is lost.
   *
   *  A lost window must be destroyed and recreated.
   */
  bool isLost() const;

  /** Returns true if the window is ready for rendering.
   *
   *  Do not call createVkSurface() or getGeometry() until this
   *  returns true.
   */
  bool isReady() const;

  /** Returns true if the window currently has input focus. */
  bool isFocused() const;

  /** Returns true if the window is currently maximized. */
  bool isMaximized() const;

  /** Returns true if the window is currently in fullscreen mode. */
  bool isFullscreen() const;

  /** Returns your custom window-specific user data.
   *  @return The userdata pointer. */
  void *getUserData() const;

  /** Sets your custom window-specific user data.
   *  @param userdata The new userdata pointer. */
  void setUserData(void *userdata);

  /** Updates specific attributes of this window.
   *  @param field_mask A bitmask of LVKW_WindowAttributesField.
   *  @param attrs The new attribute values.
   *  @throws Exception if the update fails. */
  void update(uint32_t field_mask, const LVKW_WindowAttributes &attrs);

  /** Sets the title of the window (UTF-8).
   *  @param title The new title. */
  void setTitle(const char *title);

  /** Sets the logical size of the window.
   *  @param size The new size. */
  void setSize(LVKW_LogicalVec size);

  /** Switches the window in or out of fullscreen mode.
   *  @param enabled True to enable fullscreen. */
  void setFullscreen(bool enabled);

  /** Switches the window in or out of maximized mode.
   *  @param enabled True to maximize. */
  void setMaximized(bool enabled);

  /** Sets how the cursor should behave (e.g. normal or locked).
   *  @param mode The new cursor mode. */
  void setCursorMode(LVKW_CursorMode mode);

  /** Changes the current hardware cursor.
   *  @param cursor The new cursor. NULL for system default. */
  void setCursor(LVKW_Cursor *cursor);

  /** Changes the current hardware cursor.
   *  @param cursor The RAII cursor object. */
  void setCursor(const Cursor &cursor);

  /** Sets the minimum logical size of the window.
   *  @param minSize The new minimum size. {0,0} for no limit. */
  void setMinSize(LVKW_LogicalVec minSize);

  /** Sets the maximum logical size of the window.
   *  @param maxSize The new maximum size. {0,0} for no limit. */
  void setMaxSize(LVKW_LogicalVec maxSize);

  /** Sets the aspect ratio of the window.
   *  @param aspectRatio The new aspect ratio. {0,0} for no limit. */
  void setAspectRatio(LVKW_Ratio aspectRatio);

  /** Toggles whether the window is resizable by the user.
   *  @param resizable True to allow resizing. */
  void setResizable(bool resizable);

  /** Toggles whether the window has OS decorations.
   *  @param decorated True to show decorations. */
  void setDecorated(bool decorated);

  /** Toggles whether mouse events pass through the window.
   *  @param passthrough True to enable passthrough. */
  void setMousePassthrough(bool passthrough);

  /** Asks the system to give this window input focus.
   *  @throws Exception if the request fails. */
  void requestFocus();

  /** Sets the system clipboard content to a UTF-8 string.
   *  @param text The null-terminated UTF-8 string to copy.
   *  @throws Exception if the operation fails. */
  void setClipboardText(const char *text);

  /** Retrieves the current system clipboard content as a UTF-8 string.
   *
   *  @note LIFETIME: The returned pointer is managed by the library and remains valid
   *  until the next call to getClipboardText on any window belonging to the
   *  same context, or until the context is destroyed.
   *
   *  @return The clipboard text.
   *  @throws Exception if the operation fails. */
  const char *getClipboardText() const;

  /** Sets the system clipboard with multiple data formats (MIME types).
   *  @param data pointer to clipboard data items.
   *  @param count number of items.
   *  @throws Exception if the operation fails. */
  void setClipboardData(const LVKW_ClipboardData *data, uint32_t count);

  /** Retrieves specific MIME type data from the clipboard.
   *
   *  @note LIFETIME: Managed by the library.
   *
   *  @param mime_type The desired MIME type.
   *  @param data output pointer.
   *  @param size output size.
   *  @throws Exception if the operation fails. */
  void getClipboardData(const char *mime_type, const void **data, size_t *size) const;

  /** Enumerates all MIME types currently available on the clipboard.
   *  @return A vector of MIME type strings.
   *  @throws Exception if enumeration fails. */
  std::vector<const char *> getClipboardMimeTypes() const;

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
  ~Controller();

  Controller(const Controller &) = delete;
  Controller &operator=(const Controller &) = delete;

  /** Moves a controller from another instance. */
  Controller(Controller &&other) noexcept;

  /** Transfers ownership of a controller handle. */
  Controller &operator=(Controller &&other) noexcept;

  /** Gives you the underlying C controller handle.
   *  @return The raw LVKW_Controller handle. */
  LVKW_Controller *get() const;

  /** Provides pointer-like access to the controller structure. */
  const LVKW_Controller *operator->() const;

  /** Returns detailed information about this controller.
   *  @return The controller info.
   *  @throws Exception if the query fails. */
  LVKW_CtrlInfo getInfo() const;

  /** Returns true if the controller is lost (unplugged). */
  bool isLost() const;

  /** Sets the haptic intensities for a range of channels.
   *  @param first_haptic Index of the first haptic channel to update.
   *  @param count Number of channels to update.
   *  @param intensities pointer to array of normalized values [0.0, 1.0]. */
  void setHapticLevels(uint32_t first_haptic, uint32_t count, const LVKW_real_t *intensities);

  /** Convenience method for setting standard dual-motor rumble.
   *  @param low_freq Intensity for the large motor [0.0, 1.0].
   *  @param high_freq Intensity for the small motor [0.0, 1.0]. */
  void setRumble(LVKW_real_t low_freq, LVKW_real_t high_freq);

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
  static void defaultDiagnosticCallback(const LVKW_DiagnosticInfo *info, void *userdata);

  /** Creates a context with default settings (AUTO backend).
   *  @throws Exception if creation fails. */
  Context();

  /** Creates a context using your specific creation options.
   *  @param create_info Creation parameters.
   *  @throws Exception if creation fails. */
  explicit Context(const LVKW_ContextCreateInfo &create_info);

  /** Cleans up and destroys the context. */
  ~Context();

  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  /** Moves a context from another instance.
   *  @param other The context to move from. */
  Context(Context &&other) noexcept;

  /** Transfers ownership of a context handle.
   *  @param other The context to move from.
   *  @return A reference to this context. */
  Context &operator=(Context &&other) noexcept;

  /**
   * Gives you the underlying C context handle.
   * @return The raw LVKW_Context handle. */
  LVKW_Context *get() const;

  /** Returns a list of Vulkan extensions required for this context.
   *  @return A vector of extension names. */
  std::vector<const char *> getVkExtensions() const;

  /** Configures how long to wait before sending an idle notification.
   *  @param timeout_ms The threshold in milliseconds. */
  void setIdleTimeout(uint32_t timeout_ms);

  /** Prevents the system from going idle or sleeping.
   *  @param enabled True to inhibit idle. */
  void setIdleInhibition(bool enabled);

  /** Sets the diagnostics callback for this context.
   *  @param callback The diagnostics callback function.
   *  @param userdata User data for the callback. */
  void setDiagnosticCallback(LVKW_DiagnosticCallback callback, void *userdata);

  /** Returns true if the context handle is lost.
   *
   *  A lost context must be destroyed. All associated windows are also lost.
   */
  bool isLost() const;

  /** Returns your custom global user data pointer.
   *  @return The global userdata pointer. */
  void *getUserData() const;

  /** Sets your custom global user data pointer.
   *  @param userdata The new global userdata pointer. */
  void setUserData(void *userdata);

  /** Returns a list of available monitors.
   *  @return A vector of monitor handles.
   *  @throws Exception if enumeration fails. */
  std::vector<LVKW_Monitor *> getMonitors() const;

  /** Returns the available video modes for a specific monitor.
   *  @param monitor The monitor to query.
   *  @return A vector of video modes.
   *  @throws Exception if enumeration fails. */
  std::vector<LVKW_VideoMode> getMonitorModes(const LVKW_Monitor *monitor) const;

  /** Creates a new window within this context.
   *  @param create_info Window creation parameters.
   *  @return The created Window object.
   *  @throws Exception if creation fails. */
  Window createWindow(const LVKW_WindowCreateInfo &create_info);

  /** Retrieves a handle to a standard system cursor.
   *  @param shape The desired cursor shape.
   *  @return A raw handle to the standard cursor. */
  LVKW_Cursor *getStandardCursor(LVKW_CursorShape shape) const;

  /** Creates a custom hardware cursor from pixels.
   *  @param create_info Configuration for the new cursor.
   *  @return The created RAII Cursor object. */
  Cursor createCursor(const LVKW_CursorCreateInfo &create_info);

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
  Controller createController(LVKW_CtrlId id);
#endif

 private:
  LVKW_Context *m_ctx_handle = nullptr;

  template <typename T>
  static LVKW_TelemetryCategory getCategory();
};

/**
 * Synchronizes the event queue with the OS and other sources.
 *
 * @param ctx The library context.
 * @param timeout_ms Maximum time to block waiting for events.
 * @throws Exception if synchronization fails.
 */
void syncEvents(Context &ctx, uint32_t timeout_ms = 0);

/**
 * Manually pushes a user-defined event into the queue.
 *
 * @param ctx The library context.
 * @param type One of the user-defined event types (USER_0 to USER_5).
 * @param window Optional window to associate with the event.
 * @param evt Optional payload for the event.
 * @throws Exception if posting fails.
 */
void postEvent(Context &ctx, LVKW_EventType type, LVKW_Window *window = nullptr,
               const LVKW_Event *evt = nullptr);

/**
 * Scans the current event queue and invokes the callback for matching events.
 *
 * @note LIFETIME: The event data passed to the callback is only valid
 * for the duration of the callback.
 *
 * @tparam F The callback type.
 * @param ctx The library context.
 * @param event_mask Bitmask of events to scan.
 * @param callback Function to call for each event.
 * @throws Exception if scanning fails.
 */
template <typename F>
void scanEvents(Context &ctx, LVKW_EventType event_mask, F &&callback);

#if __cplusplus < 202002L
/**
 * Convenience shorthand for non-blocking event polling.
 */
template <typename F>
void pollEvents(Context &ctx, F &&callback);

/**
 * Convenience shorthand for non-blocking event polling with an explicit mask.
 */
template <typename F>
void pollEvents(Context &ctx, LVKW_EventType mask, F &&callback);

/**
 * Convenience shorthand for blocking event waiting.
 */
template <typename Rep, typename Period, typename F>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, F &&callback);

/**
 * Convenience shorthand for blocking event waiting with an explicit mask.
 */
template <typename Rep, typename Period, typename F>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, LVKW_EventType mask,
                F &&callback);
#endif

inline bool operator==(const LVKW_PixelVec &lhs, const LVKW_PixelVec &rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator==(const LVKW_LogicalVec &lhs, const LVKW_LogicalVec &rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator==(const LVKW_Ratio &lhs, const LVKW_Ratio &rhs) noexcept {
  return lhs.numer == rhs.numer && lhs.denom == rhs.denom;
}

/* --- Inline implementations --- */
inline DndFeedback DndHoverEvent::feedback() const { return {data.feedback}; }
inline LVKW_DndAction &DndHoverEvent::action() const { return feedback().action(); }
inline void *&DndHoverEvent::sessionUserdata() const { return feedback().sessionUserdata(); }

}  // namespace lvkw

#include "details/lvkw_hpp_impl.hpp"

#if __cplusplus >= 202002L
#include "lvkw_cxx20.hpp"
#endif

#endif  // LVKW_HPP_INCLUDED
