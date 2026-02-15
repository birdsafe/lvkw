// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lvkw/details/lvkw_config.h"
#include "lvkw_api_constraints.h"
#include "lvkw_mem_internal.h"
#include "lvkw_wayland_internal.h"

#ifdef LVKW_ENABLE_INTERNAL_CHECKS
void _lvkw_wayland_symbol_poison_trap(void) {
  fprintf(stderr, "[LVKW FATAL] Poisoned Wayland symbol called.\n");
  fprintf(stderr, "This indicates a bug in LVKW: a global Wayland helper was used instead of a "
                  "context-vtable-aware one.\n");
  abort();
}
#endif

/* xdg_activation_v1 */

static void _xdg_activation_token_handle_done(void *data, struct xdg_activation_token_v1 *token,
                                              const char *token_str) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  lvkw_xdg_activation_v1_activate(ctx, ctx->protocols.opt.xdg_activation_v1, token_str, window->wl.surface);
  lvkw_xdg_activation_token_v1_destroy(ctx, token);
}

static const struct xdg_activation_token_v1_listener _xdg_activation_token_listener = {
    .done = _xdg_activation_token_handle_done,
};

LVKW_Status lvkw_wnd_requestFocus_WL(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (!ctx->protocols.opt.xdg_activation_v1) {
    // No portable compositor-side fallback exists. Treat as a no-op.
    return LVKW_SUCCESS;
  }

  struct xdg_activation_token_v1 *token =
      lvkw_xdg_activation_v1_get_activation_token(ctx, ctx->protocols.opt.xdg_activation_v1);
  lvkw_xdg_activation_token_v1_add_listener(ctx, token, &_xdg_activation_token_listener, window);

  if (ctx->protocols.wl_seat) {
    lvkw_xdg_activation_token_v1_set_serial(ctx, token, ctx->input.pointer_serial, ctx->protocols.wl_seat);
  }
  lvkw_xdg_activation_token_v1_set_surface(ctx, token, window->wl.surface);
  lvkw_xdg_activation_token_v1_commit(ctx, token);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

/* ext_idle_notification_v1 */

static void _idle_handle_idled(void *data, struct ext_idle_notification_v1 *notification) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {0};
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = true;
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_IDLE_NOTIFICATION, NULL,
                        &ev);
}

static void _idle_handle_resumed(void *data, struct ext_idle_notification_v1 *notification) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {0};
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = false;
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_IDLE_NOTIFICATION, NULL,
                        &ev);
}

const struct ext_idle_notification_v1_listener _lvkw_wayland_idle_listener = {
    .idled = _idle_handle_idled,
    .resumed = _idle_handle_resumed,
};

