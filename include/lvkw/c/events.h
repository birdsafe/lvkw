// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_EVENTS_H_INCLUDED
#define LVKW_EVENTS_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#include "lvkw/c/context.h"
#include "lvkw/c/core.h"
#include "lvkw/c/data.h"
#include "lvkw/c/display.h"
#include "lvkw/c/input.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "lvkw/c/ext/controller.h"
#endif

/**
 * @file events.h
 * @brief Standard event payloads and queue access APIs.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Fired when the OS/windowing system fully allocated window resources. */
typedef struct LVKW_WindowReadyEvent {
  char _unused;
} LVKW_WindowReadyEvent;

/** @brief Fired when the user/system requests window close. */
typedef struct LVKW_WindowCloseEvent {
  char _unused;
} LVKW_WindowCloseEvent;

/** @brief Fired when window geometry changes. */
typedef struct LVKW_WindowResizedEvent {
  LVKW_WindowGeometry geometry;
} LVKW_WindowResizedEvent;

/** @brief Fired when the window is maximized/restored. */
typedef struct LVKW_WindowMaximizationEvent {
  bool maximized;
} LVKW_WindowMaximizationEvent;

/** @brief Fired when a physical key is pressed or released. */
typedef struct LVKW_KeyboardEvent {
  LVKW_Key key;
  LVKW_ButtonState state;
  LVKW_ModifierFlags modifiers;
} LVKW_KeyboardEvent;

/** @brief Fired when mouse pointer moves. */
typedef struct LVKW_MouseMotionEvent {
  LVKW_LogicalVec position;
  LVKW_LogicalVec delta;
  LVKW_LogicalVec raw_delta;
} LVKW_MouseMotionEvent;

/** @brief Fired when a mouse button is pressed or released. */
typedef struct LVKW_MouseButtonEvent {
  LVKW_MouseButton button;
  LVKW_ButtonState state;
  LVKW_ModifierFlags modifiers;
} LVKW_MouseButtonEvent;

/** @brief Fired on scroll wheel/touchpad scroll gestures. */
typedef struct LVKW_MouseScrollEvent {
  LVKW_LogicalVec delta;
  struct {
    int32_t x;
    int32_t y;
  } steps;
} LVKW_MouseScrollEvent;

/** @brief Fired when the system idle state changes. */
typedef struct LVKW_IdleEvent {
  uint32_t timeout_ms;
  bool is_idle;
} LVKW_IdleEvent;

/** @brief Fired on monitor connection/disconnection. */
typedef struct LVKW_MonitorConnectionEvent {
  LVKW_MonitorRef *monitor_ref;
  bool connected;
} LVKW_MonitorConnectionEvent;

/** @brief Fired when monitor mode or scale changes. */
typedef struct LVKW_MonitorModeEvent {
  LVKW_Monitor *monitor;
} LVKW_MonitorModeEvent;

/** @brief Fired when committed text input is received. */
typedef struct LVKW_TextInputEvent {
  LVKW_TRANSIENT const char *text;
  uint32_t length;
} LVKW_TextInputEvent;

/** @brief Fired during active IME preedit composition. */
typedef struct LVKW_TextCompositionEvent {
  LVKW_TRANSIENT const char *text;
  uint32_t length;
  uint32_t cursor_index;
  uint32_t selection_length;
} LVKW_TextCompositionEvent;

/** @brief Fired when the window gains or loses focus. */
typedef struct LVKW_FocusEvent {
  bool focused;
} LVKW_FocusEvent;

/** @brief Feedback provided by application during a DND session. */
typedef struct LVKW_DndFeedback {
  LVKW_DndAction *action;
  void **session_userdata;
} LVKW_DndFeedback;

/** @brief Fired when drag is active over the window. */
typedef struct LVKW_DndHoverEvent {
  LVKW_LogicalVec position;
  LVKW_TRANSIENT LVKW_DndFeedback *feedback;
  LVKW_TRANSIENT const char **paths;
  LVKW_ModifierFlags modifiers;
  uint16_t path_count;
  bool entered;
} LVKW_DndHoverEvent;

