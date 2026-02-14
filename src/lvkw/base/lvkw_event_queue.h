// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_EVENT_QUEUE_H_INCLUDED
#define LVKW_EVENT_QUEUE_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"
#include "lvkw/lvkw-telemetry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LVKW_QueueBuffer {
  void *data;
  LVKW_EventType *types;
  LVKW_Window **windows;
  LVKW_Event *payloads;
  uint32_t count;
  uint32_t capacity;
} LVKW_QueueBuffer;

typedef struct LVKW_EventQueue {
  LVKW_Context_Base *ctx;

  LVKW_QueueBuffer buffers[3];
  LVKW_QueueBuffer *active;
  LVKW_QueueBuffer *stable;
  LVKW_QueueBuffer *spare;

  uint32_t max_capacity;
  double growth_factor;

#ifdef LVKW_GATHER_TELEMETRY
  uint32_t peak_count;
  uint32_t drop_count;
#endif
} LVKW_EventQueue;

LVKW_Status lvkw_event_queue_init(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                  LVKW_EventTuning tuning);
void lvkw_event_queue_cleanup(LVKW_Context_Base *ctx, LVKW_EventQueue *q);

// Returns true if an event was actually enqueued or merged
bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q, LVKW_EventType type,
                           LVKW_Window *window, const LVKW_Event *evt);

// Flushes stable events and promotes pending ones.
void lvkw_event_queue_begin_gather(LVKW_EventQueue *q);

void lvkw_event_queue_flush(LVKW_EventQueue *q);

void lvkw_event_queue_scan(const LVKW_EventQueue *q, LVKW_EventType mask,
                           LVKW_EventCallback callback, void *userdata);
                           
uint32_t lvkw_event_queue_get_count(const LVKW_EventQueue *q);

/** @brief Removes all events associated with a specific window from the queue.
 */
void lvkw_event_queue_remove_window_events(LVKW_EventQueue *q, LVKW_Window *window);


void lvkw_event_queue_get_telemetry(LVKW_EventQueue *q, LVKW_EventTelemetry *out_telemetry,
                                    bool reset);

#ifdef __cplusplus
}
#endif

#endif
