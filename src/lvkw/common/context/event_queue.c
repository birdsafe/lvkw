// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "event_queue.h"

#include <stdatomic.h>
#include <stdint.h>
#include <string.h>

static void _lvkw_event_queue_mark_context_lost(LVKW_Context_Base *ctx_base) {
  if (!ctx_base || (ctx_base->pub.flags & LVKW_CONTEXT_STATE_LOST)) return;
  ctx_base->pub.flags |= LVKW_CONTEXT_STATE_LOST;

  LVKW_Window_Base *curr = ctx_base->prv.window_list;
  while (curr) {
    curr->pub.flags |= LVKW_WINDOW_STATE_LOST;
    curr = curr->prv.next;
  }
}

static bool _lvkw_calculate_buffer_layout(uint32_t capacity, size_t *out_windows_offset,
                                          size_t *out_payloads_offset, size_t *out_total_size) {
  const uint64_t cap = (uint64_t)capacity;
  const uint64_t align_mask = 63ull;

  const uint64_t types_bytes = (uint64_t)sizeof(LVKW_EventType) * cap;
  const uint64_t windows_bytes = (uint64_t)sizeof(LVKW_Window *) * cap;
  const uint64_t payloads_bytes = (uint64_t)sizeof(LVKW_Event) * cap;

  const uint64_t windows_offset = (types_bytes + align_mask) & ~align_mask;
  const uint64_t payloads_offset = (windows_offset + windows_bytes + align_mask) & ~align_mask;
  const uint64_t total_size = payloads_offset + payloads_bytes;

  if (total_size > (uint64_t)SIZE_MAX) return false;

  *out_windows_offset = (size_t)windows_offset;
  *out_payloads_offset = (size_t)payloads_offset;
  *out_total_size = (size_t)total_size;
  return true;
}

static bool _lvkw_buffer_alloc(LVKW_Context_Base *ctx, LVKW_QueueBuffer *buf, uint32_t capacity) {
  size_t windows_offset = 0;
  size_t payloads_offset = 0;
  size_t total_size = 0;
  if (!_lvkw_calculate_buffer_layout(capacity, &windows_offset, &payloads_offset, &total_size)) {
    memset(buf, 0, sizeof(LVKW_QueueBuffer));
    return false;
  }

  buf->data = lvkw_context_alloc_aligned64(ctx, total_size);
  if (!buf->data) {
    memset(buf, 0, sizeof(LVKW_QueueBuffer));
    return false;
  }

  buf->types = (LVKW_EventType *)buf->data;
  buf->windows = (LVKW_Window **)((uint8_t *)buf->data + windows_offset);
  buf->payloads = (LVKW_Event *)((uint8_t *)buf->data + payloads_offset);
  buf->count = 0;
  buf->capacity = capacity;

  _lvkw_transient_pool_init(&buf->transient_pool);

  return true;
}

static void _lvkw_buffer_free(LVKW_Context_Base *ctx, LVKW_QueueBuffer *buf) {
  if (buf->data) {
    _lvkw_transient_pool_destroy(&buf->transient_pool, ctx);
    lvkw_context_free_aligned64(ctx, buf->data);
  }
  memset(buf, 0, sizeof(LVKW_QueueBuffer));
}

bool _lvkw_event_queue_grow(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  uint32_t new_capacity = (uint32_t)((double)q->active->capacity * q->growth_factor);
  if (new_capacity > q->max_capacity) new_capacity = q->max_capacity;
  if (new_capacity <= q->active->capacity) return false;

  LVKW_QueueBuffer new_buf;
  if (!_lvkw_buffer_alloc(ctx, &new_buf, new_capacity)) return false;

  // 2. Copy current active data to the new larger buffer.
  if (q->active->count > 0) {
    memcpy(new_buf.types, q->active->types, sizeof(LVKW_EventType) * q->active->count);
    memcpy(new_buf.windows, q->active->windows, sizeof(LVKW_Window *) * q->active->count);
    memcpy(new_buf.payloads, q->active->payloads, sizeof(LVKW_Event) * q->active->count);
    new_buf.count = q->active->count;
  }

  // Transfer the transient pool ownership
  _lvkw_transient_pool_destroy(&new_buf.transient_pool, ctx);
  new_buf.transient_pool = q->active->transient_pool;
  
  // Clear the old pool's pointers so _lvkw_buffer_free doesn't destroy the shared resources
  _lvkw_transient_pool_init(&q->active->transient_pool);

  // 3. Swap it out.
  _lvkw_buffer_free(ctx, q->active);
  *q->active = new_buf;

  return true;
}

