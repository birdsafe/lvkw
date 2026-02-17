// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_SHORTCUTS_H_INCLUDED
#define LVKW_SHORTCUTS_H_INCLUDED

#include "lvkw/c/context.h"
#include "lvkw/c/display.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline LVKW_Status lvkw_context_setIdleInhibition(LVKW_Context *context, bool enabled);

static inline LVKW_Status lvkw_display_setWindowTitle(LVKW_Window *window, const char *title);
static inline LVKW_Status lvkw_display_setWindowSize(LVKW_Window *window, LVKW_LogicalVec size);
static inline LVKW_Status lvkw_display_setWindowFullscreen(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_display_setWindowMaximized(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_display_setWindowCursorMode(LVKW_Window *window, LVKW_CursorMode mode);
static inline LVKW_Status lvkw_display_setWindowCursor(LVKW_Window *window, LVKW_Cursor *cursor);
static inline LVKW_Status lvkw_display_setWindowMonitor(LVKW_Window *window, LVKW_Monitor *monitor);
static inline LVKW_Status lvkw_display_setWindowMinSize(LVKW_Window *window, LVKW_LogicalVec min_size);
static inline LVKW_Status lvkw_display_setWindowMaxSize(LVKW_Window *window, LVKW_LogicalVec max_size);
static inline LVKW_Status lvkw_display_setWindowAspectRatio(LVKW_Window *window,
                                                            LVKW_Fraction aspect_ratio);
static inline LVKW_Status lvkw_display_setWindowResizable(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_display_setWindowDecorated(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_display_setWindowMousePassthrough(LVKW_Window *window,
                                                                  bool enabled);
static inline LVKW_Status lvkw_display_setWindowTextInputType(LVKW_Window *window,
                                                               LVKW_TextInputType type);
static inline LVKW_Status lvkw_display_setWindowTextInputRect(LVKW_Window *window,
                                                               LVKW_LogicalRect rect);

#ifdef __cplusplus
}
#endif

#include "lvkw/details/lvkw_shortcuts_impls.h"

#endif  // LVKW_SHORTCUTS_H_INCLUDED
