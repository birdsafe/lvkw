// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_SHORTCUTS_H_INCLUDED
#define LVKW_SHORTCUTS_H_INCLUDED

#include "lvkw-context.h"
#include "lvkw-events.h"
#include "lvkw-window.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convenience shorthand for non-blocking event polling.
 *
 * Equivalent to:
 * @code
 * lvkw_ctx_syncEvents(ctx, 0);
 * lvkw_ctx_scanEvents(ctx, mask, callback, userdata);
 * @endcode
 */
static inline void lvkw_ctx_pollEvents(LVKW_Context *ctx, LVKW_EventType mask,
                                       LVKW_EventCallback callback, void *userdata);

/**
 * @brief Convenience shorthand for blocking event waiting.
 *
 * Equivalent to:
 * @code
 * lvkw_ctx_syncEvents(ctx, timeout_ms);
 * lvkw_ctx_scanEvents(ctx, mask, callback, userdata);
 * @endcode
 */
static inline void lvkw_ctx_waitEvents(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType mask,
                                       LVKW_EventCallback callback, void *userdata);

/*  ----- CONTEXT ATTRIBUTE ASSIGNMENT HELPERS ----- */

static inline LVKW_Status lvkw_ctx_setIdleTimeout(LVKW_Context *ctx, uint32_t timeout_ms);
static inline LVKW_Status lvkw_ctx_setIdleInhibition(LVKW_Context *ctx, bool enabled);
static inline LVKW_Status lvkw_ctx_setDiagnosticCallback(LVKW_Context *ctx,
                                                         LVKW_DiagnosticCallback callback,
                                                         void *userdata);
static inline LVKW_Status lvkw_ctx_setEventMask(LVKW_Context *ctx, uint32_t event_mask);

/* ----- WINDOW ATTRIBUTE ASSIGNMENT HELPERS ----- */

static inline LVKW_Status lvkw_wnd_setTitle(LVKW_Window *window, const char *title);
static inline LVKW_Status lvkw_wnd_setSize(LVKW_Window *window, LVKW_LogicalVec size);
static inline LVKW_Status lvkw_wnd_setFullscreen(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_wnd_setMaximized(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_wnd_setCursorMode(LVKW_Window *window, LVKW_CursorMode mode);
static inline LVKW_Status lvkw_wnd_setCursor(LVKW_Window *window, LVKW_Cursor *cursor);
static inline LVKW_Status lvkw_wnd_setMonitor(LVKW_Window *window, LVKW_Monitor *monitor);
static inline LVKW_Status lvkw_wnd_setMinSize(LVKW_Window *window, LVKW_LogicalVec min_size);
static inline LVKW_Status lvkw_wnd_setMaxSize(LVKW_Window *window, LVKW_LogicalVec max_size);
static inline LVKW_Status lvkw_wnd_setAspectRatio(LVKW_Window *window, LVKW_Fraction aspect_ratio);
static inline LVKW_Status lvkw_wnd_setResizable(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_wnd_setDecorated(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_wnd_setMousePassthrough(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_wnd_setAcceptDnd(LVKW_Window *window, bool enabled);
static inline LVKW_Status lvkw_wnd_setTextInputType(LVKW_Window *window, LVKW_TextInputType type);
static inline LVKW_Status lvkw_wnd_setTextInputRect(LVKW_Window *window, LVKW_LogicalRect rect);

#ifdef __cplusplus
}
#endif

#include "lvkw/details/lvkw_shortcuts_impls.h"

#endif // LVKW_SHORTCUTS_H_INCLUDED
