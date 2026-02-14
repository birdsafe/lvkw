// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_event_queue.h"

#include <string.h>

static bool _is_event_compressible(LVKW_EventType type) {
  return type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
         type == LVKW_EVENT_TYPE_WINDOW_RESIZED;
}

LVKW_Status lvkw_event_queue_init(LVKW_Context_Base *ctx, LVKW_EventQueue *q,
                                  LVKW_EventTuning tuning) {
  memset(q, 0, sizeof(LVKW_EventQueue));
  q->max_capacity = tuning.max_capacity;
  q->growth_factor = tuning.growth_factor;

  if (tuning.initial_capacity > 0) {
    if (tuning.initial_capacity > tuning.max_capacity)
      tuning.initial_capacity = tuning.max_capacity;

    q->types = (LVKW_EventType *)lvkw_context_alloc_aligned(
        ctx, tuning.initial_capacity * sizeof(LVKW_EventType));
    q->windows = (LVKW_Window **)lvkw_context_alloc_aligned(
        ctx, tuning.initial_capacity * sizeof(LVKW_Window *));
    q->payloads = (LVKW_EventPayload *)lvkw_context_alloc_aligned(
        ctx, tuning.initial_capacity * sizeof(LVKW_EventPayload));

    if (!q->types || !q->windows || !q->payloads) {
      lvkw_event_queue_cleanup(ctx, q);
      return LVKW_ERROR;
    }
    q->capacity = tuning.initial_capacity;
  }

  return LVKW_SUCCESS;
}

void lvkw_event_queue_cleanup(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  if (q->types) lvkw_context_free_aligned(ctx, q->types);
  if (q->windows) lvkw_context_free_aligned(ctx, q->windows);
  if (q->payloads) lvkw_context_free_aligned(ctx, q->payloads);
  memset(q, 0, sizeof(LVKW_EventQueue));
}

uint32_t lvkw_event_queue_get_count(const LVKW_EventQueue *q) { return q->count; }

static bool _lvkw_event_queue_grow(LVKW_Context_Base *ctx, LVKW_EventQueue *q) {
  uint32_t new_capacity =
      q->capacity == 0 ? 64 : (uint32_t)((double)q->capacity * q->growth_factor);
  if (new_capacity <= q->capacity) new_capacity = q->capacity + 1;
  if (new_capacity > q->max_capacity) new_capacity = q->max_capacity;

  if (new_capacity <= q->capacity) return false;

  LVKW_EventType *new_types =
      (LVKW_EventType *)lvkw_context_alloc_aligned(ctx, new_capacity * sizeof(LVKW_EventType));
  LVKW_Window **new_windows =
      (LVKW_Window **)lvkw_context_alloc_aligned(ctx, new_capacity * sizeof(LVKW_Window *));
  LVKW_EventPayload *new_payloads = (LVKW_EventPayload *)lvkw_context_alloc_aligned(
      ctx, new_capacity * sizeof(LVKW_EventPayload));

  if (!new_types || !new_windows || !new_payloads) {
    if (new_types) lvkw_context_free_aligned(ctx, new_types);
    if (new_windows) lvkw_context_free_aligned(ctx, new_windows);
    if (new_payloads) lvkw_context_free_aligned(ctx, new_payloads);
    return false;
  }

  uint32_t idx = 0;
  uint32_t i = q->head;
  uint32_t processed = 0;
  uint32_t old_count = q->count;

  while (processed < old_count || (q->capacity > 0 && i != q->tail)) {
    if (q->types[i] != 0) {
      new_types[idx] = q->types[i];
      new_windows[idx] = q->windows[i];
      new_payloads[idx] = q->payloads[i];
      idx++;
    }
    i = (i + 1) % q->capacity;
    processed++;
    if (q->capacity == 0) break;
  }

  if (q->types) lvkw_context_free_aligned(ctx, q->types);
  if (q->windows) lvkw_context_free_aligned(ctx, q->windows);
  if (q->payloads) lvkw_context_free_aligned(ctx, q->payloads);

  q->types = new_types;
  q->windows = new_windows;
  q->payloads = new_payloads;
  q->capacity = new_capacity;
  q->head = 0;
  q->tail = idx;
  q->count = idx;

  return true;
}