LVKW_Status lvkw_ctx_update_WL(LVKW_Context *ctx_handle, uint32_t field_mask,
                               const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  if (field_mask & LVKW_CTX_ATTR_IDLE_TIMEOUT) {
    uint32_t timeout_ms = attributes->idle_timeout_ms;

    // Clear existing tracker
    if (ctx->idle.notification) {
      lvkw_ext_idle_notification_v1_destroy(ctx, ctx->idle.notification);
      ctx->idle.notification = NULL;
      ctx->idle.timeout_ms = 0;
    }

    if (timeout_ms != LVKW_NEVER) {
      if (!ctx->protocols.opt.ext_idle_notifier_v1) {
        LVKW_REPORT_CTX_DIAGNOSTIC(ctx_handle, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                   "ext_idle_notifier_v1 not available");
        return LVKW_ERROR;
      }

      if (!ctx->protocols.wl_seat) {
        LVKW_REPORT_CTX_DIAGNOSTIC(ctx_handle, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                   "wl_seat not available");
        return LVKW_ERROR;
      }

      ctx->idle.timeout_ms = timeout_ms;
      ctx->idle.notification = lvkw_ext_idle_notifier_v1_get_idle_notification(ctx, 
          ctx->protocols.opt.ext_idle_notifier_v1, timeout_ms, ctx->protocols.wl_seat);
      lvkw_ext_idle_notification_v1_add_listener(ctx, ctx->idle.notification, &_lvkw_wayland_idle_listener,
                                            ctx);
    }
  }

  if (field_mask & LVKW_CTX_ATTR_INHIBIT_IDLE) {
    if (ctx->inhibit_idle != attributes->inhibit_idle) {
      ctx->inhibit_idle = attributes->inhibit_idle;

      if (ctx->inhibit_idle && !ctx->protocols.opt.zwp_idle_inhibit_manager_v1) {
        LVKW_REPORT_CTX_DIAGNOSTIC(ctx_handle, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                   "zwp_idle_inhibit_manager_v1 not available");
      }

      // Update all existing windows
      LVKW_Window_Base *curr = ctx->base.prv.window_list;
      while (curr) {
        LVKW_Window_WL *window = (LVKW_Window_WL *)curr;
        if (ctx->inhibit_idle) {
          if (!window->ext.idle_inhibitor && ctx->protocols.opt.zwp_idle_inhibit_manager_v1) {
            window->ext.idle_inhibitor = lvkw_zwp_idle_inhibit_manager_v1_create_inhibitor(ctx, 
                ctx->protocols.opt.zwp_idle_inhibit_manager_v1, window->wl.surface);
          }
        }
        else {
          if (window->ext.idle_inhibitor) {
            lvkw_zwp_idle_inhibitor_v1_destroy(ctx, window->ext.idle_inhibitor);
            window->ext.idle_inhibitor = NULL;
          }
        }
        curr = curr->prv.next;
      }
    }
  }

  _lvkw_update_base_attributes(&ctx->base, field_mask, attributes);

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setClipboardText_WL(LVKW_Window *window, const char *text) {
  LVKW_API_VALIDATE(wnd_setClipboardText, window, text);

  const size_t text_size = strlen(text);
  const LVKW_ClipboardData items[2] = {
      {.mime_type = "text/plain;charset=utf-8", .data = text, .size = text_size},
      {.mime_type = "text/plain", .data = text, .size = text_size},
  };

  return lvkw_wnd_setClipboardData_WL(window, items, 2);
}

static void _clipboard_invalidate_mime_query(LVKW_Context_WL *ctx) {
  if (ctx->input.clipboard.mime_query_ptr) {
    lvkw_context_free(&ctx->base, (void *)ctx->input.clipboard.mime_query_ptr);
    ctx->input.clipboard.mime_query_ptr = NULL;
  }
  ctx->input.clipboard.mime_query_count = 0;
}

static void _clipboard_clear_owned_data(LVKW_Context_WL *ctx) {
  for (uint32_t i = 0; i < ctx->input.clipboard.owned_mime_count; ++i) {
    lvkw_context_free(&ctx->base, ctx->input.clipboard.owned_mimes[i].bytes);
  }
  if (ctx->input.clipboard.owned_mimes) {
    lvkw_context_free(&ctx->base, ctx->input.clipboard.owned_mimes);
  }

  ctx->input.clipboard.owned_mimes = NULL;
  ctx->input.clipboard.owned_mime_count = 0;
}

static void _clipboard_invalidate_read_cache(LVKW_Context_WL *ctx) {
  ctx->input.clipboard.read_cache_size = 0;
}

static bool _clipboard_ensure_read_cache_capacity(LVKW_Context_WL *ctx, size_t capacity) {
  if (capacity <= ctx->input.clipboard.read_cache_capacity) return true;

  uint8_t *next = lvkw_context_realloc(&ctx->base, ctx->input.clipboard.read_cache,
                                       ctx->input.clipboard.read_cache_capacity, capacity);
  if (!next) return false;

  ctx->input.clipboard.read_cache = next;
  ctx->input.clipboard.read_cache_capacity = capacity;
  return true;
}

static bool _clipboard_copy_into_read_cache(LVKW_Context_WL *ctx, const void *data, size_t size,
                                            bool add_nul) {
  size_t need_size = size + (add_nul ? 1 : 0);
  if (!_clipboard_ensure_read_cache_capacity(ctx, need_size)) return false;

  if (size > 0) memcpy(ctx->input.clipboard.read_cache, data, size);
  if (add_nul) ctx->input.clipboard.read_cache[size] = '\0';

  ctx->input.clipboard.read_cache_size = size;
  return true;
}

static bool _clipboard_ensure_read_cache_nul(LVKW_Context_WL *ctx) {
  const size_t text_size = ctx->input.clipboard.read_cache_size;
  if (!_clipboard_ensure_read_cache_capacity(ctx, text_size + 1)) return false;
  ctx->input.clipboard.read_cache[text_size] = '\0';
  return true;
}

static LVKW_WaylandClipboardMime *_clipboard_find_owned_mime(LVKW_Context_WL *ctx,
                                                             const char *mime_type) {
  for (uint32_t i = 0; i < ctx->input.clipboard.owned_mime_count; ++i) {
    if (strcmp(ctx->input.clipboard.owned_mimes[i].mime_type, mime_type) == 0) {
      return &ctx->input.clipboard.owned_mimes[i];
    }
  }
  return NULL;
}

static bool _clipboard_read_offer_bytes(LVKW_Context_WL *ctx, struct wl_data_offer *offer,
                                        const char *mime_type, uint8_t **out_bytes,
                                        size_t *out_size) {
  int pipefd[2];
  if (pipe(pipefd) != 0) return false;

  lvkw_wl_data_offer_receive(ctx, offer, mime_type, pipefd[1]);
  close(pipefd[1]);
  lvkw_wl_display_flush(ctx, ctx->wl.display);
  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) {
    close(pipefd[0]);
    return false;
  }

  uint8_t *buffer = NULL;
  size_t size = 0;
  size_t capacity = 0;
  uint8_t tmp[2048];
  for (;;) {
    const ssize_t read_size = read(pipefd[0], tmp, sizeof(tmp));
    if (read_size <= 0) break;

    const size_t add_size = (size_t)read_size;
    if ((size + add_size) > capacity) {
      size_t next_capacity = capacity == 0 ? 2048 : capacity * 2;
      while (next_capacity < (size + add_size)) next_capacity *= 2;

      uint8_t *next = lvkw_context_realloc(&ctx->base, buffer, capacity, next_capacity);
      if (!next) {
        close(pipefd[0]);
        if (buffer) lvkw_context_free(&ctx->base, buffer);
        return false;
      }

      buffer = next;
      capacity = next_capacity;
    }

    memcpy(buffer + size, tmp, add_size);
    size += add_size;
  }

  close(pipefd[0]);

  *out_bytes = buffer;
  *out_size = size;
  return true;
}