LVKW_Status lvkw_event_queue_init(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                  LVKW_EventTuning tuning) {
  memset(q, 0, sizeof(LVKW_EventQueue));

  if (tuning.initial_capacity == 0u || tuning.max_capacity == 0u || tuning.external_capacity == 0u ||
      tuning.max_capacity < tuning.initial_capacity || tuning.growth_factor <= (LVKW_Scalar)1.0) {
    return LVKW_ERROR;
  }

  q->ctx = ctx;
  q->max_capacity = tuning.max_capacity;
  q->growth_factor = tuning.growth_factor;
  q->external_capacity = tuning.external_capacity;

  for (int i = 0; i < 2; ++i) {
    if (!_lvkw_buffer_alloc(ctx, &q->buffers[i], tuning.initial_capacity)) {
      lvkw_event_queue_cleanup(ctx, q);
      return LVKW_ERROR;
    }
  }

  uint64_t external_size_u64 = (uint64_t)sizeof(LVKW_ExternalEvent) * (uint64_t)q->external_capacity;
  if (external_size_u64 > (uint64_t)SIZE_MAX) {
    lvkw_event_queue_cleanup(ctx, q);
    return LVKW_ERROR;
  }
  size_t external_size = (size_t)external_size_u64;
  q->external = lvkw_context_alloc(ctx, external_size);
  if (!q->external) {
    lvkw_event_queue_cleanup(ctx, q);
    return LVKW_ERROR;
  }

  q->active = &q->buffers[0];
  q->stable = &q->buffers[1];
  atomic_store_explicit(&q->commit_id, 0u, memory_order_relaxed);


  return LVKW_SUCCESS;
}

void lvkw_event_queue_cleanup(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  for (int i = 0; i < 2; ++i) {
    _lvkw_buffer_free(ctx, &q->buffers[i]);
  }
  if (q->external) lvkw_context_free(ctx, q->external);
  memset(q, 0, sizeof(LVKW_EventQueue));
}

uint64_t lvkw_event_queue_get_commit_id(const LVKW_EventQueue *q) {
  return LVKW_ATOMIC_LOAD_RELAXED(&q->commit_id);
}

void lvkw_event_queue_note_commit_success(LVKW_EventQueue *q) {
  (void)LVKW_ATOMIC_FETCH_ADD_RELAXED(&q->commit_id, 1u);
}

uint32_t lvkw_event_queue_get_count(const LVKW_EventQueue *q) { return q->stable->count; }

void lvkw_event_queue_flush(LVKW_EventQueue *q) {
  #ifdef LVKW_GATHER_METRICS
  if (q->active->count > q->peak_count) q->peak_count = q->active->count;
#endif
  q->active->count = 0;
}

bool lvkw_event_queue_push_external(LVKW_EventQueue *q, LVKW_EventType type, LVKW_Window *window,
                                    const LVKW_Event *evt) {
  uint32_t reserve_tail;
  uint32_t head;

  do {
    reserve_tail = atomic_load_explicit(&q->external_reserve_tail, memory_order_relaxed);
    head = atomic_load_explicit(&q->external_head, memory_order_acquire);
    if (reserve_tail - head >= q->external_capacity) {
      return false;  // Queue full
    }
  } while (!atomic_compare_exchange_weak_explicit(
      &q->external_reserve_tail, &reserve_tail, reserve_tail + 1, memory_order_acq_rel,
      memory_order_relaxed));

  LVKW_ExternalEvent *slot = &q->external[reserve_tail % q->external_capacity];
  slot->type = type;
  slot->window = window;
  if (evt)
    slot->payload = *evt;
  else
    memset(&slot->payload, 0, sizeof(slot->payload));

  while (atomic_load_explicit(&q->external_tail, memory_order_acquire) != reserve_tail) {
  }
  atomic_store_explicit(&q->external_tail, reserve_tail + 1, memory_order_release);

  return true;
}