bool lvkw_event_queue_push(LVKW_Context_Base *ctx, LVKW_EventQueue *q, LVKW_EventType type,
                           LVKW_Window *window, const LVKW_Event *evt) {
  // Tail compression
  if (q->count > 0) {
    uint32_t last_idx = (q->tail + q->capacity - 1) % q->capacity;
    LVKW_EventType last_type = q->types[last_idx];
    LVKW_Window *last_window = q->windows[last_idx];

    if (type == LVKW_EVENT_TYPE_MOUSE_SCROLL && last_type == LVKW_EVENT_TYPE_MOUSE_SCROLL &&
        last_window == window) {
      q->payloads[last_idx].mouse_scroll.delta.x += evt->mouse_scroll.delta.x;
      q->payloads[last_idx].mouse_scroll.delta.y += evt->mouse_scroll.delta.y;
      return true;
    }
    if (type == LVKW_EVENT_TYPE_MOUSE_MOTION && last_type == LVKW_EVENT_TYPE_MOUSE_MOTION &&
        last_window == window) {
      q->payloads[last_idx].mouse_motion.delta.x += evt->mouse_motion.delta.x;
      q->payloads[last_idx].mouse_motion.delta.y += evt->mouse_motion.delta.y;
      q->payloads[last_idx].mouse_motion.raw_delta.x += evt->mouse_motion.raw_delta.x;
      q->payloads[last_idx].mouse_motion.raw_delta.y += evt->mouse_motion.raw_delta.y;

      // Update absolute position to latest
      q->payloads[last_idx].mouse_motion.position = evt->mouse_motion.position;
      return true;
    }
    if (type == LVKW_EVENT_TYPE_WINDOW_RESIZED && last_type == LVKW_EVENT_TYPE_WINDOW_RESIZED &&
        last_window == window) {
      q->payloads[last_idx].resized.geometry = evt->resized.geometry;
      return true;
    }
  }

  // Full queue check
  if (q->count >= q->capacity) {
    if (q->capacity >= q->max_capacity) {
      if (_is_event_compressible(type)) {
#ifdef LVKW_GATHER_TELEMETRY
        q->drop_count++;
#endif
        return false;
      }

      // Find oldest compressible
      int32_t evict_idx = -1;
      for (uint32_t j = 0; j < q->capacity; ++j) {
        uint32_t idx = (q->head + j) % q->capacity;
        if (_is_event_compressible(q->types[idx])) {
          evict_idx = (int32_t)idx;
          break;
        }
      }

      if (evict_idx == -1) {
#ifdef LVKW_GATHER_TELEMETRY
        q->drop_count++;
#endif
        return false;
      }

      // To evict at evict_idx and make space at tail (which is head),
      // we shift everything from head to evict_idx-1 forward by one.
      uint32_t curr = (uint32_t)evict_idx;
      while (curr != q->head) {
        uint32_t prev = (curr + q->capacity - 1) % q->capacity;
        q->types[curr] = q->types[prev];
        q->windows[curr] = q->windows[prev];
        q->payloads[curr] = q->payloads[prev];
        curr = prev;
      }

      q->head = (q->head + 1) % q->capacity;
      q->count--;
      // Now q->tail (which was the old head) is effectively free as it's no longer in the [head,
      // count] range
    }
    else {
      if (!_lvkw_event_queue_grow(ctx, q)) {
#ifdef LVKW_GATHER_TELEMETRY
        q->drop_count++;
#endif
        return false;
      }
    }
  }

  q->types[q->tail] = type;
  q->windows[q->tail] = window;

  // Manual copy of payload based on type
  switch (type) {
    case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
      q->payloads[q->tail].close_requested = evt->close_requested;
      break;
    case LVKW_EVENT_TYPE_WINDOW_RESIZED:
      q->payloads[q->tail].resized = evt->resized;
      break;
    case LVKW_EVENT_TYPE_KEY:
      q->payloads[q->tail].key = evt->key;
      break;
    case LVKW_EVENT_TYPE_WINDOW_READY:
      q->payloads[q->tail].window_ready = evt->window_ready;
      break;
    case LVKW_EVENT_TYPE_MOUSE_MOTION:
      q->payloads[q->tail].mouse_motion = evt->mouse_motion;
      break;
    case LVKW_EVENT_TYPE_MOUSE_BUTTON:
      q->payloads[q->tail].mouse_button = evt->mouse_button;
      break;
    case LVKW_EVENT_TYPE_MOUSE_SCROLL:
      q->payloads[q->tail].mouse_scroll = evt->mouse_scroll;
      break;
    case LVKW_EVENT_TYPE_IDLE_NOTIFICATION:
      q->payloads[q->tail].idle = evt->idle;
      break;
    case LVKW_EVENT_TYPE_MONITOR_CONNECTION:
      q->payloads[q->tail].monitor_connection = evt->monitor_connection;
      break;
    case LVKW_EVENT_TYPE_MONITOR_MODE:
      q->payloads[q->tail].monitor_mode = evt->monitor_mode;
      break;
    case LVKW_EVENT_TYPE_TEXT_INPUT:
      q->payloads[q->tail].text_input = evt->text_input;
      break;
    case LVKW_EVENT_TYPE_FOCUS:
      q->payloads[q->tail].focus = evt->focus;
      break;
    case LVKW_EVENT_TYPE_WINDOW_MAXIMIZED:
      q->payloads[q->tail].maximized = evt->maximized;
      break;
    case LVKW_EVENT_TYPE_DND_HOVER:
      q->payloads[q->tail].dnd_hover = evt->dnd_hover;
      break;
    case LVKW_EVENT_TYPE_DND_LEAVE:
      q->payloads[q->tail].dnd_leave = evt->dnd_leave;
      break;
    case LVKW_EVENT_TYPE_DND_DROP:
      q->payloads[q->tail].dnd_drop = evt->dnd_drop;
      break;
    case LVKW_EVENT_TYPE_TEXT_COMPOSITION:
      q->payloads[q->tail].text_composition = evt->text_composition;
      break;
#ifdef LVKW_ENABLE_CONTROLLER
    case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION:
      q->payloads[q->tail].controller_connection = evt->controller_connection;
      break;
#endif
    default:
      // Fallback for unknown types (safest path is bitwise copy if possible, but union is complex)
      memset(&q->payloads[q->tail], 0, sizeof(LVKW_EventPayload));
      break;
  }

  q->tail = (q->tail + 1) % q->capacity;
  q->count++;

#ifdef LVKW_GATHER_TELEMETRY
  if (q->count > q->peak_count) q->peak_count = q->count;
#endif

  return true;
}

