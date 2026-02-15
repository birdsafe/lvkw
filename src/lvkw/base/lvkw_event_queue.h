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

#define LVKW_COMPRESSIBLE_EVENT_MASK ( \
      LVKW_EVENT_TYPE_MOUSE_MOTION \
    | LVKW_EVENT_TYPE_MOUSE_SCROLL \
    | LVKW_EVENT_TYPE_WINDOW_RESIZED )

#define LVKW_QUEUE_EVICT_STRATEGY_OLDEST_ONLY 0u
#define LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE 1u
#define LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE_WINDOW 2u

#ifndef LVKW_QUEUE_EVICT_STRATEGY
#define LVKW_QUEUE_EVICT_STRATEGY LVKW_QUEUE_EVICT_STRATEGY_OLDEST_ONLY
#endif

LVKW_FORCE_INLINE int _lvkw_event_queue_compressible_bucket(LVKW_EventType type) {
  switch (type) {
    case LVKW_EVENT_TYPE_MOUSE_MOTION: return 0;
    case LVKW_EVENT_TYPE_MOUSE_SCROLL: return 1;
    case LVKW_EVENT_TYPE_WINDOW_RESIZED: return 2;
    default: return -1;
  }
}

LVKW_FORCE_INLINE uint32_t _lvkw_event_queue_evict_oldest_compressible_once(LVKW_QueueBuffer *qb) {
  for (uint32_t i = 0; i < qb->count; ++i) {
    if ((uint32_t)qb->types[i] & (uint32_t)LVKW_COMPRESSIBLE_EVENT_MASK) {
      for (uint32_t j = i + 1; j < qb->count; ++j) {
        qb->types[j - 1] = qb->types[j];
        qb->windows[j - 1] = qb->windows[j];
        qb->payloads[j - 1] = qb->payloads[j];
      }
      qb->count--;
      return 1u;
    }
  }
  return 0u;
}

LVKW_FORCE_INLINE uint32_t _lvkw_event_queue_compact_half_by_type(LVKW_QueueBuffer *qb) {
  uint32_t counts[3] = {0, 0, 0};
  int32_t newest_idx[3] = {-1, -1, -1};

  for (uint32_t i = 0; i < qb->count; ++i) {
    int b = _lvkw_event_queue_compressible_bucket(qb->types[i]);
    if (b >= 0) {
      counts[b]++;
      newest_idx[b] = (int32_t)i;
    }
  }

  uint32_t target_evict[3] = {
    counts[0] / 2u,
    counts[1] / 2u,
    counts[2] / 2u,
  };

  if ((target_evict[0] | target_evict[1] | target_evict[2]) == 0u) {
    return 0u;
  }

  uint32_t evicted[3] = {0, 0, 0};
  uint32_t older_seen[3] = {0, 0, 0};
  uint32_t write_idx = 0;

  for (uint32_t read_idx = 0; read_idx < qb->count; ++read_idx) {
    int b = _lvkw_event_queue_compressible_bucket(qb->types[read_idx]);
    bool drop = false;

    if (b >= 0 && (int32_t)read_idx != newest_idx[b]) {
      uint32_t pos = older_seen[b]++;
      if ((pos & 1u) == 0u && evicted[b] < target_evict[b]) {
        drop = true;
        evicted[b]++;
      }
    }

    if (!drop) {
      if (write_idx != read_idx) {
        qb->types[write_idx] = qb->types[read_idx];
        qb->windows[write_idx] = qb->windows[read_idx];
        qb->payloads[write_idx] = qb->payloads[read_idx];
      }
      write_idx++;
    }
  }

  qb->count = write_idx;
  return evicted[0] + evicted[1] + evicted[2];
}

LVKW_FORCE_INLINE uint32_t _lvkw_event_queue_compact_half_by_type_window(LVKW_QueueBuffer *qb) {
  uint32_t write_idx = 0;
  uint32_t total_evicted = 0;

  for (uint32_t read_idx = 0; read_idx < qb->count; ++read_idx) {
    LVKW_EventType type = qb->types[read_idx];
    LVKW_Window *window = qb->windows[read_idx];
    bool drop = false;

    if ((uint32_t)type & (uint32_t)LVKW_COMPRESSIBLE_EVENT_MASK) {
      uint32_t bucket_total = 0;
      int32_t newest_idx = -1;
      uint32_t rank = 0;

      for (uint32_t i = 0; i < qb->count; ++i) {
        if (qb->types[i] == type && qb->windows[i] == window) {
          bucket_total++;
          newest_idx = (int32_t)i;
          if (i < read_idx) {
            rank++;
          }
        }
      }

      if (bucket_total >= 2u && (int32_t)read_idx != newest_idx) {
        uint32_t target = bucket_total / 2u;
        uint32_t evicted_before = (rank + 1u) / 2u;
        if ((rank & 1u) == 0u && evicted_before < target) {
          drop = true;
          total_evicted++;
        }
      }
    }

    if (!drop) {
      if (write_idx != read_idx) {
        qb->types[write_idx] = qb->types[read_idx];
        qb->windows[write_idx] = qb->windows[read_idx];
        qb->payloads[write_idx] = qb->payloads[read_idx];
      }
      write_idx++;
    }
  }

  qb->count = write_idx;
  return total_evicted;
}

LVKW_FORCE_INLINE uint32_t _lvkw_event_queue_reclaim_compressible(LVKW_QueueBuffer *qb) {
#if LVKW_QUEUE_EVICT_STRATEGY == LVKW_QUEUE_EVICT_STRATEGY_OLDEST_ONLY
  return _lvkw_event_queue_evict_oldest_compressible_once(qb);
#elif LVKW_QUEUE_EVICT_STRATEGY == LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE
  return _lvkw_event_queue_compact_half_by_type(qb);
#elif LVKW_QUEUE_EVICT_STRATEGY == LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE_WINDOW
  return _lvkw_event_queue_compact_half_by_type_window(qb);
#else
#error "Invalid LVKW_QUEUE_EVICT_STRATEGY"
#endif
}

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
    if (_lvkw_event_queue_reclaim_compressible(qb) == 0u) {
      if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_TELEMETRY
        q->drop_count++;
#endif
        return false;
      }
      qb = q->active;
    }
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
    if (_lvkw_event_queue_reclaim_compressible(qb) == 0u) {
      if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_TELEMETRY
        q->drop_count++;
#endif
        return false;
      }
      qb = q->active;
    }
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