static LVKW_Status _clipboard_try_get_data(LVKW_Context_WL *ctx, const char *mime_type,
                                           bool report_missing) {
  _clipboard_invalidate_read_cache(ctx);

  LVKW_WaylandClipboardMime *owned = _clipboard_find_owned_mime(ctx, mime_type);
  if (owned) {
    if (!_clipboard_copy_into_read_cache(ctx, owned->bytes, owned->size, false)) return LVKW_ERROR;
    return LVKW_SUCCESS;
  }

  if (!ctx->input.clipboard.selection_offer) {
    if (report_missing) {
      LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                 "No clipboard selection is available");
    }
    return LVKW_ERROR;
  }

  if (!_lvkw_wayland_offer_meta_has_mime(ctx, ctx->input.clipboard.selection_offer, mime_type)) {
    if (report_missing) {
      LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                                 "Requested MIME type not present in clipboard");
    }
    return LVKW_ERROR;
  }

  uint8_t *offer_data = NULL;
  size_t offer_size = 0;
  if (!_clipboard_read_offer_bytes(ctx, ctx->input.clipboard.selection_offer, mime_type, &offer_data,
                                   &offer_size)) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Clipboard transfer failed");
    return LVKW_ERROR;
  }

  const bool copied = _clipboard_copy_into_read_cache(ctx, offer_data, offer_size, false);
  if (offer_data) lvkw_context_free(&ctx->base, offer_data);
  return copied ? LVKW_SUCCESS : LVKW_ERROR;
}

static void _clipboard_data_source_send(void *data, struct wl_data_source *source,
                                        const char *mime_type, int32_t fd) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (!ctx || !mime_type) {
    close(fd);
    return;
  }

  LVKW_WaylandClipboardMime *mime = NULL;
  for (uint32_t i = 0; i < ctx->input.clipboard.owned_mime_count; ++i) {
    if (strcmp(ctx->input.clipboard.owned_mimes[i].mime_type, mime_type) == 0) {
      mime = &ctx->input.clipboard.owned_mimes[i];
      break;
    }
  }

  if (!mime) {
    close(fd);
    return;
  }

  const uint8_t *bytes = (const uint8_t *)mime->bytes;
  size_t remaining = mime->size;
  while (remaining > 0) {
    const ssize_t written = write(fd, bytes, remaining);
    if (written <= 0) break;
    bytes += (size_t)written;
    remaining -= (size_t)written;
  }

  close(fd);
  (void)source;
}