bool lvkw_event_queue_pop(LVKW_EventQueue *q, LVKW_EventType mask, LVKW_EventType *out_type,
                          LVKW_Window **out_window, LVKW_Event *out_evt) {
  while (q->count > 0) {
    LVKW_EventType type = q->types[q->head];

    if (type == 0) {
      q->head = (q->head + 1) % q->capacity;
      continue;
    }

    if (type & mask) {
      *out_type = type;
      *out_window = q->windows[q->head];

      switch (type) {
        case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
          out_evt->close_requested = q->payloads[q->head].close_requested;
          break;
        case LVKW_EVENT_TYPE_WINDOW_RESIZED:
          out_evt->resized = q->payloads[q->head].resized;
          break;
        case LVKW_EVENT_TYPE_KEY:
          out_evt->key = q->payloads[q->head].key;
          break;
        case LVKW_EVENT_TYPE_WINDOW_READY:
          out_evt->window_ready = q->payloads[q->head].window_ready;
          break;
        case LVKW_EVENT_TYPE_MOUSE_MOTION:
          out_evt->mouse_motion = q->payloads[q->head].mouse_motion;
          break;
        case LVKW_EVENT_TYPE_MOUSE_BUTTON:
          out_evt->mouse_button = q->payloads[q->head].mouse_button;
          break;
        case LVKW_EVENT_TYPE_MOUSE_SCROLL:
          out_evt->mouse_scroll = q->payloads[q->head].mouse_scroll;
          break;
        case LVKW_EVENT_TYPE_IDLE_NOTIFICATION:
          out_evt->idle = q->payloads[q->head].idle;
          break;
        case LVKW_EVENT_TYPE_MONITOR_CONNECTION:
          out_evt->monitor_connection = q->payloads[q->head].monitor_connection;
          break;
        case LVKW_EVENT_TYPE_MONITOR_MODE:
          out_evt->monitor_mode = q->payloads[q->head].monitor_mode;
          break;
        case LVKW_EVENT_TYPE_TEXT_INPUT:
          out_evt->text_input = q->payloads[q->head].text_input;
          break;
        case LVKW_EVENT_TYPE_FOCUS:
          out_evt->focus = q->payloads[q->head].focus;
          break;
        case LVKW_EVENT_TYPE_WINDOW_MAXIMIZED:
          out_evt->maximized = q->payloads[q->head].maximized;
          break;
        case LVKW_EVENT_TYPE_DND_HOVER:
          out_evt->dnd_hover = q->payloads[q->head].dnd_hover;
          break;
        case LVKW_EVENT_TYPE_DND_LEAVE:
          out_evt->dnd_leave = q->payloads[q->head].dnd_leave;
          break;
        case LVKW_EVENT_TYPE_DND_DROP:
          out_evt->dnd_drop = q->payloads[q->head].dnd_drop;
          break;
        case LVKW_EVENT_TYPE_TEXT_COMPOSITION:
          out_evt->text_composition = q->payloads[q->head].text_composition;
          break;
#ifdef LVKW_ENABLE_CONTROLLER
        case LVKW_EVENT_TYPE_CONTROLLER_CONNECTION:
          out_evt->controller_connection = q->payloads[q->head].controller_connection;
          break;
#endif
        default:
          break;
      }

      q->types[q->head] = 0;
      q->count--;
      q->head = (q->head + 1) % q->capacity;

      // Advance head over tombstones
      while (q->count > 0 && q->types[q->head] == 0) {
        q->head = (q->head + 1) % q->capacity;
      }

      return true;
    }

    // Flush mismatched event
    q->types[q->head] = 0;
    q->count--;
    q->head = (q->head + 1) % q->capacity;
  }
  return false;
}

