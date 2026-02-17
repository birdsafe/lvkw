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

#include "event_queue.h"

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

LVKW_Status _lvkw_context_init_base(LVKW_Context_Base *ctx_base,
                                    const LVKW_ContextCreateInfo *create_info) {
  const LVKW_ContextTuning defaults = LVKW_CONTEXT_TUNING_DEFAULT;
  const LVKW_ContextTuning *tuning = create_info->tuning ? create_info->tuning : &defaults;

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

  ctx_base->prv.vk_loader = tuning->vk_loader;
  ctx_base->prv.event_mask = create_info->attributes.event_mask;
  if (ctx_base->prv.event_mask == 0) {
    ctx_base->prv.event_mask = LVKW_EVENT_TYPE_ALL;
  }
  _lvkw_string_cache_init(&ctx_base->prv.string_cache);
#if LVKW_API_VALIDATION > 0
  ctx_base->prv.creator_thread = _lvkw_get_current_thread_id();
#endif

  return lvkw_event_queue_init(ctx_base, &ctx_base->prv.event_queue, tuning->events);
}

void _lvkw_context_cleanup_base(LVKW_Context_Base *ctx_base) {
  lvkw_event_queue_cleanup(ctx_base, &ctx_base->prv.event_queue);
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
    // N.B. Backends that have additional monitor-specific data should have
    // cleaned it up in their own destroyContext implementation before calling this.
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
    ctx_base->prv.event_mask = attributes->event_mask;
  }
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