static void _clipboard_data_source_cancelled(void *data, struct wl_data_source *source) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (!ctx) return;
  if (ctx->input.clipboard.owned_source != source) return;

  lvkw_wl_data_source_destroy(ctx, source);
  ctx->input.clipboard.owned_source = NULL;
  _clipboard_clear_owned_data(ctx);
}

static void _clipboard_data_source_target(void *data, struct wl_data_source *source,
                                          const char *mime_type) {
  (void)data;
  (void)source;
  (void)mime_type;
}

static void _clipboard_data_source_dnd_drop_performed(void *data, struct wl_data_source *source) {
  (void)data;
  (void)source;
}

static void _clipboard_data_source_dnd_finished(void *data, struct wl_data_source *source) {
  (void)data;
  (void)source;
}

static void _clipboard_data_source_action(void *data, struct wl_data_source *source,
                                          uint32_t dnd_action) {
  (void)data;
  (void)source;
  (void)dnd_action;
}

static const struct wl_data_source_listener _clipboard_data_source_listener = {
    .target = _clipboard_data_source_target,
    .send = _clipboard_data_source_send,
    .cancelled = _clipboard_data_source_cancelled,
    .dnd_drop_performed = _clipboard_data_source_dnd_drop_performed,
    .dnd_finished = _clipboard_data_source_dnd_finished,
    .action = _clipboard_data_source_action,
};

