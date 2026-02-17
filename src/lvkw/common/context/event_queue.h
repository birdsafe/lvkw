// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_EVENT_QUEUE_H_INCLUDED
#define LVKW_EVENT_QUEUE_H_INCLUDED

#include "lvkw/lvkw.h"
#include "internal.h"
#include "lvkw/c/instrumentation.h"

#include "assume.h"
#include <limits.h>
#include <stdlib.h>

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

#ifdef __cplusplus
#define LVKW_ATOMIC_LOAD_RELAXED(ptr) std::atomic_load_explicit((ptr), std::memory_order_relaxed)
#else
#define LVKW_ATOMIC_LOAD_RELAXED(ptr) atomic_load_explicit((ptr), memory_order_relaxed)
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

typedef struct _LVKW_EvictBucket {
  LVKW_Window *window;
  LVKW_EventType type;
  uint32_t count;
  uint32_t target_evict;
  uint32_t evicted;
  uint32_t older_seen;
  int32_t newest_idx;
  uint8_t used;
} _LVKW_EvictBucket;

LVKW_FORCE_INLINE uint32_t _lvkw_mix_bucket_hash(LVKW_Window *window, LVKW_EventType type,
                                                  uint32_t mask) {
  const uintptr_t w = (uintptr_t)window;
  uint64_t h = (uint64_t)w;
  h ^= h >> 33;
  h *= 0xff51afd7ed558ccdULL;
  h ^= h >> 33;
  h ^= (uint64_t)(uint32_t)type * 0x9e3779b97f4a7c15ULL;
  return (uint32_t)h & mask;
}

LVKW_FORCE_INLINE _LVKW_EvictBucket *_lvkw_find_bucket(_LVKW_EvictBucket *table,
                                                        uint32_t mask, LVKW_Window *window,
                                                        LVKW_EventType type) {
  uint32_t idx = _lvkw_mix_bucket_hash(window, type, mask);
  for (;;) {
    _LVKW_EvictBucket *slot = &table[idx];
    if (!slot->used || (slot->window == window && slot->type == type)) {
      return slot;
    }
    idx = (idx + 1u) & mask;
  }
}

