#include <stdlib.h>
#include <string.h>

#include "lvkw_api_constraints.h"
#include "lvkw_mock_internal.h"

LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx_handle, uint32_t field_mask,
                                 const LVKW_ContextAttributes *attributes);

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

LVKW_Status lvkw_ctx_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;

  LVKW_Allocator allocator = {.alloc_cb = _lvkw_default_alloc, .free_cb = _lvkw_default_free};

  if (create_info->allocator.alloc_cb) {
    allocator = create_info->allocator;
  }

  LVKW_Context_Mock *ctx =
      (LVKW_Context_Mock *)lvkw_alloc(&allocator, create_info->userdata, sizeof(LVKW_Context_Mock));
  if (!ctx) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_OUT_OF_MEMORY,
                                     "Failed to allocate storage for context");
    return LVKW_ERROR;
  }

  memset(ctx, 0, sizeof(*ctx));
  _lvkw_context_init_base(&ctx->base, create_info);
  ctx->base.prv.alloc_cb = allocator;

  LVKW_EventTuning tuning = create_info->tuning->events;
  LVKW_Status res = lvkw_event_queue_init(&ctx->base, &ctx->event_queue, tuning);
  if (res != LVKW_SUCCESS) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_OUT_OF_MEMORY, "Failed to allocate event queue pool");
    lvkw_context_free(&ctx->base, ctx);
    return LVKW_ERROR;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

  // Apply initial attributes
  lvkw_ctx_update_Mock((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_destroy_Mock(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  // Destroy all windows in list
  while (ctx->base.prv.window_list) {
    lvkw_wnd_destroy_Mock((LVKW_Window *)ctx->base.prv.window_list);
  }

  lvkw_event_queue_cleanup(&ctx->base, &ctx->event_queue);
  _lvkw_context_cleanup_base(&ctx->base);

  lvkw_context_free(&ctx->base, ctx);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getVkExtensions_Mock(LVKW_Context *ctx_handle, uint32_t *count,
                                          const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);

  (void)ctx_handle;
  static const char *extensions[] = {NULL};

  if (count) {
    *count = 0;
  }
  if (out_extensions) {
    *out_extensions = extensions;
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_pollEvents_Mock(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                     void *userdata) {
  LVKW_API_VALIDATE(ctx_pollEvents, ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_Mock(ctx_handle, 0, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents_Mock(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                     LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_waitEvents, ctx_handle, timeout_ms, event_mask, callback, userdata);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  LVKW_EventType type;
  LVKW_Window *window;
  LVKW_Event evt;

  while (lvkw_event_queue_pop(&ctx->event_queue, event_mask, &type, &window, &evt)) {
    if (type == LVKW_EVENT_TYPE_WINDOW_READY) {
      ((LVKW_Window_Base *)window)->pub.flags |= LVKW_WND_STATE_READY;
    }

    if (type == LVKW_EVENT_TYPE_DND_HOVER) {
      LVKW_Window_Base *wb = (LVKW_Window_Base *)window;
      if (evt.dnd_hover.entered) {
        wb->prv.session_userdata = NULL;
        wb->prv.current_action = LVKW_DND_ACTION_COPY;
      }
      static LVKW_DndFeedback feedback;
      feedback.session_userdata = &wb->prv.session_userdata;
      feedback.action = &wb->prv.current_action;
      evt.dnd_hover.feedback = &feedback;
    } else if (type == LVKW_EVENT_TYPE_DND_DROP) {
      LVKW_Window_Base *wb = (LVKW_Window_Base *)window;
      evt.dnd_drop.session_userdata = &wb->prv.session_userdata;
    } else if (type == LVKW_EVENT_TYPE_DND_LEAVE) {
      LVKW_Window_Base *wb = (LVKW_Window_Base *)window;
      evt.dnd_leave.session_userdata = &wb->prv.session_userdata;
    }

    callback(type, window, &evt, userdata);
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitors_Mock(LVKW_Context *ctx_handle, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_monitors, count);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  if (!out_monitors) {
    *count = ctx->monitor_count;
    return LVKW_SUCCESS;
  }

  uint32_t to_copy = ctx->monitor_count < *count ? ctx->monitor_count : *count;
  for (uint32_t i = 0; i < to_copy; i++) {
    out_monitors[i] = ctx->monitors[i];
  }
  *count = to_copy;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitorModes_Mock(LVKW_Context *ctx_handle, LVKW_MonitorId monitor, LVKW_VideoMode *out_modes,
                                          uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  /* Find the monitor by ID */
  for (uint32_t i = 0; i < ctx->monitor_count; i++) {
    if (ctx->monitors[i].id == monitor) {
      if (!out_modes) {
        *count = ctx->monitor_mode_counts[i];
        return LVKW_SUCCESS;
      }

      uint32_t to_copy = ctx->monitor_mode_counts[i] < *count ? ctx->monitor_mode_counts[i] : *count;
      for (uint32_t j = 0; j < to_copy; j++) {
        out_modes[j] = ctx->monitor_modes[i][j];
      }
      *count = to_copy;
      return LVKW_SUCCESS;
    }
  }

  /* Monitor not found */
  LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_INVALID_ARGUMENT, "Monitor ID not found");
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_update_Mock(LVKW_Context *ctx_handle, uint32_t field_mask,
                                 const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    ctx->idle_timeout_ms = attributes->idle_timeout_ms;
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    ctx->inhibit_idle = attributes->inhibit_idle;
  }

  if (field_mask & LVKW_CTX_ATTR_DIAGNOSTICS) {
    ctx->base.prv.diagnostic_cb = attributes->diagnostic_cb;
    ctx->base.prv.diagnostic_userdata = attributes->diagnostic_userdata;
  }

  return LVKW_SUCCESS;
}
