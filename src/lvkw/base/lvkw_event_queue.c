#include "lvkw_event_queue.h"

#include <string.h>

static bool _is_event_compressible(LVKW_EventType type) {
  return type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
         type == LVKW_EVENT_TYPE_WINDOW_RESIZED;
}

LVKW_Status lvkw_event_queue_init(LVKW_Context_Base *ctx, LVKW_EventQueue *q, uint32_t initial_capacity,
                                  uint32_t max_capacity) {
  memset(q, 0, sizeof(LVKW_EventQueue));
  q->max_capacity = max_capacity;

  if (initial_capacity > 0) {
    if (initial_capacity > max_capacity) initial_capacity = max_capacity;

    q->pool = (LVKW_Event *)lvkw_context_alloc(ctx, initial_capacity * sizeof(LVKW_Event));
    if (!q->pool) {
      return LVKW_ERROR_NOOP;
    }
    q->capacity = initial_capacity;
  }

  return LVKW_OK;
}

void lvkw_event_queue_cleanup(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  if (q->pool) {
    lvkw_context_free(ctx, q->pool);
  }
  memset(q, 0, sizeof(LVKW_EventQueue));
}

uint32_t lvkw_event_queue_get_count(const LVKW_EventQueue *q) { return q->count; }

static bool _lvkw_event_queue_grow(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  uint32_t new_capacity = q->capacity == 0 ? 64 : q->capacity * 2;
  if (new_capacity > q->max_capacity) new_capacity = q->max_capacity;

  if (new_capacity <= q->capacity) return false;

  LVKW_Event *new_pool = (LVKW_Event *)lvkw_context_alloc(ctx, new_capacity * sizeof(LVKW_Event));
  if (!new_pool) return false;

  uint32_t idx = 0;
  uint32_t i = q->head;
  uint32_t processed = 0;
  uint32_t old_count = q->count;

  while (processed < old_count || (q->capacity > 0 && i != q->tail)) {
    if (q->pool[i].type != 0) {
      new_pool[idx++] = q->pool[i];
    }
    i = (i + 1) % q->capacity;
    processed++;
    if (q->capacity == 0) break;
  }

  if (q->pool) lvkw_context_free(ctx, q->pool);
  q->pool = new_pool;
  q->capacity = new_capacity;
  q->head = 0;
  q->tail = idx;
  q->count = idx;  // Update count based on non-tombstoned events

  return true;
}

bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q, const LVKW_Event *evt) {
  // Tail compression
  if (q->count > 0) {
    uint32_t last_idx = (q->tail + q->capacity - 1) % q->capacity;
    LVKW_Event *last_ev = &q->pool[last_idx];

    if (evt->type == LVKW_EVENT_TYPE_MOUSE_SCROLL && last_ev->type == LVKW_EVENT_TYPE_MOUSE_SCROLL &&
        last_ev->common.window == evt->common.window) {
      last_ev->mouse_scroll.dx += evt->mouse_scroll.dx;
      last_ev->mouse_scroll.dy += evt->mouse_scroll.dy;
      return true;
    }
    if (evt->type == LVKW_EVENT_TYPE_MOUSE_MOTION && last_ev->type == LVKW_EVENT_TYPE_MOUSE_MOTION &&
        last_ev->common.window == evt->common.window) {
      last_ev->mouse_motion.dx += evt->mouse_motion.dx;
      last_ev->mouse_motion.dy += evt->mouse_motion.dy;
      if (evt->mouse_motion.x != -1.0) {
        last_ev->mouse_motion.x = evt->mouse_motion.x;
        last_ev->mouse_motion.y = evt->mouse_motion.y;
      }
      return true;
    }
    if (evt->type == LVKW_EVENT_TYPE_WINDOW_RESIZED && last_ev->type == LVKW_EVENT_TYPE_WINDOW_RESIZED &&
        last_ev->common.window == evt->common.window) {
      last_ev->resized.size = evt->resized.size;
      last_ev->resized.framebufferSize = evt->resized.framebufferSize;
      return true;
    }
  }

  // Full queue check
  if (q->count >= q->capacity) {
    if (q->capacity >= q->max_capacity) {
      if (_is_event_compressible(evt->type)) return false;

      // Evict oldest compressible
      uint32_t i = q->head;
      bool evicted = false;
      for (uint32_t j = 0; j < q->capacity; ++j) {
        uint32_t idx = (i + j) % q->capacity;
        if (_is_event_compressible(q->pool[idx].type)) {
          q->pool[idx].type = 0;
          q->count--;
          evicted = true;
          break;
        }
      }

      if (!evicted) return false;
    }
    else {
      if (!_lvkw_event_queue_grow(ctx, q)) {
        if (_is_event_compressible(evt->type)) return false;
        return false;
      }
    }
  }

  q->pool[q->tail] = *evt;
  q->tail = (q->tail + 1) % q->capacity;
  q->count++;
  return true;
}

bool lvkw_event_queue_pop(LVKW_EventQueue *q, LVKW_EventType mask, LVKW_Event *out_evt) {
  uint32_t processed = 0;
  uint32_t original_count = q->count;

  while (processed < original_count) {
    uint32_t i = q->head;
    LVKW_Event ev = q->pool[i];

    if (ev.type == 0) {
      // Tombstoned event, just skip it permanently
      q->head = (q->head + 1) % q->capacity;
      q->count--;
      original_count--;  // One less real event in the queue
      continue;
    }

    if (ev.type & mask) {
      // Found a match, pop it
      q->pool[i].type = 0;
      q->head = (q->head + 1) % q->capacity;
      q->count--;
      *out_evt = ev;
      return true;
    }
    else {
      // Not a match, rotate it to the tail
      q->head = (q->head + 1) % q->capacity;
      q->pool[q->tail] = ev;
      q->tail = (q->tail + 1) % q->capacity;
      processed++;
    }
  }
  return false;
}

void lvkw_event_queue_remove_window_events(LVKW_EventQueue *q, LVKW_Window *window) {
  if (!q->pool || q->count == 0) return;

  for (uint32_t i = 0; i < q->capacity; ++i) {
    LVKW_Event *ev = &q->pool[i];
    if (ev->type == 0) continue;

    // In LVKW_Event, 'window' is the first field in all union members
    // except 'type'. We can safely cast to a struct that starts with the window
    // pointer.
    struct {
      LVKW_EventType type;
      LVKW_Window *window;
    } *evt_header = (void *)ev;

    if (evt_header->window == window) {
      ev->type = (LVKW_EventType)0;
    }
  }
}