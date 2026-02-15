// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_EVENT_QUEUE_H_INCLUDED
#define LVKW_EVENT_QUEUE_H_INCLUDED

#ifdef __cplusplus
#include <atomic>
#define LVKW_ATOMIC(t) std::atomic<t>
extern "C" {
#else
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#define LVKW_ATOMIC(t) _Atomic t
#endif

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"
#include "lvkw/lvkw-telemetry.h"

typedef struct LVKW_QueueBuffer {
  void *data;
  LVKW_EventType *types;
  LVKW_Window **windows;
  LVKW_Event *payloads;
  uint32_t count;
  uint32_t capacity;
} LVKW_QueueBuffer;

typedef struct LVKW_ExternalEvent {
  LVKW_EventType type;
  LVKW_Window *window;
  LVKW_Event payload;
} LVKW_ExternalEvent;

typedef struct LVKW_EventQueue {
  LVKW_Context_Base *ctx;

  LVKW_QueueBuffer buffers[2];
  LVKW_QueueBuffer *active;
  LVKW_QueueBuffer *stable;

  /* Secondary channel for cross-thread events */
  LVKW_ExternalEvent *external;
  uint32_t external_capacity;
  LVKW_ATOMIC(uint32_t) external_head;
  LVKW_ATOMIC(uint32_t) external_tail;

  uint32_t max_capacity;
  double growth_factor;

#ifdef LVKW_GATHER_TELEMETRY
  uint32_t peak_count;
  uint32_t drop_count;
#endif
} LVKW_EventQueue;

#include "lvkw_assume.h"

#if defined(_MSC_VER)
#define LVKW_FORCE_INLINE static __forceinline
#define LVKW_LIKELY(x) (x)
#define LVKW_UNLIKELY(x) (x)
#else
#define LVKW_FORCE_INLINE static inline __attribute__((always_inline))
#define LVKW_LIKELY(x) __builtin_expect(!!(x), 1)
#define LVKW_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

LVKW_Status lvkw_event_queue_init(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                  LVKW_EventTuning tuning);
void lvkw_event_queue_cleanup(LVKW_Context_Base *ctx, LVKW_EventQueue *q);

// Returns true if an event was actually enqueued
LVKW_FORCE_INLINE bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                             LVKW_EventType type, LVKW_Window *window,
                                             const LVKW_Event *evt);

// Returns true if an event was actually enqueued or merged
LVKW_FORCE_INLINE bool lvkw_event_queue_push_compressible(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                                          LVKW_EventType type, LVKW_Window *window,
                                                          const LVKW_Event *evt);

bool _lvkw_event_queue_grow(LVKW_Context_Base *ctx, LVKW_EventQueue *q);

LVKW_FORCE_INLINE bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                             LVKW_EventType type, LVKW_Window *window,
                                             const LVKW_Event *evt) {
  LVKW_CTX_ASSUME(ctx, evt != NULL, "Event payload must not be NULL");

  if (LVKW_UNLIKELY(q->active->count >= q->active->capacity)) {
    if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_TELEMETRY
      q->drop_count++;
#endif
      return false;
    }
  }

  uint32_t idx = q->active->count++;
  q->active->types[idx] = type;
  q->active->windows[idx] = window;
  q->active->payloads[idx] = *evt;

#ifdef LVKW_GATHER_TELEMETRY
  if (q->active->count > q->peak_count) q->peak_count = q->active->count;
#endif

  return true;
}

LVKW_FORCE_INLINE bool lvkw_event_queue_push_compressible(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                                          LVKW_EventType type, LVKW_Window *window,
                                                          const LVKW_Event *evt) {
  LVKW_CTX_ASSUME(ctx, evt != NULL, "Event payload must not be NULL");

  for (int32_t i = (int32_t)q->active->count - 1; i >= 0; --i) {
    if (q->active->windows[i] == window) {
      if (q->active->types[i] == type) {
        if (type == LVKW_EVENT_TYPE_MOUSE_MOTION) {
          q->active->payloads[i].mouse_motion.position = evt->mouse_motion.position;
          q->active->payloads[i].mouse_motion.delta.x += evt->mouse_motion.delta.x;
          q->active->payloads[i].mouse_motion.delta.y += evt->mouse_motion.delta.y;
          q->active->payloads[i].mouse_motion.raw_delta.x += evt->mouse_motion.raw_delta.x;
          q->active->payloads[i].mouse_motion.raw_delta.y += evt->mouse_motion.raw_delta.y;
        } else if (type == LVKW_EVENT_TYPE_MOUSE_SCROLL) {
          q->active->payloads[i].mouse_scroll.delta.x += evt->mouse_scroll.delta.x;
          q->active->payloads[i].mouse_scroll.delta.y += evt->mouse_scroll.delta.y;
        } else {
          q->active->payloads[i] = *evt;
        }
        return true;
      }

      /* If we found an event for the same window that is NOT compressible with this one,
       * we must stop searching to preserve event ordering (e.g., don't move motion
       * after a click).
       */
      break;
    }
  }

  return lvkw_event_queue_push(ctx, q, type, window, evt);
}

// Wait-free push for cross-thread events.
bool lvkw_event_queue_push_external(LVKW_EventQueue *q, LVKW_EventType type, LVKW_Window *window,
                                    const LVKW_Event *evt);

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
