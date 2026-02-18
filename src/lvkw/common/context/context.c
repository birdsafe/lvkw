// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stddef.h>
#include <string.h>

#include "lvkw/details/lvkw_config.h"
#include "lvkw/c/context.h"
#include "lvkw/lvkw.h"
#include "api_constraints.h"
#include "internal.h"

LVKW_Status lvkw_context_create(const LVKW_ContextCreateInfo *create_info,
                               LVKW_Context **out_context) {
  LVKW_API_VALIDATE(createContext, create_info, out_context);

  LVKW_ContextCreateInfo create_info_copy;
  const LVKW_ContextTuning tuning_defaults = LVKW_CONTEXT_TUNING_DEFAULT;

  if (!create_info->tuning) {
    create_info_copy = *create_info;
    create_info_copy.tuning = &tuning_defaults;
    create_info = &create_info_copy;
  }

  return _lvkw_createContext_impl(create_info, out_context);
}

LVKW_Status lvkw_events_post(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                             const LVKW_Event *evt) {
  LVKW_API_VALIDATE(ctx_postEvent, ctx_handle, type, window, evt);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  
  uint32_t mask = atomic_load_explicit(&ctx_base->prv.event_mask, memory_order_relaxed);
  if (!(mask & (uint32_t)type)) {
    return LVKW_SUCCESS;
  }

  if (!_lvkw_notification_ring_push(&ctx_base->prv.external_notifications, type, window, evt)) {
    return LVKW_ERROR;
  }
  
  return LVKW_SUCCESS;
}

LVKW_Version lvkw_core_getVersion(void) {
  return (LVKW_Version){
      .major = LVKW_VERSION_MAJOR,
      .minor = LVKW_VERSION_MINOR,
      .patch = LVKW_VERSION_PATCH,
  };
}

