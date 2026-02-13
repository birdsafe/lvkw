#ifndef LVKW_EVENT_QUEUE_H_INCLUDED
#define LVKW_EVENT_QUEUE_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

typedef union LVKW_EventPayload {
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
#ifdef LVKW_CONTROLLER_ENABLED
  LVKW_CtrlConnectionEvent controller_connection;
#endif
  LVKW_TextInputEvent text_input;
  LVKW_TextCompositionEvent text_composition;
  LVKW_FocusEvent focus;
  LVKW_DndHoverEvent dnd_hover;
  LVKW_DndLeaveEvent dnd_leave;
  LVKW_DndDropEvent dnd_drop;
} LVKW_EventPayload;

typedef struct LVKW_EventQueue {
  LVKW_EventType *types;
  LVKW_Window **windows;
  LVKW_EventPayload *payloads;

  uint32_t head;
  uint32_t tail;
  uint32_t count;
  uint32_t capacity;
  uint32_t max_capacity;
  double growth_factor;
} LVKW_EventQueue;

LVKW_Status lvkw_event_queue_init(LVKW_Context_Base *ctx, LVKW_EventQueue *q, LVKW_EventTuning tuning);
void lvkw_event_queue_cleanup(LVKW_Context_Base *ctx, LVKW_EventQueue *q);

// Returns true if an event was actually enqueued or merged
bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q, LVKW_EventType type, LVKW_Window* window, const LVKW_Event *evt);

// Pops one event from the queue that matches the mask. Returns true if an event
// was popped. Skips tombstoned events (type == 0) and non-matching events.
bool lvkw_event_queue_pop(LVKW_EventQueue *q, LVKW_EventType mask, LVKW_EventType* out_type, LVKW_Window** out_window, LVKW_Event *evt);

uint32_t lvkw_event_queue_get_count(const LVKW_EventQueue *q);

/** @brief Removes all events associated with a specific window from the queue.
 */
void lvkw_event_queue_remove_window_events(LVKW_EventQueue *q, LVKW_Window *window);

#endif
