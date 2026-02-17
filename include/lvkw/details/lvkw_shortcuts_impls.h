// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_SHORTCUTS_IMPLS_H_INCLUDED
#define LVKW_SHORTCUTS_IMPLS_H_INCLUDED

#include "lvkw/c/shortcuts.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline LVKW_Status lvkw_context_setIdleTimeout(LVKW_Context *context, uint32_t timeout_ms) {
  LVKW_ContextAttributes attrs = {0};
  attrs.idle_timeout_ms = timeout_ms;
  return lvkw_context_update(context, LVKW_CONTEXT_ATTR_IDLE_TIMEOUT, &attrs);
}

static inline LVKW_Status lvkw_context_setIdleInhibition(LVKW_Context *context, bool enabled) {
  LVKW_ContextAttributes attrs = {0};
  attrs.inhibit_idle = enabled;
  return lvkw_context_update(context, LVKW_CONTEXT_ATTR_INHIBIT_IDLE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowTitle(LVKW_Window *window, const char *title) {
  LVKW_WindowAttributes attrs = {0};
  attrs.title = title;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_TITLE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowSize(LVKW_Window *window, LVKW_LogicalVec size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.logicalSize = size;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_LOGICAL_SIZE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowFullscreen(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.fullscreen = enabled;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_FULLSCREEN, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowMaximized(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.maximized = enabled;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_MAXIMIZED, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowCursorMode(LVKW_Window *window, LVKW_CursorMode mode) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor_mode = mode;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_CURSOR_MODE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowCursor(LVKW_Window *window, LVKW_Cursor *cursor) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor = cursor;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_CURSOR, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowMonitor(LVKW_Window *window, LVKW_Monitor *monitor) {
  LVKW_WindowAttributes attrs = {0};
  attrs.monitor = monitor;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_MONITOR, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowMinSize(LVKW_Window *window, LVKW_LogicalVec min_size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.minSize = min_size;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_MIN_SIZE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowMaxSize(LVKW_Window *window, LVKW_LogicalVec max_size) {
  LVKW_WindowAttributes attrs = {0};
  attrs.maxSize = max_size;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_MAX_SIZE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowAspectRatio(LVKW_Window *window,
                                                            LVKW_Fraction aspect_ratio) {
  LVKW_WindowAttributes attrs = {0};
  attrs.aspect_ratio = aspect_ratio;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_ASPECT_RATIO, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowResizable(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.resizable = enabled;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_RESIZABLE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowDecorated(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.decorated = enabled;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_DECORATED, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowMousePassthrough(LVKW_Window *window,
                                                                 bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.mouse_passthrough = enabled;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_MOUSE_PASSTHROUGH, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowTextInputType(LVKW_Window *window,
                                                              LVKW_TextInputType type) {
  LVKW_WindowAttributes attrs = {0};
  attrs.text_input_type = type;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE, &attrs);
}

static inline LVKW_Status lvkw_display_setWindowTextInputRect(LVKW_Window *window,
                                                              LVKW_LogicalRect rect) {
  LVKW_WindowAttributes attrs = {0};
  attrs.text_input_rect = rect;
  return lvkw_display_updateWindow(window, LVKW_WINDOW_ATTR_TEXT_INPUT_RECT, &attrs);
}

#ifdef __cplusplus
}
#endif

#endif  // LVKW_SHORTCUTS_IMPLS_H_INCLUDED
