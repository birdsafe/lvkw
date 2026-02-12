#ifndef LVKW_EVENT_QUEUE_H_INCLUDED
#define LVKW_EVENT_QUEUE_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

typedef struct LVKW_EventQueue {
  LVKW_Event *pool;
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
bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q, const LVKW_Event *evt);

// Pops one event from the queue that matches the mask. Returns true if an event
// was popped. Skips tombstoned events (type == 0) and non-matching events.
bool lvkw_event_queue_pop(LVKW_EventQueue *q, LVKW_EventType mask, LVKW_Event *out_evt);

uint32_t lvkw_event_queue_get_count(const LVKW_EventQueue *q);

/** @brief Removes all events associated with a specific window from the queue.
 */
void lvkw_event_queue_remove_window_events(LVKW_EventQueue *q, LVKW_Window *window);

#endif