LVKW_FORCE_INLINE uint32_t _lvkw_event_queue_compact_half_by_type_window(LVKW_QueueBuffer *qb) {
  if (qb->count < 2u) {
    return 0u;
  }

  uint32_t compressible_count = 0;
  for (uint32_t i = 0; i < qb->count; ++i) {
    if ((uint32_t)qb->types[i] & (uint32_t)LVKW_COMPRESSIBLE_EVENT_MASK) {
      compressible_count++;
    }
  }

  if (compressible_count == 0u) {
    return 0u;
  }

  uint32_t table_capacity = 16u;
  while (table_capacity < (compressible_count << 1u)) {
    table_capacity <<= 1u;
  }

  _LVKW_EvictBucket stack_table[64];
  _LVKW_EvictBucket *table = NULL;
  bool needs_heap = table_capacity > (uint32_t)(sizeof(stack_table) / sizeof(stack_table[0]));
  if (needs_heap) {
    table = (_LVKW_EvictBucket *)malloc(sizeof(_LVKW_EvictBucket) * table_capacity);
    if (!table) {
      return _lvkw_event_queue_evict_oldest_compressible_once(qb);
    }
  } else {
    table = stack_table;
  }

  memset(table, 0, sizeof(_LVKW_EvictBucket) * table_capacity);
  const uint32_t mask = table_capacity - 1u;

  for (uint32_t i = 0; i < qb->count; ++i) {
    LVKW_EventType type = qb->types[i];
    if (!((uint32_t)type & (uint32_t)LVKW_COMPRESSIBLE_EVENT_MASK)) {
      continue;
    }

    _LVKW_EvictBucket *bucket = _lvkw_find_bucket(table, mask, qb->windows[i], type);
    if (!bucket->used) {
      bucket->used = 1u;
      bucket->window = qb->windows[i];
      bucket->type = type;
      bucket->newest_idx = (int32_t)i;
      bucket->count = 1u;
    } else {
      bucket->count++;
      bucket->newest_idx = (int32_t)i;
    }
  }

  for (uint32_t i = 0; i < table_capacity; ++i) {
    if (table[i].used) {
      table[i].target_evict = table[i].count / 2u;
    }
  }

  uint32_t write_idx = 0;
  uint32_t total_evicted = 0;

  for (uint32_t read_idx = 0; read_idx < qb->count; ++read_idx) {
    LVKW_EventType type = qb->types[read_idx];
    bool drop = false;

    if ((uint32_t)type & (uint32_t)LVKW_COMPRESSIBLE_EVENT_MASK) {
      _LVKW_EvictBucket *bucket = _lvkw_find_bucket(table, mask, qb->windows[read_idx], type);
      if ((int32_t)read_idx != bucket->newest_idx) {
        uint32_t pos = bucket->older_seen++;
        if ((pos & 1u) == 0u && bucket->evicted < bucket->target_evict) {
          drop = true;
          bucket->evicted++;
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
  if (needs_heap) {
    free(table);
  }
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
                                         const LVKW_Event *evt);
static inline bool lvkw_event_queue_push_compressible(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                                      LVKW_EventType type, LVKW_Window *window,
                                                      const LVKW_Event *evt);
static inline bool lvkw_event_queue_push_compressible_with_mask(LVKW_Context_Base *ctx,
                                                                 LVKW_EventQueue *q, uint32_t mask,
                                                                 LVKW_EventType type,
                                                                 LVKW_Window *window,
                                                                 const LVKW_Event *evt);

LVKW_FORCE_INLINE bool _lvkw_event_queue_mask_allows(uint32_t mask, LVKW_EventType type) {
  return (mask & (uint32_t)type) != 0u;
}

static inline bool lvkw_event_queue_push_unfiltered(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                                    LVKW_EventType type, LVKW_Window *window,
                                                    const LVKW_Event *evt) {
  LVKW_CONTEXT_ASSUME(ctx, evt != NULL, "Event payload must not be NULL");

  LVKW_QueueBuffer *qb = q->active;
  uint32_t count = qb->count;

  if (LVKW_UNLIKELY(count >= qb->capacity)) {
    if (_lvkw_event_queue_reclaim_compressible(qb) == 0u) {
      if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_METRICS
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

#ifdef LVKW_GATHER_METRICS
  if (qb->count > q->peak_count) q->peak_count = qb->count;
#endif

  return true;
}

static inline bool lvkw_event_queue_push_with_mask(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                                   uint32_t mask, LVKW_EventType type,
                                                   LVKW_Window *window, const LVKW_Event *evt) {
  if (LVKW_UNLIKELY(!_lvkw_event_queue_mask_allows(mask, type))) {
    return false;
  }
  return lvkw_event_queue_push_unfiltered(ctx, q, type, window, evt);
}

static inline bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                         LVKW_EventType type, LVKW_Window *window,
                                         const LVKW_Event *evt) {
  uint32_t event_mask = LVKW_ATOMIC_LOAD_RELAXED(&ctx->prv.event_mask);
  return lvkw_event_queue_push_with_mask(ctx, q, event_mask, type, window, evt);
}

static inline bool lvkw_event_queue_push_compressible(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                                      LVKW_EventType type, LVKW_Window *window,
                                                      const LVKW_Event *evt) {
  uint32_t event_mask = LVKW_ATOMIC_LOAD_RELAXED(&ctx->prv.event_mask);
  return lvkw_event_queue_push_compressible_with_mask(ctx, q, event_mask, type, window, evt);
}

static inline bool lvkw_event_queue_push_compressible_with_mask(LVKW_Context_Base *ctx,
                                                                 LVKW_EventQueue *q, uint32_t mask,
                                                                 LVKW_EventType type,
                                                                 LVKW_Window *window,
                                                                 const LVKW_Event *evt) {
  LVKW_CONTEXT_ASSUME(ctx, type & LVKW_COMPRESSIBLE_EVENT_MASK, "Only compressible events should be here");
  LVKW_CONTEXT_ASSUME(ctx, evt != NULL, "Event payload must not be NULL");

  if (LVKW_UNLIKELY(!_lvkw_event_queue_mask_allows(mask, type))) {
    return false;
  }

  
  LVKW_QueueBuffer * qb = q->active;
  LVKW_Window **const windows = qb->windows;
  LVKW_EventType *const types = qb->types;
  LVKW_Event *const payloads = qb->payloads;

  for (int32_t i = (int32_t)qb->count - 1; i >= 0; --i) {
    if (windows[i] == window && types[i] == type) {
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
        target->mouse_scroll.steps.x += evt->mouse_scroll.steps.x;
        target->mouse_scroll.steps.y += evt->mouse_scroll.steps.y;
      }
      else {
        *target = *evt;
      }
      return true;
    }

    /* If we found an event for the same window that is NOT compressible with this one,
      * we must stop searching to preserve event ordering.
      */
    if (windows[i] == window) {
      break;
    }
  }

  uint32_t count = qb->count;
  if (LVKW_UNLIKELY(count >= qb->capacity)) {
    if (_lvkw_event_queue_reclaim_compressible(qb) == 0u) {
      if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_METRICS
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

#ifdef LVKW_GATHER_METRICS
  if (qb->count > q->peak_count) q->peak_count = qb->count;
#endif

  return true;
}

// Lock-free push for cross-thread events.
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

void *lvkw_event_queue_transient_alloc(LVKW_EventQueue *q, size_t size);
const char *lvkw_event_queue_transient_intern(LVKW_EventQueue *q, const char *str);
const char *lvkw_event_queue_transient_intern_sized(LVKW_EventQueue *q, const char *str, size_t len);

void lvkw_event_queue_get_metrics(LVKW_EventQueue *q, LVKW_EventMetrics *out_metrics,
                                    bool reset);

#ifdef __cplusplus
}
#endif

#endif