void lvkw_event_queue_begin_gather(LVKW_EventQueue *q) {
  // 1. Drain external queue into active buffer
  uint32_t head = atomic_load_explicit(&q->external_head, memory_order_relaxed);
  uint32_t tail = atomic_load_explicit(&q->external_tail, memory_order_acquire);

  while (head != tail) {
    LVKW_ExternalEvent *slot = &q->external[head % q->external_capacity];
    // External events were already mask-filtered at API ingress.
    lvkw_event_queue_push_unfiltered(q->ctx, q, slot->type, slot->window, &slot->payload);
    head++;
  }
  atomic_store_explicit(&q->external_head, head, memory_order_release);

#ifdef LVKW_GATHER_METRICS
  if (q->active->count > q->peak_count) q->peak_count = q->active->count;
#endif

  // 2. Swap: stable <-> active
  LVKW_QueueBuffer *tmp = q->stable;
  q->stable = q->active;
  q->active = tmp;

  q->active->count = 0;
  _lvkw_transient_pool_clear(&q->active->transient_pool, q->ctx);

  /* If there was growth in the interim, catch up the buffer that just rotated out of stable.
   */
  if (q->active->capacity < q->stable->capacity) {
    LVKW_QueueBuffer replacement;
    if (!_lvkw_buffer_alloc(q->ctx, &replacement, q->stable->capacity)) {
      _lvkw_event_queue_mark_context_lost(q->ctx);
      return;
    }
    _lvkw_buffer_free(q->ctx, q->active);
    *q->active = replacement;
  }
}

void lvkw_event_queue_scan(const LVKW_EventQueue *q, LVKW_EventType mask,
                           LVKW_EventCallback callback, void *userdata) {

  // TODO: Investigate the possibility of SIMD sheananigans here.

  const LVKW_QueueBuffer *buf = q->stable;
  uint32_t count = buf->count;
  for (uint32_t i = 0; i < count; ++i) {
    if (buf->types[i] & mask) {
      callback(buf->types[i], buf->windows[i], &buf->payloads[i], userdata);
    }
  }
}

void lvkw_event_queue_remove_window_events(LVKW_EventQueue *q, LVKW_Window *window) {
  LVKW_QueueBuffer *targets[] = {q->active, q->stable};

  for (int i = 0; i < 2; ++i) {
    LVKW_QueueBuffer *buf = targets[i];
    uint32_t write_idx = 0;
    for (uint32_t read_idx = 0; read_idx < buf->count; ++read_idx) {
      if (buf->windows[read_idx] != window) {
        if (write_idx != read_idx) {
          buf->types[write_idx] = buf->types[read_idx];
          buf->windows[write_idx] = buf->windows[read_idx];
          buf->payloads[write_idx] = buf->payloads[read_idx];
        }
        write_idx++;
      }
    }
    buf->count = write_idx;
  }
}

void *lvkw_event_queue_transient_alloc(LVKW_EventQueue *q, size_t size) {
  return _lvkw_transient_pool_alloc(&q->active->transient_pool, q->ctx, size);
}

const char *lvkw_event_queue_transient_intern(LVKW_EventQueue *q, const char *str) {
  return _lvkw_transient_pool_intern(&q->active->transient_pool, q->ctx, str);
}

const char *lvkw_event_queue_transient_intern_sized(LVKW_EventQueue *q, const char *str, size_t len) {
  return _lvkw_transient_pool_intern_sized(&q->active->transient_pool, q->ctx, str, len);
}

void lvkw_event_queue_get_metrics(LVKW_EventQueue *q, LVKW_EventMetrics *out_metrics,
                                    bool reset) {
#ifdef LVKW_GATHER_METRICS
  memset(out_metrics, 0, sizeof(LVKW_EventMetrics));
  out_metrics->current_capacity = q->active->capacity;


  out_metrics->peak_count = q->peak_count;
  out_metrics->drop_count = q->drop_count;

  if (reset) {
    q->peak_count = 0;
    q->drop_count = 0;
  }
#endif
}
