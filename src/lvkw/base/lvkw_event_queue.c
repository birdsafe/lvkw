// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_event_queue.h"

#include <string.h>

static bool _lvkw_buffer_alloc(LVKW_Context_Base *ctx, LVKW_QueueBuffer *buf, uint32_t capacity) {
  size_t types_offset = 0;
  size_t windows_offset = (types_offset + sizeof(LVKW_EventType) * capacity + 63) & ~63;
  size_t payloads_offset = (windows_offset + sizeof(LVKW_Window *) * capacity + 63) & ~63;
  size_t total_size = payloads_offset + sizeof(LVKW_Event) * capacity;

  buf->data = lvkw_context_alloc_aligned64(ctx, total_size);
  if (!buf->data) {
    memset(buf, 0, sizeof(LVKW_QueueBuffer));
    return false;
  }

  buf->types = (LVKW_EventType *)((uint8_t *)buf->data + types_offset);
  buf->windows = (LVKW_Window **)((uint8_t *)buf->data + windows_offset);
  buf->payloads = (LVKW_Event *)((uint8_t *)buf->data + payloads_offset);
  buf->count = 0;
  buf->capacity = capacity;

  return true;
}

static void _lvkw_buffer_free(LVKW_Context_Base *ctx, LVKW_QueueBuffer *buf) {
  if (buf->data) lvkw_context_free_aligned64(ctx, buf->data);
  memset(buf, 0, sizeof(LVKW_QueueBuffer));
}

static bool _is_event_compressible(LVKW_EventType type) {
  return type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
         type == LVKW_EVENT_TYPE_WINDOW_RESIZED;
}

static bool _lvkw_event_queue_grow(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  uint32_t new_capacity = (uint32_t)((double)q->active->capacity * q->growth_factor);
  if (new_capacity > q->max_capacity) new_capacity = q->max_capacity;
  if (new_capacity <= q->active->capacity) return false;

  // 1. Allocate a brand new buffer that is larger.
  LVKW_QueueBuffer new_buf;
  if (!_lvkw_buffer_alloc(ctx, &new_buf, new_capacity)) return false;

  // 2. Also ensure spare is large enough for the NEXT rotation.
  _lvkw_buffer_free(ctx, q->spare);
  if (!_lvkw_buffer_alloc(ctx, q->spare, new_capacity)) {
    _lvkw_buffer_free(ctx, &new_buf);
    return false;
  }

  // 3. Copy current active data to the new larger buffer.
  if (q->active->count > 0) {
    memcpy(new_buf.types, q->active->types, sizeof(LVKW_EventType) * q->active->count);
    memcpy(new_buf.windows, q->active->windows, sizeof(LVKW_Window *) * q->active->count);
    memcpy(new_buf.payloads, q->active->payloads, sizeof(LVKW_Event) * q->active->count);
    new_buf.count = q->active->count;
  }

  // 4. Swap everything out.
  _lvkw_buffer_free(ctx, q->active);
  *q->active = new_buf;

  return true;
}

LVKW_Status lvkw_event_queue_init(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                  LVKW_EventTuning tuning) {
  memset(q, 0, sizeof(LVKW_EventQueue));
  q->ctx = ctx;
  q->max_capacity = tuning.max_capacity;
  q->growth_factor = tuning.growth_factor;

  for (int i = 0; i < 3; ++i) {
    if (!_lvkw_buffer_alloc(ctx, &q->buffers[i], tuning.initial_capacity)) {
      lvkw_event_queue_cleanup(ctx, q);
      return LVKW_ERROR;
    }
  }

  q->active = &q->buffers[0];
  q->stable = &q->buffers[1];
  q->spare = &q->buffers[2];

  return LVKW_SUCCESS;
}

void lvkw_event_queue_cleanup(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  for (int i = 0; i < 3; ++i) {
    _lvkw_buffer_free(ctx, &q->buffers[i]);
  }
  memset(q, 0, sizeof(LVKW_EventQueue));
}

uint32_t lvkw_event_queue_get_count(const LVKW_EventQueue *q) { return q->stable->count; }

void lvkw_event_queue_flush(LVKW_EventQueue *q) { q->active->count = 0; }

bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q, LVKW_EventType type,
                           LVKW_Window *window, const LVKW_Event *evt) {
  if (_is_event_compressible(type)) {
    for (int32_t i = (int32_t)q->active->count - 1; i >= 0; --i) {
      if (q->active->windows[i] == window) {
        if (q->active->types[i] == type) {
          if (type == LVKW_EVENT_TYPE_MOUSE_MOTION) {
            q->active->payloads[i].mouse_motion.position = evt->mouse_motion.position;
            q->active->payloads[i].mouse_motion.delta.x += evt->mouse_motion.delta.x;
            q->active->payloads[i].mouse_motion.delta.y += evt->mouse_motion.delta.y;
            q->active->payloads[i].mouse_motion.raw_delta.x += evt->mouse_motion.raw_delta.x;
            q->active->payloads[i].mouse_motion.raw_delta.y += evt->mouse_motion.raw_delta.y;
          }
          else if (type == LVKW_EVENT_TYPE_MOUSE_SCROLL) {
            q->active->payloads[i].mouse_scroll.delta.x += evt->mouse_scroll.delta.x;
            q->active->payloads[i].mouse_scroll.delta.y += evt->mouse_scroll.delta.y;
          }
          else {
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
  }

  if (q->active->count >= q->active->capacity) {
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

void lvkw_event_queue_begin_gather(LVKW_EventQueue *q) {
  // Rotate: stable -> spare -> active -> stable
  LVKW_QueueBuffer *old_stable = q->stable;
  q->stable = q->active;
  q->active = q->spare;
  q->spare = old_stable;

  q->active->count = 0;

  /* If there was growth in the interim, catch up the buffer that just rotated out of stable.
   * active and stable are already guaranteed to be large enough by _lvkw_event_queue_grow.
   */
  if (q->spare->capacity < q->stable->capacity) {
    _lvkw_buffer_free(q->ctx, q->spare);
    _lvkw_buffer_alloc(q->ctx, q->spare, q->stable->capacity);
  }
}

void lvkw_event_queue_scan(const LVKW_EventQueue *q, LVKW_EventType mask,
                           LVKW_EventCallback callback, void *userdata) {

  // TODO: Investigate the possibility of SIMD sheananigans here.

  uint32_t count = q->stable->count;
  for (uint32_t i = 0; i < count; ++i) {
    if (q->stable->types[i] & mask) {
      callback(q->stable->types[i], q->stable->windows[i], &q->stable->payloads[i], userdata);
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

void lvkw_event_queue_get_telemetry(LVKW_EventQueue *q, LVKW_EventTelemetry *out_telemetry,
                                    bool reset) {
  memset(out_telemetry, 0, sizeof(LVKW_EventTelemetry));
  out_telemetry->current_capacity = q->active->capacity;

#ifdef LVKW_GATHER_TELEMETRY
  out_telemetry->peak_count = q->peak_count;
  out_telemetry->drop_count = q->drop_count;

  if (reset) {
    q->peak_count = q->stable->count;
    q->drop_count = 0;
  }
#endif
}
