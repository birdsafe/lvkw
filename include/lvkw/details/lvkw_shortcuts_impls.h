// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_SHORTCUTS_IMPLS_H_INCLUDED
#define LVKW_SHORTCUTS_IMPLS_H_INCLUDED

#include "lvkw/lvkw-shortcuts.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void lvkw_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType mask,
                                       LVKW_EventCallback callback, void *userdata) {
  lvkw_ctx_syncEvents(ctx, 0);
  lvkw_ctx_scanEvents(ctx, mask, callback, userdata);
}

static inline void lvkw_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType mask,
                                       LVKW_EventCallback callback, void *userdata) {
  lvkw_ctx_syncEvents(ctx, timeout_ms);
  lvkw_ctx_scanEvents(ctx, mask, callback, userdata);
}

/*  ----- CONTEXT ATTRIBUTE ASSIGNMENT HELPERS ----- */

static inline LVKW_Status lvkw_ctx_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms) {
  LVKW_ContextAttributes attrs = {0};
  attrs.idle_timeout_ms = timeout_ms;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_IDLE_TIMEOUT, &attrs);
}

static inline LVKW_Status lvkw_ctx_setIdleInhibition(LVKW_Context *ctx, bool enabled) {
  LVKW_ContextAttributes attrs = {0};
  attrs.inhibit_idle = enabled;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_INHIBIT_IDLE, &attrs);
}

static inline LVKW_Status lvkw_ctx_setDiagnosticCallback(LVKW_Context *ctx,
                                                         LVKW_DiagnosticCallback callback,
                                                         void *userdata) {
  LVKW_ContextAttributes attrs = {0};
  attrs.diagnostic_cb = callback;
  attrs.diagnostic_userdata = userdata;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_DIAGNOSTICS, &attrs);
}

static inline LVKW_Status lvkw_ctx_setEventMask(LVKW_Context *ctx, uint32_t event_mask) {
  LVKW_ContextAttributes attrs = {0};
  attrs.event_mask = (LVKW_EventType)event_mask;
  return lvkw_ctx_update(ctx, LVKW_CTX_ATTR_EVENT_MASK, &attrs);
}

/* ----- WINDOW ATTRIBUTE ASSIGNMENT HELPERS ----- */

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

static inline LVKW_Status lvkw_wnd_setCursor(LVKW_Window *window, LVKW_Cursor *cursor) {
  LVKW_WindowAttributes attrs = {0};
  attrs.cursor = cursor;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_CURSOR, &attrs);
}

static inline LVKW_Status lvkw_wnd_setMonitor(LVKW_Window *window, LVKW_Monitor *monitor) {
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

static inline LVKW_Status lvkw_wnd_setAspectRatio(LVKW_Window *window, LVKW_Fraction aspect_ratio) {
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

static inline LVKW_Status lvkw_wnd_setMousePassthrough(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.mouse_passthrough = enabled;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_MOUSE_PASSTHROUGH, &attrs);
}

static inline LVKW_Status lvkw_wnd_setAcceptDnd(LVKW_Window *window, bool enabled) {
  LVKW_WindowAttributes attrs = {0};
  attrs.accept_dnd = enabled;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_ACCEPT_DND, &attrs);
}

static inline LVKW_Status lvkw_wnd_setTextInputType(LVKW_Window *window, LVKW_TextInputType type) {
  LVKW_WindowAttributes attrs = {0};
  attrs.text_input_type = type;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_TEXT_INPUT_TYPE, &attrs);
}

static inline LVKW_Status lvkw_wnd_setTextInputRect(LVKW_Window *window, LVKW_LogicalRect rect) {
  LVKW_WindowAttributes attrs = {0};
  attrs.text_input_rect = rect;
  return lvkw_wnd_update(window, LVKW_WND_ATTR_TEXT_INPUT_RECT, &attrs);
}

#ifdef __cplusplus
}
#endif

#endif // LVKW_SHORTCUTS_IMPLS_H_INCLUDED