LVKW_Status lvkw_wnd_setClipboardData_WL(LVKW_Window *window, const LVKW_ClipboardData *data,
                                         uint32_t count) {
  LVKW_API_VALIDATE(wnd_setClipboardData, window, data, count);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;

  if (!ctx->protocols.opt.wl_data_device_manager || !ctx->input.data_device) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                               "Clipboard requires wl_data_device_manager");
    return LVKW_ERROR;
  }

  if (ctx->input.clipboard_serial == 0) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                               "No valid input serial available for clipboard ownership");
    return LVKW_ERROR;
  }

  _clipboard_invalidate_mime_query(ctx);

  if (count == 0) {
    lvkw_wl_data_device_set_selection(ctx, ctx->input.data_device, NULL, ctx->input.clipboard_serial);
    lvkw_wl_display_flush(ctx, ctx->wl.display);
    _lvkw_wayland_check_error(ctx);
    if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

    if (ctx->input.clipboard.owned_source) {
      lvkw_wl_data_source_destroy(ctx, ctx->input.clipboard.owned_source);
      ctx->input.clipboard.owned_source = NULL;
    }
    _clipboard_clear_owned_data(ctx);
    return LVKW_SUCCESS;
  }

  LVKW_WaylandClipboardMime *owned_copy =
      lvkw_context_alloc(&ctx->base, sizeof(LVKW_WaylandClipboardMime) * count);
  if (!owned_copy) return LVKW_ERROR;
  memset(owned_copy, 0, sizeof(LVKW_WaylandClipboardMime) * count);

  for (uint32_t i = 0; i < count; ++i) {
    owned_copy[i].mime_type = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, data[i].mime_type);
    if (!owned_copy[i].mime_type) {
      for (uint32_t j = 0; j < i; ++j) lvkw_context_free(&ctx->base, owned_copy[j].bytes);
      lvkw_context_free(&ctx->base, owned_copy);
      return LVKW_ERROR;
    }
    owned_copy[i].size = data[i].size;
    if (data[i].size > 0) {
      owned_copy[i].bytes = lvkw_context_alloc(&ctx->base, data[i].size);
      if (!owned_copy[i].bytes) {
        for (uint32_t j = 0; j < i; ++j) lvkw_context_free(&ctx->base, owned_copy[j].bytes);
        lvkw_context_free(&ctx->base, owned_copy);
        return LVKW_ERROR;
      }
      memcpy(owned_copy[i].bytes, data[i].data, data[i].size);
    }
  }

  struct wl_data_source *source = lvkw_wl_data_device_manager_create_data_source(
      ctx, ctx->protocols.opt.wl_data_device_manager);
  if (!source) {
    for (uint32_t i = 0; i < count; ++i) lvkw_context_free(&ctx->base, owned_copy[i].bytes);
    lvkw_context_free(&ctx->base, owned_copy);
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Failed to create wl_data_source");
    return LVKW_ERROR;
  }

  lvkw_wl_data_source_add_listener(ctx, source, &_clipboard_data_source_listener, ctx);
  for (uint32_t i = 0; i < count; ++i) {
    lvkw_wl_data_source_offer(ctx, source, owned_copy[i].mime_type);
  }

  lvkw_wl_data_device_set_selection(ctx, ctx->input.data_device, source, ctx->input.clipboard_serial);
  lvkw_wl_display_flush(ctx, ctx->wl.display);
  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) {
    lvkw_wl_data_source_destroy(ctx, source);
    for (uint32_t i = 0; i < count; ++i) lvkw_context_free(&ctx->base, owned_copy[i].bytes);
    lvkw_context_free(&ctx->base, owned_copy);
    return LVKW_ERROR_CONTEXT_LOST;
  }

  if (ctx->input.clipboard.owned_source) {
    lvkw_wl_data_source_destroy(ctx, ctx->input.clipboard.owned_source);
  }
  _clipboard_clear_owned_data(ctx);

  ctx->input.clipboard.owned_source = source;
  ctx->input.clipboard.owned_mimes = owned_copy;
  ctx->input.clipboard.owned_mime_count = count;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardText_WL(LVKW_Window *window, const char **out_text) {
  LVKW_API_VALIDATE(wnd_getClipboardText, window, out_text);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;

  LVKW_Status status = _clipboard_try_get_data(ctx, "text/plain;charset=utf-8", false);
  if (status != LVKW_SUCCESS) {
    status = _clipboard_try_get_data(ctx, "text/plain", false);
  }
  if (status != LVKW_SUCCESS) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                               "Clipboard does not provide text/plain data");
    return LVKW_ERROR;
  }

  if (!_clipboard_ensure_read_cache_nul(ctx)) {
    return LVKW_ERROR;
  }

  *out_text = (const char *)ctx->input.clipboard.read_cache;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardData_WL(LVKW_Window *window, const char *mime_type,
                                         const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(wnd_getClipboardData, window, mime_type, out_data, out_size);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;

  LVKW_Status status = _clipboard_try_get_data(ctx, mime_type, true);
  if (status != LVKW_SUCCESS) return status;

  *out_data = ctx->input.clipboard.read_cache;
  *out_size = ctx->input.clipboard.read_cache_size;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_WL(LVKW_Window *window, const char ***out_mime_types,
                                              uint32_t *count) {
  LVKW_API_VALIDATE(wnd_getClipboardMimeTypes, window, out_mime_types, count);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;

  _clipboard_invalidate_mime_query(ctx);

  const char **src = NULL;
  uint32_t src_count = 0;
  if (ctx->input.clipboard.owned_mime_count > 0) {
    src_count = ctx->input.clipboard.owned_mime_count;
    const char **owned_mimes = lvkw_context_alloc(&ctx->base, sizeof(const char *) * src_count);
    if (!owned_mimes) return LVKW_ERROR;
    for (uint32_t i = 0; i < src_count; ++i) {
      owned_mimes[i] = ctx->input.clipboard.owned_mimes[i].mime_type;
    }
    ctx->input.clipboard.mime_query_ptr = owned_mimes;
    ctx->input.clipboard.mime_query_count = src_count;
  }
  else if (ctx->input.clipboard.selection_offer) {
    LVKW_WaylandDataOffer *meta =
        _lvkw_wayland_offer_meta_get(ctx, ctx->input.clipboard.selection_offer);
    if (meta && meta->mime_count > 0) {
      src = meta->mime_types;
      src_count = meta->mime_count;

      const char **query_mimes = lvkw_context_alloc(&ctx->base, sizeof(const char *) * src_count);
      if (!query_mimes) return LVKW_ERROR;
      memcpy(query_mimes, src, sizeof(const char *) * src_count);
      ctx->input.clipboard.mime_query_ptr = query_mimes;
      ctx->input.clipboard.mime_query_count = src_count;
    }
  }

  *count = ctx->input.clipboard.mime_query_count;
  if (out_mime_types) *out_mime_types = ctx->input.clipboard.mime_query_ptr;
  return LVKW_SUCCESS;
}