bool lvkw_event_queue_peek(const LVKW_EventQueue *q, LVKW_EventType mask) {
  if (q->count == 0) return false;

  for (uint32_t j = 0; j < q->capacity; ++j) {
    uint32_t idx = (q->head + j) % q->capacity;
    if (q->types[idx] == 0) continue;
    if (q->types[idx] & mask) return true;
  }
  return false;
}

void lvkw_event_queue_remove_window_events(LVKW_EventQueue *q, LVKW_Window *window) {
  if (!q->types || q->count == 0) return;

  for (uint32_t i = 0; i < q->capacity; ++i) {
    if (q->types[i] == 0) continue;

    if (q->windows[i] == window) {
      q->types[i] = 0;
      q->count--;
    }
  }
}

void lvkw_event_queue_get_telemetry(LVKW_EventQueue *q, LVKW_EventTelemetry *out_telemetry,
                                    bool reset) {
  memset(out_telemetry, 0, sizeof(LVKW_EventTelemetry));
  out_telemetry->current_capacity = q->capacity;

#ifdef LVKW_GATHER_TELEMETRY
  out_telemetry->peak_count = q->peak_count;
  out_telemetry->drop_count = q->drop_count;

  if (reset) {
    q->peak_count = q->count;
    q->drop_count = 0;
  }
#endif
}