#include "lvkw/lvkw-window.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline LVKW_Status lvkw_wnd_setTitle(LVKW_Window *window, const char *title) {
  LVKW_WindowAttributes attrs = {0};
  attrs.title = title;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_TITLE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setSize(LVKW_Window *window, LVKW_LogicalVec size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.logicalSize = size;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_LOGICAL_SIZE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setFullscreen(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.fullscreen = enabled;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_FULLSCREEN, &attrs);
}

static inline LVKW_Status lvkw_wnd_setMaximized(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.maximized = enabled;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_MAXIMIZED, &attrs);
}

static inline LVKW_Status lvkw_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_mode = mode;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR_MODE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setCursorShape(LVKW_Window *window, LVKW_CursorShape shape) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_shape = shape;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR_SHAPE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setMonitor(LVKW_Window *window, LVKW_MonitorId monitor) {
  LVKW_WindowAttributes attrs = {0};
  attrs.monitor = monitor;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_MONITOR, &attrs);
}

static inline LVKW_Status lvkw_wnd_setMinSize(LVKW_Window *window, LVKW_LogicalVec min_size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.minSize = min_size;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_MIN_SIZE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setMaxSize(LVKW_Window *window, LVKW_LogicalVec max_size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.maxSize = max_size;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_MAX_SIZE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setAspectRatio(LVKW_Window *window, LVKW_Ratio aspect_ratio) {
  LVKW_WindowAttributes attrs = {0};
  attrs.aspect_ratio = aspect_ratio;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_ASPECT_RATIO, &attrs);
}

static inline LVKW_Status lvkw_wnd_setResizable(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.resizable = enabled;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_RESIZABLE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setDecorated(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.decorated = enabled;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_DECORATED, &attrs);
}

#ifdef __cplusplus
}
#endif
