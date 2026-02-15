// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_EVENT_QUEUE_H_INCLUDED
#define LVKW_EVENT_QUEUE_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"
#include "lvkw/lvkw-telemetry.h"

#include "lvkw_assume.h"

#ifdef __cplusplus
extern "C" {
#endif

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

// // Returns true if an event was actually enqueued
// static inline bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
//                                              LVKW_EventType type, LVKW_Window *window,
//                                              const LVKW_Event *evt);

// // Returns true if an event was actually enqueued or merged
// static inline bool lvkw_event_queue_push_compressible(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
//                                                           LVKW_EventType type, LVKW_Window *window,
//                                                           const LVKW_Event *evt);

bool _lvkw_event_queue_grow(LVKW_Context_Base *ctx, LVKW_EventQueue *q);

static inline bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                             LVKW_EventType type, LVKW_Window *window,
                                             const LVKW_Event *evt) {
  LVKW_CTX_ASSUME(ctx, evt != NULL, "Event payload must not be NULL");

  if (LVKW_UNLIKELY(!(ctx->prv.event_mask & type))) {
    return false;
  }

  LVKW_QueueBuffer * qb = q->active;
  uint32_t count = qb->count;

  if (LVKW_UNLIKELY(count >= qb->capacity)) {
    if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_TELEMETRY
      q->drop_count++;
#endif
      return false;
    }
    qb = q->active;
  }

  const uint32_t idx = qb->count++;

  qb->types[idx] = type;
  qb->windows[idx] = window;
  qb->payloads[idx] = *evt;

#ifdef LVKW_GATHER_TELEMETRY
  if (qb->count > q->peak_count) q->peak_count = qb->count;
#endif

  return true;
}

#define LVKW_COMPRESSIBLE_EVENT_MASK ( \
      LVKW_EVENT_TYPE_MOUSE_MOTION \
    | LVKW_EVENT_TYPE_MOUSE_SCROLL \
    | LVKW_EVENT_TYPE_WINDOW_RESIZED ) 

static inline bool lvkw_event_queue_push_compressible(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                                          LVKW_EventType type, LVKW_Window *window,
                                                          const LVKW_Event *evt) {
  LVKW_CTX_ASSUME(ctx, type & LVKW_COMPRESSIBLE_EVENT_MASK, "Only compressible events should be here");
  LVKW_CTX_ASSUME(ctx, evt != NULL, "Event payload must not be NULL");

  if (LVKW_UNLIKELY(!(ctx->prv.event_mask & type))) {
    return false;
  }

  LVKW_QueueBuffer * qb = q->active;
  LVKW_Window **const windows = qb->windows;
  LVKW_EventType *const types = qb->types;
  LVKW_Event *const payloads = qb->payloads;

  for (int32_t i = (int32_t)qb->count - 1; i >= 0; --i) {
    if (windows[i] == window) {
      if (types[i] == type) {
        LVKW_Event *const target = &payloads[i];
        if (type == LVKW_EVENT_TYPE_MOUSE_MOTION) {
          target->mouse_motion.position = evt->mouse_motion.position;
          target->mouse_motion.delta.x += evt->mouse_motion.delta.x;
          target->mouse_motion.delta.y += evt->mouse_motion.delta.y;
          target->mouse_motion.raw_delta.x += evt->mouse_motion.raw_delta.x;
          target->mouse_motion.raw_delta.y += evt->mouse_motion.raw_delta.y;
        }
        else if (type == LVKW_EVENT_TYPE_MOUSE_SCROLL) {
          target->mouse_scroll.delta.x += evt->mouse_scroll.delta.x;
          target->mouse_scroll.delta.y += evt->mouse_scroll.delta.y;
        }
        else {
          *target = *evt;
        }
        return true;
      }

      /* If we found an event for the same window that is NOT compressible with this one,
       * we must stop searching to preserve event ordering.
       */
      break;
    }
  }

  // Fall back to standard push logic (inlined for performance)
  uint32_t count = qb->count;
  if (LVKW_UNLIKELY(count >= qb->capacity)) {
    if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_TELEMETRY
      q->drop_count++;
#endif
      return false;
    }
    qb = q->active;
  }

  const uint32_t idx = qb->count++;
  qb->types[idx] = type;
  qb->windows[idx] = window;
  qb->payloads[idx] = *evt;

#ifdef LVKW_GATHER_TELEMETRY
  if (qb->count > q->peak_count) q->peak_count = qb->count;
#endif

  return true;
}

// Wait-free push for cross-thread events.
bool lvkw_event_queue_push_external(LVKW_EventQueue *q, LVKW_EventType type,
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