/** @brief Fired when drag leaves the window without drop. */
typedef struct LVKW_DndLeaveEvent {
  void **session_userdata;
} LVKW_DndLeaveEvent;

/** @brief Fired when data/files are dropped on the window. */
typedef struct LVKW_DndDropEvent {
  LVKW_LogicalVec position;
  void **session_userdata;
  LVKW_TRANSIENT const char **paths;
  LVKW_ModifierFlags modifiers;
  uint16_t path_count;
} LVKW_DndDropEvent;

/** @brief Unified standard event payload union. */
typedef struct LVKW_Event {
  union {
    LVKW_WindowReadyEvent window_ready;
    LVKW_WindowCloseEvent close_requested;
    LVKW_WindowResizedEvent resized;
    LVKW_WindowMaximizationEvent maximized;
    LVKW_KeyboardEvent key;
    LVKW_MouseMotionEvent mouse_motion;
    LVKW_MouseButtonEvent mouse_button;
    LVKW_MouseScrollEvent mouse_scroll;
    LVKW_IdleEvent idle;
    LVKW_MonitorConnectionEvent monitor_connection;
    LVKW_MonitorModeEvent monitor_mode;
#ifdef LVKW_ENABLE_CONTROLLER
    LVKW_CtrlConnectionEvent controller_connection;
#endif
    LVKW_TextInputEvent text_input;
    LVKW_TextCompositionEvent text_composition;
    LVKW_FocusEvent focus;
    LVKW_DndHoverEvent dnd_hover;
    LVKW_DndLeaveEvent dnd_leave;
    LVKW_DndDropEvent dnd_drop;
  };
} LVKW_Event;

/** @brief Callback signature for event scanning. */
typedef void (*LVKW_EventCallback)(LVKW_EventType type, LVKW_Window *window, const LVKW_Event *evt,
                                   void *userdata);

LVKW_HOT LVKW_Status lvkw_events_pump(LVKW_Context *context, uint32_t timeout_ms);

LVKW_HOT LVKW_Status lvkw_events_commit(LVKW_Context *context);

LVKW_HOT LVKW_Status lvkw_events_post(LVKW_Context *context, LVKW_EventType type,
                                      LVKW_Window *window, const LVKW_Event *evt);

LVKW_HOT LVKW_Status lvkw_events_scan(LVKW_Context *context, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata);

static inline LVKW_Status lvkw_events_setMask(LVKW_Context *context, uint32_t event_mask) {
  LVKW_ContextAttributes attrs = {0};
  attrs.event_mask = (LVKW_EventType)event_mask;
  return lvkw_context_update(context, LVKW_CONTEXT_ATTR_EVENT_MASK, &attrs);
}

/** @brief Convenience shorthand for non-blocking event poll + scan. */
static inline LVKW_Status lvkw_events_poll(LVKW_Context *context, LVKW_EventType mask,
                                           LVKW_EventCallback callback, void *userdata) {
  LVKW_Status status = lvkw_events_pump(context, 0);
  if (status != LVKW_SUCCESS) {
    return status;
  }
  status = lvkw_events_commit(context);
  if (status != LVKW_SUCCESS) {
    return status;
  }
  return lvkw_events_scan(context, mask, callback, userdata);
}

/** @brief Convenience shorthand for blocking event wait + scan. */
static inline LVKW_Status lvkw_events_wait(LVKW_Context *context, uint32_t timeout_ms,
                                           LVKW_EventType mask, LVKW_EventCallback callback,
                                           void *userdata) {
  LVKW_Status status = lvkw_events_pump(context, timeout_ms);
  if (status != LVKW_SUCCESS) {
    return status;
  }
  status = lvkw_events_commit(context);
  if (status != LVKW_SUCCESS) {
    return status;
  }
  return lvkw_events_scan(context, mask, callback, userdata);
}

#ifdef __cplusplus
}
#endif

#endif  // LVKW_EVENTS_H_INCLUDED
