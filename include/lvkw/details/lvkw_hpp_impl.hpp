// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_HPP_IMPL_HPP_INCLUDED
#define LVKW_HPP_IMPL_HPP_INCLUDED

namespace lvkw {

/* --- Exception Implementations --- */

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

/* --- Cursor Implementations --- */

inline Cursor::~Cursor() {
  if (is_owned()) {
    lvkw_cursor_destroy(m_cursor_handle);
  }
}

inline Cursor::Cursor(Cursor &&other) noexcept : m_cursor_handle(other.m_cursor_handle) {
  other.m_cursor_handle = nullptr;
}

inline Cursor &Cursor::operator=(Cursor &&other) noexcept {
  if (this != &other) {
    if (is_owned()) {
      lvkw_cursor_destroy(m_cursor_handle);
    }
    m_cursor_handle = other.m_cursor_handle;
    other.m_cursor_handle = nullptr;
  }
  return *this;
}

inline LVKW_Cursor *Cursor::get() const { return m_cursor_handle; }

inline bool Cursor::is_owned() const {
  return m_cursor_handle && !(m_cursor_handle->flags & LVKW_CURSOR_FLAG_SYSTEM);
}

/* --- Window Implementations --- */

inline Window::~Window() {
  if (m_window_handle) {
    lvkw_wnd_destroy(m_window_handle);
  }
}

inline Window::Window(Window &&other) noexcept : m_window_handle(other.m_window_handle) {
  other.m_window_handle = nullptr;
}

inline Window &Window::operator=(Window &&other) noexcept {
  if (this != &other) {
    if (m_window_handle) {
      lvkw_wnd_destroy(m_window_handle);
    }
    m_window_handle = other.m_window_handle;
    other.m_window_handle = nullptr;
  }
  return *this;
}

inline LVKW_Window *Window::get() const { return m_window_handle; }

inline VkSurfaceKHR Window::createVkSurface(VkInstance instance) const {
  VkSurfaceKHR surface;
  check(lvkw_wnd_createVkSurface(m_window_handle, instance, &surface),
        "Failed to create Vulkan surface");
  return surface;
}

inline LVKW_WindowGeometry Window::getGeometry() const {
  LVKW_WindowGeometry geometry;
  check(lvkw_wnd_getGeometry(m_window_handle, &geometry), "Failed to get window geometry");
  return geometry;
}

inline bool Window::isLost() const { return m_window_handle->flags & LVKW_WND_STATE_LOST; }

inline bool Window::isReady() const { return m_window_handle->flags & LVKW_WND_STATE_READY; }

inline bool Window::isFocused() const { return m_window_handle->flags & LVKW_WND_STATE_FOCUSED; }

inline bool Window::isMaximized() const { return m_window_handle->flags & LVKW_WND_STATE_MAXIMIZED; }

inline bool Window::isFullscreen() const { return m_window_handle->flags & LVKW_WND_STATE_FULLSCREEN; }

inline void *Window::getUserData() const { return m_window_handle->userdata; }

inline void Window::setUserData(void *userdata) { m_window_handle->userdata = userdata; }

inline void Window::update(uint32_t field_mask, const LVKW_WindowAttributes &attrs) {
  check(lvkw_wnd_update(m_window_handle, field_mask, &attrs), "Failed to update window attributes");
}

inline void Window::setTitle(const char *title) {
  LVKW_WindowAttributes attrs = {};
  attrs.title = title;
  update(LVKW_WND_ATTR_TITLE, attrs);
}

inline void Window::setSize(LVKW_LogicalVec size) {
  LVKW_WindowAttributes attrs = {};
  attrs.logicalSize = size;
  update(LVKW_WND_ATTR_LOGICAL_SIZE, attrs);
}

inline void Window::setFullscreen(bool enabled) {
  LVKW_WindowAttributes attrs = {};
  attrs.fullscreen = enabled;
  update(LVKW_WND_ATTR_FULLSCREEN, attrs);
}

inline void Window::setMaximized(bool enabled) {
  LVKW_WindowAttributes attrs = {};
  attrs.maximized = enabled;
  update(LVKW_WND_ATTR_MAXIMIZED, attrs);
}

inline void Window::setCursorMode(LVKW_CursorMode mode) {
  LVKW_WindowAttributes attrs = {};
  attrs.cursor_mode = mode;
  update(LVKW_WND_ATTR_CURSOR_MODE, attrs);
}

inline void Window::setCursor(LVKW_Cursor *cursor) {
  LVKW_WindowAttributes attrs = {};
  attrs.cursor = cursor;
  update(LVKW_WND_ATTR_CURSOR, attrs);
}

inline void Window::setCursor(const Cursor &cursor) { setCursor(cursor.get()); }

inline void Window::setMinSize(LVKW_LogicalVec minSize) {
  LVKW_WindowAttributes attrs = {};
  attrs.minSize = minSize;
  update(LVKW_WND_ATTR_MIN_SIZE, attrs);
}

inline void Window::setMaxSize(LVKW_LogicalVec maxSize) {
  LVKW_WindowAttributes attrs = {};
  attrs.maxSize = maxSize;
  update(LVKW_WND_ATTR_MAX_SIZE, attrs);
}

inline void Window::setAspectRatio(LVKW_Fraction aspectRatio) {
  LVKW_WindowAttributes attrs = {};
  attrs.aspect_ratio = aspectRatio;
  update(LVKW_WND_ATTR_ASPECT_RATIO, attrs);
}

inline void Window::setResizable(bool resizable) {
  LVKW_WindowAttributes attrs = {};
  attrs.resizable = resizable;
  update(LVKW_WND_ATTR_RESIZABLE, attrs);
}

inline void Window::setDecorated(bool decorated) {
  LVKW_WindowAttributes attrs = {};
  attrs.decorated = decorated;
  update(LVKW_WND_ATTR_DECORATED, attrs);
}

inline void Window::setMousePassthrough(bool passthrough) {
  LVKW_WindowAttributes attrs = {};
  attrs.mouse_passthrough = passthrough;
  update(LVKW_WND_ATTR_MOUSE_PASSTHROUGH, attrs);
}

inline void Window::requestFocus() {
  check(lvkw_wnd_requestFocus(m_window_handle), "Failed to request focus");
}

inline void Window::setClipboardText(const char *text) {
  check(lvkw_wnd_setClipboardText(m_window_handle, text), "Failed to set clipboard text");
}

inline const char *Window::getClipboardText() const {
  const char *text;
  check(lvkw_wnd_getClipboardText(m_window_handle, &text), "Failed to get clipboard text");
  return text;
}

inline void Window::setClipboardData(const LVKW_ClipboardData *data, uint32_t count) {
  check(lvkw_wnd_setClipboardData(m_window_handle, data, count), "Failed to set clipboard data");
}

inline void Window::getClipboardData(const char *mime_type, const void **data, size_t *size) const {
  check(lvkw_wnd_getClipboardData(m_window_handle, mime_type, data, size),
        "Failed to get clipboard data");
}

inline std::vector<const char *> Window::getClipboardMimeTypes() const {
  uint32_t count = 0;
  check(lvkw_wnd_getClipboardMimeTypes(m_window_handle, nullptr, &count),
        "Failed to get MIME type count");
  if (count == 0) return std::vector<const char *>();
  const char **mime_types_ptr = nullptr;
  check(lvkw_wnd_getClipboardMimeTypes(m_window_handle, &mime_types_ptr, &count),
        "Failed to get MIME types");
  if (!mime_types_ptr || count == 0) return std::vector<const char *>();
  return std::vector<const char *>(mime_types_ptr, mime_types_ptr + count);
}

/* --- Controller Implementations --- */

#ifdef LVKW_ENABLE_CONTROLLER
inline Controller::~Controller() {
  if (m_controller_handle) {
    lvkw_ctrl_destroy(m_controller_handle);
  }
}

inline Controller::Controller(Controller &&other) noexcept
    : m_controller_handle(other.m_controller_handle) {
  other.m_controller_handle = nullptr;
}

inline Controller &Controller::operator=(Controller &&other) noexcept {
  if (this != &other) {
    if (m_controller_handle) {
      lvkw_ctrl_destroy(m_controller_handle);
    }
    m_controller_handle = other.m_controller_handle;
    other.m_controller_handle = nullptr;
  }
  return *this;
}

inline LVKW_Controller *Controller::get() const { return m_controller_handle; }

inline const LVKW_Controller *Controller::operator->() const { return m_controller_handle; }

inline LVKW_CtrlInfo Controller::getInfo() const {
  LVKW_CtrlInfo info;
  check(lvkw_ctrl_getInfo(m_controller_handle, &info), "Failed to get controller info");
  return info;
}

inline bool Controller::isLost() const {
  return m_controller_handle->flags & LVKW_WND_STATE_LOST;
}

inline void Controller::setHapticLevels(uint32_t first_haptic, uint32_t count,
                                        const LVKW_Scalar *intensities) {
  check(lvkw_ctrl_setHapticLevels(m_controller_handle, first_haptic, count, intensities),
        "Failed to set controller haptic levels");
}

inline void Controller::setRumble(LVKW_Scalar low_freq, LVKW_Scalar high_freq) {
  const LVKW_Scalar levels[] = {low_freq, high_freq};
  setHapticLevels(LVKW_CTRL_HAPTIC_LOW_FREQ, 2, levels);
}
#endif

/* --- Context Implementations --- */

inline void Context::defaultDiagnosticCallback(const LVKW_DiagnosticInfo *info, void * /*userdata*/) {
  std::cerr << "LVKW Diagnostic: " << info->message << " (Code: " << (int)info->diagnostic << ")"
            << std::endl;
}

inline Context::Context() {
  LVKW_ContextCreateInfo ci = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  ci.backend = LVKW_BACKEND_AUTO;
  ci.attributes.diagnostic_cb = defaultDiagnosticCallback;
  check(lvkw_createContext(&ci, &m_ctx_handle), "Failed to create LVKW context");
}

inline Context::Context(const LVKW_ContextCreateInfo &create_info) {
  LVKW_ContextCreateInfo ci = create_info;
  if (!ci.attributes.diagnostic_cb) {
    ci.attributes.diagnostic_cb = defaultDiagnosticCallback;
  }
  check(lvkw_createContext(&ci, &m_ctx_handle), "Failed to create LVKW context");
}

inline Context::~Context() {
  if (m_ctx_handle) {
    lvkw_ctx_destroy(m_ctx_handle);
  }
}

inline Context::Context(Context &&other) noexcept : m_ctx_handle(other.m_ctx_handle) {
  other.m_ctx_handle = nullptr;
}

inline Context &Context::operator=(Context &&other) noexcept {
  if (this != &other) {
    if (m_ctx_handle) {
      lvkw_ctx_destroy(m_ctx_handle);
    }
    m_ctx_handle = other.m_ctx_handle;
    other.m_ctx_handle = nullptr;
  }
  return *this;
}

inline LVKW_Context *Context::get() const { return m_ctx_handle; }

inline std::vector<const char *> Context::getVkExtensions() const {
  uint32_t count = 0;
  const char *const *extensions = nullptr;
  check(lvkw_ctx_getVkExtensions(m_ctx_handle, &count, &extensions),
        "Failed to get Vulkan extensions");
  if (!extensions || count == 0) return std::vector<const char *>();
  return std::vector<const char *>(extensions, extensions + count);
}

inline void Context::setIdleTimeout(uint32_t timeout_ms) {
  LVKW_ContextAttributes attrs = {};
  attrs.idle_timeout_ms = timeout_ms;
  check(lvkw_ctx_update(m_ctx_handle, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs),
        "Failed to update context attributes");
}

inline void Context::setIdleInhibition(bool enabled) {
  LVKW_ContextAttributes attrs = {};
  attrs.inhibit_idle = enabled;
  check(lvkw_ctx_update(m_ctx_handle, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs),
        "Failed to update context attributes");
}

inline void Context::setDiagnosticCallback(LVKW_DiagnosticCallback callback, void *userdata) {
  check(lvkw_ctx_setDiagnosticCallback(m_ctx_handle, callback, userdata),
        "Failed to set diagnostics callback");
}

inline bool Context::isLost() const { return m_ctx_handle->flags & LVKW_CTX_STATE_LOST; }

inline void *Context::getUserData() const { return m_ctx_handle->userdata; }

inline void Context::setUserData(void *userdata) { m_ctx_handle->userdata = userdata; }

inline std::vector<LVKW_Monitor *> Context::getMonitors() const {
  uint32_t count = 0;
  check(lvkw_ctx_getMonitors(m_ctx_handle, nullptr, &count), "Failed to get monitor count");
  std::vector<LVKW_Monitor *> monitors(count);
  check(lvkw_ctx_getMonitors(m_ctx_handle, monitors.data(), &count), "Failed to get monitors");
  return monitors;
}

inline std::vector<LVKW_VideoMode> Context::getMonitorModes(const LVKW_Monitor *monitor) const {
  uint32_t count = 0;
  check(lvkw_ctx_getMonitorModes(m_ctx_handle, monitor, nullptr, &count),
        "Failed to get mode count");
  std::vector<LVKW_VideoMode> modes(count);
  check(lvkw_ctx_getMonitorModes(m_ctx_handle, monitor, modes.data(), &count),
        "Failed to get modes");
  return modes;
}

inline Window Context::createWindow(const LVKW_WindowCreateInfo &create_info) {
  LVKW_Window *handle;
  check(lvkw_ctx_createWindow(m_ctx_handle, &create_info, &handle), "Failed to create LVKW window");
  return Window(handle);
}

inline LVKW_Cursor *Context::getStandardCursor(LVKW_CursorShape shape) const {
  LVKW_Cursor *handle;
  check(lvkw_ctx_getStandardCursor(m_ctx_handle, shape, &handle),
        "Failed to get standard cursor");
  return handle;
}

inline Cursor Context::createCursor(const LVKW_CursorCreateInfo &create_info) {
  LVKW_Cursor *handle;
  check(lvkw_ctx_createCursor(m_ctx_handle, &create_info, &handle),
        "Failed to create custom cursor");
  return Cursor(handle);
}

#ifdef LVKW_ENABLE_CONTROLLER
inline Controller Context::createController(LVKW_CtrlId id) {
  LVKW_Controller *handle;
  check(lvkw_ctrl_create(m_ctx_handle, id, &handle), "Failed to create LVKW controller");
  return Controller(handle);
}

inline std::vector<LVKW_CtrlId> Context::listControllers() const {
  uint32_t count = 0;
  check(lvkw_ctrl_list(m_ctx_handle, nullptr, &count), "Failed to count controllers");
  std::vector<LVKW_CtrlId> ids(count);
  if (count > 0) {
    check(lvkw_ctrl_list(m_ctx_handle, ids.data(), &count), "Failed to list controllers");
    ids.resize(count);
  }
  return ids;
}
#endif

template <typename T>
LVKW_MetricsCategory Context::getCategory() {
  if (std::is_same<T, LVKW_EventMetrics>::value) return LVKW_METRICS_CATEGORY_EVENTS;
  return LVKW_METRICS_CATEGORY_NONE;
}

/* --- Free Function Implementations --- */

inline void syncEvents(Context &ctx, uint32_t timeout_ms) {
  check(lvkw_ctx_syncEvents(ctx.get(), timeout_ms), "Failed to sync events");
}

inline void postEvent(Context &ctx, LVKW_EventType type, LVKW_Window *window,
                      const LVKW_Event *evt) {
  check(lvkw_ctx_postEvent(ctx.get(), type, window, evt), "Failed to post event");
}

template <typename F>
void scanEvents(Context &ctx, LVKW_EventType event_mask, F &&callback) {
  check(lvkw_ctx_scanEvents(
            ctx.get(), event_mask,
            [](LVKW_EventType type, LVKW_Window *window, const LVKW_Event *evt, void *userdata) {
              typedef typename std::remove_reference<F>::type F_raw;
              F_raw &cb = *static_cast<F_raw *>(userdata);
              cb(type, window, *evt);
            },
            &callback),
        "Failed to scan events");
}

#if __cplusplus < 202002L
template <typename F>
void pollEvents(Context &ctx, F &&callback) {
  pollEvents(ctx, LVKW_EVENT_TYPE_ALL, std::forward<F>(callback));
}

template <typename F>
void pollEvents(Context &ctx, LVKW_EventType mask, F &&callback) {
  syncEvents(ctx, 0);
  scanEvents(ctx, mask, std::forward<F>(callback));
}

template <typename Rep, typename Period, typename F>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, F &&callback) {
  waitEvents(ctx, timeout, LVKW_EVENT_TYPE_ALL, std::forward<F>(callback));
}

template <typename Rep, typename Period, typename F>
void waitEvents(Context &ctx, std::chrono::duration<Rep, Period> timeout, LVKW_EventType mask,
                F &&callback) {
  syncEvents(ctx, static_cast<uint32_t>(
                      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()));
  scanEvents(ctx, mask, std::forward<F>(callback));
}
#endif

}  // namespace lvkw

#endif  // LVKW_HPP_IMPL_HPP_INCLUDED