LVKW_Status lvkw_wnd_getContext(LVKW_Window *window_handle, LVKW_Context **out_context) {
  LVKW_API_VALIDATE(wnd_getContext, window_handle, out_context);

  *out_context = (LVKW_Context *)((const LVKW_Window_Base *)window_handle)->prv.ctx_base;
  return LVKW_SUCCESS;
}

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void *_lvkw_default_realloc(void *ptr, size_t new_size, void *userdata) {
  (void)userdata;
  return realloc(ptr, new_size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

void _lvkw_update_state_from_event(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window, const LVKW_Event *evt) {
    (void)ctx;
    (void)type;
    (void)window;
    (void)evt;
    // TODO: Implement state updates from events (e.g. window flags)
}

void _lvkw_dispatch_event(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                          const LVKW_Event *evt) {
  uint32_t mask = atomic_load_explicit(&ctx->prv.event_mask, memory_order_relaxed);
  if (!(mask & (uint32_t)type)) {
    return;
  }

  _lvkw_update_state_from_event(ctx, type, window, evt);

  if (ctx->prv.event_callback) {
    ctx->prv.event_callback(type, window, evt, ctx->prv.event_userdata);
  }
}

bool _lvkw_notification_ring_push(LVKW_EventNotificationRing *ring, LVKW_EventType type,
                                  LVKW_Window *window, const LVKW_Event *evt) {
  uint32_t reserve_tail;
  uint32_t head;

  do {
    reserve_tail = atomic_load_explicit(&ring->reserve_tail, memory_order_relaxed);
    head = atomic_load_explicit(&ring->head, memory_order_acquire);
    if (reserve_tail - head >= ring->capacity) {
      return false;  // Queue full
    }
  } while (!atomic_compare_exchange_weak_explicit(
      &ring->reserve_tail, &reserve_tail, reserve_tail + 1, memory_order_acq_rel,
      memory_order_relaxed));

  LVKW_ExternalEvent *slot = &ring->buffer[reserve_tail % ring->capacity];
  slot->type = type;
  slot->window = window;
  if (evt)
    slot->payload = *evt;
  else
    memset(&slot->payload, 0, sizeof(slot->payload));

  while (atomic_load_explicit(&ring->tail, memory_order_acquire) != reserve_tail) {
    // Spin until our slot is ready to be committed
  }
  atomic_store_explicit(&ring->tail, reserve_tail + 1, memory_order_release);

  return true;
}

void _lvkw_notification_ring_dispatch_all(LVKW_Context_Base *ctx) {
  LVKW_EventNotificationRing *ring = &ctx->prv.external_notifications;
  uint32_t head = atomic_load_explicit(&ring->head, memory_order_relaxed);
  uint32_t tail = atomic_load_explicit(&ring->tail, memory_order_acquire);

  while (head != tail) {
    LVKW_ExternalEvent *slot = &ring->buffer[head % ring->capacity];
    _lvkw_dispatch_event(ctx, slot->type, slot->window, &slot->payload);
    head++;
  }
  atomic_store_explicit(&ring->head, head, memory_order_release);
}

LVKW_Status _lvkw_context_init_base(LVKW_Context_Base *ctx_base,
                                    const LVKW_ContextCreateInfo *create_info) {
  memset(ctx_base, 0, sizeof(*ctx_base));
  ctx_base->pub.userdata = create_info->userdata;
  ctx_base->prv.diagnostic_cb = create_info->attributes.diagnostic_cb;
  ctx_base->prv.diagnostic_userdata = create_info->attributes.diagnostic_userdata;

  if (create_info->allocator.alloc_cb) {
    ctx_base->prv.allocator = create_info->allocator;
  }
  else {
    ctx_base->prv.allocator.alloc_cb = _lvkw_default_alloc;
    ctx_base->prv.allocator.realloc_cb = _lvkw_default_realloc;
    ctx_base->prv.allocator.free_cb = _lvkw_default_free;
    ctx_base->prv.allocator.userdata = NULL;
  }

  const LVKW_ContextTuning defaults = LVKW_CONTEXT_TUNING_DEFAULT;
  const LVKW_ContextTuning *tuning = create_info->tuning ? create_info->tuning : &defaults;

  ctx_base->prv.vk_loader = tuning->vk_loader;
  
  uint32_t initial_event_mask = (uint32_t)create_info->attributes.event_mask;
  if (initial_event_mask == 0u) {
    initial_event_mask = (uint32_t)LVKW_EVENT_TYPE_ALL;
  }
  atomic_store_explicit(&ctx_base->prv.event_mask, initial_event_mask, memory_order_relaxed);
  ctx_base->prv.pump_event_mask = initial_event_mask;
  
  ctx_base->prv.event_callback = create_info->attributes.event_callback;
  ctx_base->prv.event_userdata = create_info->attributes.event_userdata;

  // Initialize notification ring
  // For now, fixed size 64 as per old defaults
  ctx_base->prv.external_notifications.capacity = 64; 
  ctx_base->prv.external_notifications.buffer = lvkw_context_alloc(ctx_base, sizeof(LVKW_ExternalEvent) * 64);
  if (!ctx_base->prv.external_notifications.buffer) {
      return LVKW_ERROR;
  }
  atomic_init(&ctx_base->prv.external_notifications.head, 0);
  atomic_init(&ctx_base->prv.external_notifications.tail, 0);
  atomic_init(&ctx_base->prv.external_notifications.reserve_tail, 0);

  _lvkw_string_cache_init(&ctx_base->prv.string_cache);
#if LVKW_API_VALIDATION > 0
  ctx_base->prv.creator_thread = _lvkw_get_current_thread_id();
#endif

  return LVKW_SUCCESS;
}

void _lvkw_context_cleanup_base(LVKW_Context_Base *ctx_base) {
  if (ctx_base->prv.external_notifications.buffer) {
      lvkw_context_free(ctx_base, ctx_base->prv.external_notifications.buffer);
  }

  _lvkw_string_cache_destroy(&ctx_base->prv.string_cache, ctx_base);

#ifdef LVKW_ENABLE_CONTROLLER
  LVKW_Controller_Base *c_curr = ctx_base->prv.controller_list;
  while (c_curr) {
    LVKW_Controller_Base *c_next = c_curr->prv.next;
    if (c_curr->prv.analogs_backing) {
      lvkw_context_free(ctx_base, c_curr->prv.analogs_backing);
    }
    if (c_curr->prv.buttons_backing) {
      lvkw_context_free(ctx_base, c_curr->prv.buttons_backing);
    }
    if (c_curr->prv.analog_channels_backing) {
      lvkw_context_free(ctx_base, c_curr->prv.analog_channels_backing);
    }
    if (c_curr->prv.button_channels_backing) {
      lvkw_context_free(ctx_base, c_curr->prv.button_channels_backing);
    }
    if (c_curr->prv.haptic_channels_backing) {
      lvkw_context_free(ctx_base, c_curr->prv.haptic_channels_backing);
    }
    lvkw_context_free(ctx_base, c_curr);
    c_curr = c_next;
  }
#endif

  // Clean up monitors
  LVKW_Monitor_Base *m_curr = ctx_base->prv.monitor_list;
  while (m_curr) {
    LVKW_Monitor_Base *m_next = m_curr->prv.next;
    lvkw_context_free(ctx_base, m_curr);
    m_curr = m_next;
  }
}

void _lvkw_update_base_attributes(LVKW_Context_Base *ctx_base, uint32_t field_mask,
                                  const LVKW_ContextAttributes *attributes) {
  if (field_mask & LVKW_CONTEXT_ATTR_DIAGNOSTICS) {
    ctx_base->prv.diagnostic_cb = attributes->diagnostic_cb;
    ctx_base->prv.diagnostic_userdata = attributes->diagnostic_userdata;
  }
  if (field_mask & LVKW_CONTEXT_ATTR_EVENT_MASK) {
    atomic_store_explicit(&ctx_base->prv.event_mask, (uint32_t)attributes->event_mask,
                          memory_order_relaxed);
  }
  if (field_mask & LVKW_CONTEXT_ATTR_EVENT_CALLBACK) {
    ctx_base->prv.event_callback = attributes->event_callback;
    ctx_base->prv.event_userdata = attributes->event_userdata;
  }
}

LVKW_Status lvkw_context_update(LVKW_Context *ctx_handle, uint32_t field_mask,
                               const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  _lvkw_update_base_attributes(ctx_base, field_mask, attributes);
  return LVKW_SUCCESS;
}

void _lvkw_context_mark_lost(LVKW_Context_Base *ctx_base) {
  if (!ctx_base || (ctx_base->pub.flags & LVKW_CONTEXT_STATE_LOST)) return;
  ctx_base->pub.flags |= LVKW_CONTEXT_STATE_LOST;

  LVKW_Window_Base *curr = ctx_base->prv.window_list;
  while (curr) {
    curr->pub.flags |= LVKW_WINDOW_STATE_LOST;
    curr = curr->prv.next;
  }
}

void _lvkw_window_list_add(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base) {
  window_base->prv.next = ctx_base->prv.window_list;
  ctx_base->prv.window_list = window_base;
}

void _lvkw_window_list_remove(LVKW_Context_Base *ctx_base, LVKW_Window_Base *window_base) {
  LVKW_Window_Base **curr = &ctx_base->prv.window_list;
  while (*curr) {
    if (*curr == window_base) {
      *curr = window_base->prv.next;
      break;
    }
    curr = &((*curr)->prv.next);
  }
}

#ifdef LVKW_ENABLE_DIAGNOSTICS
void _lvkw_report_bootstrap_diagnostic_internal(const LVKW_ContextCreateInfo *create_info,
                                                LVKW_Diagnostic diagnostic, const char *message) {
  if (create_info && create_info->attributes.diagnostic_cb) {
    LVKW_DiagnosticInfo info = {
        .diagnostic = diagnostic,
        .message = message,
        .context = NULL,
        .window = NULL,
    };
    create_info->attributes.diagnostic_cb(&info, create_info->attributes.diagnostic_userdata);
  }
}

void _lvkw_reportDiagnostic(LVKW_Context *ctx_handle, LVKW_Window *window_handle,
                            LVKW_Diagnostic diagnostic, const char *message) {
  if (!ctx_handle) return;
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  if (ctx_base->prv.diagnostic_cb) {
    LVKW_DiagnosticInfo info = {
        .diagnostic = diagnostic,
        .message = message,
        .context = ctx_handle,
        .window = window_handle,
    };
    ctx_base->prv.diagnostic_cb(&info, ctx_base->prv.diagnostic_userdata);
  }
}
#else
void _lvkw_reportDiagnostic(LVKW_Context *ctx_handle, LVKW_Window *window_handle,
                            LVKW_Diagnostic diagnostic, const char *message) {
  (void)ctx_handle;
  (void)window_handle;
  (void)diagnostic;
  (void)message;
}
#endif
