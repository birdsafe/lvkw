// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "lvkw/details/lvkw_config.h"
#include "api_constraints.h"
#include "mem_internal.h"
#include "wayland_internal.h"

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

  uint32_t activation_serial = ctx->input.clipboard_serial;
  if (activation_serial == 0) activation_serial = ctx->input.pointer_serial;

  if (ctx->protocols.wl_seat && activation_serial != 0) {
    lvkw_xdg_activation_token_v1_set_serial(ctx, token, activation_serial, ctx->protocols.wl_seat);
  }
  lvkw_xdg_activation_token_v1_set_surface(ctx, token, window->wl.surface);
  lvkw_xdg_activation_token_v1_commit(ctx, token);

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

/* ext_idle_notification_v1 */

static void _idle_handle_idled(void *data, struct ext_idle_notification_v1 *notification) {
  (void)notification;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {0};
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = true;
  lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                        LVKW_EVENT_TYPE_IDLE_STATE_CHANGED, NULL, &ev);
}

static void _idle_handle_resumed(void *data, struct ext_idle_notification_v1 *notification) {
  (void)notification;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_Event ev = {0};
  ev.idle.timeout_ms = ctx->idle.timeout_ms;
  ev.idle.is_idle = false;
  lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                        LVKW_EVENT_TYPE_IDLE_STATE_CHANGED, NULL, &ev);
}

const struct ext_idle_notification_v1_listener _lvkw_wayland_idle_listener = {
    .idled = _idle_handle_idled,
    .resumed = _idle_handle_resumed,
};

LVKW_Status lvkw_ctx_update_WL(LVKW_Context *ctx_handle, uint32_t field_mask,
                               const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  if (field_mask & LVKW_CONTEXT_ATTR_INHIBIT_IDLE) {
    if (ctx->linux_base.inhibit_idle != attributes->inhibit_idle) {
      ctx->linux_base.inhibit_idle = attributes->inhibit_idle;

      if (ctx->linux_base.inhibit_idle && !ctx->protocols.opt.zwp_idle_inhibit_manager_v1) {
        LVKW_REPORT_CTX_DIAGNOSTIC(ctx_handle, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                   "zwp_idle_inhibit_manager_v1 not available");
      }

      // Update all existing windows
      LVKW_Window_Base *curr = ctx->linux_base.base.prv.window_list;
      while (curr) {
        LVKW_Window_WL *window = (LVKW_Window_WL *)curr;
        if (ctx->linux_base.inhibit_idle) {
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

  _lvkw_update_base_attributes(&ctx->linux_base.base, field_mask, attributes);

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

/* Data Exchange & Selections */

#define LVKW_WL_DND_OFFER_MAGIC 0x4C564B57u

LVKW_WaylandDataOffer *_lvkw_wayland_offer_meta_get(LVKW_Context_WL *ctx,
                                                     struct wl_data_offer *offer) {
  if (!offer) return NULL;
  LVKW_WaylandDataOffer *meta = lvkw_wl_proxy_get_user_data(ctx, (struct wl_proxy *)offer);
  if (!meta || meta->magic != LVKW_WL_DND_OFFER_MAGIC) return NULL;
  return meta;
}

bool _lvkw_wayland_offer_meta_attach(LVKW_Context_WL *ctx, struct wl_data_offer *offer) {
  if (!offer) return false;
  LVKW_WaylandDataOffer *meta = lvkw_context_alloc(&ctx->linux_base.base, sizeof(LVKW_WaylandDataOffer));
  if (!meta) return false;
  memset(meta, 0, sizeof(*meta));
  meta->magic = LVKW_WL_DND_OFFER_MAGIC;
  meta->ctx = ctx;
  lvkw_wl_proxy_set_user_data(ctx, (struct wl_proxy *)offer, meta);
  return true;
}

bool _lvkw_wayland_offer_meta_append_mime(LVKW_Context_WL *ctx, struct wl_data_offer *offer,
                                          const char *mime_type) {
  LVKW_WaylandDataOffer *meta = _lvkw_wayland_offer_meta_get(ctx, offer);
  if (!meta || !mime_type) return false;

  const char *interned = _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache, &ctx->linux_base.base, mime_type);
  if (!interned) return false;

  for (uint32_t i = 0; i < meta->mime_count; ++i) {
    if (strcmp(meta->mime_types[i], interned) == 0) return true;
  }

  if (meta->mime_count == meta->mime_capacity) {
    uint32_t next_capacity = meta->mime_capacity == 0 ? 8 : meta->mime_capacity * 2;
    const char **next =
        lvkw_context_realloc(&ctx->linux_base.base, (void *)meta->mime_types,
                             sizeof(const char *) * meta->mime_capacity,
                             sizeof(const char *) * next_capacity);
    if (!next) return false;
    meta->mime_types = next;
    meta->mime_capacity = next_capacity;
  }

  meta->mime_types[meta->mime_count++] = interned;
  return true;
}

bool _lvkw_wayland_offer_meta_has_mime(LVKW_Context_WL *ctx, const struct wl_data_offer *offer,
                                       const char *mime_type) {
  if (!offer || !mime_type) return false;
  const LVKW_WaylandDataOffer *meta =
      _lvkw_wayland_offer_meta_get(ctx, (struct wl_data_offer *)(void *)offer);
  if (!meta) return false;

  for (uint32_t i = 0; i < meta->mime_count; ++i) {
    if (strcmp(meta->mime_types[i], mime_type) == 0) return true;
  }
  return false;
}

void _lvkw_wayland_offer_destroy(LVKW_Context_WL *ctx, struct wl_data_offer *offer) {
  if (!offer) return;
  LVKW_WaylandDataOffer *meta = _lvkw_wayland_offer_meta_get(ctx, offer);
  if (meta) {
    if (meta->mime_types) {
      lvkw_context_free(&ctx->linux_base.base, (void *)meta->mime_types);
    }
    lvkw_context_free(&ctx->linux_base.base, meta);
    lvkw_wl_proxy_set_user_data(ctx, (struct wl_proxy *)offer, NULL);
  }
  lvkw_wl_data_offer_destroy(ctx, offer);
}

bool _lvkw_wayland_primary_offer_meta_attach(LVKW_Context_WL *ctx, struct zwp_primary_selection_offer_v1 *offer) {
  if (!offer) return false;
  LVKW_WaylandDataOffer *meta = lvkw_context_alloc(&ctx->linux_base.base, sizeof(LVKW_WaylandDataOffer));
  if (!meta) return false;
  memset(meta, 0, sizeof(*meta));
  meta->magic = LVKW_WL_DND_OFFER_MAGIC;
  meta->ctx = ctx;
  lvkw_wl_proxy_set_user_data(ctx, (struct wl_proxy *)offer, meta);
  return true;
}

LVKW_WaylandDataOffer *_lvkw_wayland_primary_offer_meta_get(LVKW_Context_WL *ctx,
                                                            struct zwp_primary_selection_offer_v1 *offer) {
  if (!offer) return NULL;
  LVKW_WaylandDataOffer *meta = lvkw_wl_proxy_get_user_data(ctx, (struct wl_proxy *)offer);
  if (!meta || meta->magic != LVKW_WL_DND_OFFER_MAGIC) return NULL;
  return meta;
}

bool _lvkw_wayland_primary_offer_meta_append_mime(LVKW_Context_WL *ctx, struct zwp_primary_selection_offer_v1 *offer,
                                                  const char *mime_type) {
  LVKW_WaylandDataOffer *meta = _lvkw_wayland_primary_offer_meta_get(ctx, offer);
  if (!meta || !mime_type) return false;

  const char *interned = _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache, &ctx->linux_base.base, mime_type);
  if (!interned) return false;

  for (uint32_t i = 0; i < meta->mime_count; ++i) {
    if (strcmp(meta->mime_types[i], interned) == 0) return true;
  }

  if (meta->mime_count == meta->mime_capacity) {
    uint32_t next_capacity = meta->mime_capacity == 0 ? 8 : meta->mime_capacity * 2;
    const char **next =
        lvkw_context_realloc(&ctx->linux_base.base, (void *)meta->mime_types,
                             sizeof(const char *) * meta->mime_capacity,
                             sizeof(const char *) * next_capacity);
    if (!next) return false;
    meta->mime_types = next;
    meta->mime_capacity = next_capacity;
  }

  meta->mime_types[meta->mime_count++] = interned;
  return true;
}

bool _lvkw_wayland_primary_offer_meta_has_mime(LVKW_Context_WL *ctx, const struct zwp_primary_selection_offer_v1 *offer,
                                               const char *mime_type) {
  if (!offer || !mime_type) return false;
  const LVKW_WaylandDataOffer *meta =
      _lvkw_wayland_primary_offer_meta_get(ctx, (struct zwp_primary_selection_offer_v1 *)(void *)offer);
  if (!meta) return false;

  for (uint32_t i = 0; i < meta->mime_count; ++i) {
    if (strcmp(meta->mime_types[i], mime_type) == 0) return true;
  }
  return false;
}

void _lvkw_wayland_primary_offer_destroy(LVKW_Context_WL *ctx, struct zwp_primary_selection_offer_v1 *offer) {
  if (!offer) return;
  LVKW_WaylandDataOffer *meta = _lvkw_wayland_primary_offer_meta_get(ctx, offer);
  if (meta) {
    if (meta->mime_types) {
      lvkw_context_free(&ctx->linux_base.base, (void *)meta->mime_types);
    }
    lvkw_context_free(&ctx->linux_base.base, meta);
    lvkw_wl_proxy_set_user_data(ctx, (struct wl_proxy *)offer, NULL);
  }
  lvkw_zwp_primary_selection_offer_v1_destroy(ctx, offer);
}

typedef struct LVKW_WaylandSelectionHandler {
  LVKW_DataExchangeTarget target;
  const char *debug_name;
} LVKW_WaylandSelectionHandler;

static bool _wayland_selection_acquire(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                       LVKW_WaylandSelectionHandler *out_handler) {
  LVKW_Window_WL *wl_window = (LVKW_Window_WL *)window;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)wl_window->base.prv.ctx_base;

  if (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    if (!ctx->protocols.opt.wl_data_device_manager || !ctx->input.data_device) {
      LVKW_REPORT_WIND_DIAGNOSTIC(&wl_window->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                  "Clipboard requires wl_data_device_manager");
      return false;
    }
    out_handler->target = target;
    out_handler->debug_name = "clipboard";
    return true;
  }

  if (target == LVKW_DATA_EXCHANGE_TARGET_PRIMARY) {
    if (!ctx->protocols.opt.zwp_primary_selection_device_manager_v1 ||
        !ctx->input.primary_selection_device) {
      LVKW_REPORT_WIND_DIAGNOSTIC(&wl_window->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                  "Primary selection requires zwp_primary_selection_device_manager_v1");
      return false;
    }
    out_handler->target = target;
    out_handler->debug_name = "primary selection";
    return true;
  }

  return false;
}

static void _selection_invalidate_mime_query(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target) {
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
  if (state->mime_query_ptr) {
    lvkw_context_free(&ctx->linux_base.base, (void *)state->mime_query_ptr);
    state->mime_query_ptr = NULL;
  }
  state->mime_query_count = 0;
}

static void _selection_clear_owned_data_generic(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target) {
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
  for (uint32_t i = 0; i < state->owned_mime_count; ++i) {
    lvkw_context_free(&ctx->linux_base.base, state->owned_mimes[i].bytes);
  }
  if (state->owned_mimes) {
    lvkw_context_free(&ctx->linux_base.base, state->owned_mimes);
  }

  state->owned_mimes = NULL;
  state->owned_mime_count = 0;
}

static void _selection_invalidate_read_cache(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target) {
  ctx->input.selections[target].read_cache_size = 0;
}

static bool _selection_ensure_read_cache_capacity(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target, size_t capacity) {
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
  if (capacity <= state->read_cache_capacity) return true;

  uint8_t *next = lvkw_context_realloc(&ctx->linux_base.base, state->read_cache,
                                       state->read_cache_capacity, capacity);
  if (!next) return false;

  state->read_cache = next;
  state->read_cache_capacity = capacity;
  return true;
}

static bool _selection_copy_into_read_cache(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target, const void *data, size_t size,
                                            bool add_nul) {
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
  size_t need_size = size + (add_nul ? 1 : 0);
  if (!_selection_ensure_read_cache_capacity(ctx, target, need_size)) return false;

  if (size > 0) memcpy(state->read_cache, data, size);
  if (add_nul) state->read_cache[size] = '\0';

  state->read_cache_size = size;
  return true;
}

static bool _selection_ensure_read_cache_nul(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target) {
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
  const size_t text_size = state->read_cache_size;
  if (!_selection_ensure_read_cache_capacity(ctx, target, text_size + 1)) return false;
  state->read_cache[text_size] = '\0';
  return true;
}

static LVKW_WaylandClipboardMime *_selection_find_owned_mime(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target,
                                                             const char *mime_type) {
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
  for (uint32_t i = 0; i < state->owned_mime_count; ++i) {
    if (strcmp(state->owned_mimes[i].mime_type, mime_type) == 0) {
      return &state->owned_mimes[i];
    }
  }
  return NULL;
}

static bool _read_offer_generic(LVKW_Context_WL *ctx, void *offer, const char *mime_type,
                                 void (*receive_fn)(LVKW_Context_WL *, void *, const char *, int),
                                 void **out_data, size_t *out_size, bool null_terminate) {
  int pipefd[2];
  if (pipe(pipefd) != 0) return false;

  receive_fn(ctx, offer, mime_type, pipefd[1]);
  close(pipefd[1]);
  lvkw_wl_display_flush(ctx, ctx->wl.display);
  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) {
    close(pipefd[0]);
    return false;
  }

  uint8_t *buffer = NULL;
  size_t size = 0;
  size_t capacity = 0;
  uint8_t tmp[2048];

  int flags = fcntl(pipefd[0], F_GETFL, 0);
  fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

  const int total_timeout_ms = 1000;
  struct timespec start_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  bool success = false;
  for (;;) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int elapsed_ms = (int)((now.tv_sec - start_time.tv_sec) * 1000 +
                           (now.tv_nsec - start_time.tv_nsec) / 1000000);
    int remaining_ms = total_timeout_ms - elapsed_ms;

    if (remaining_ms <= 0) break;

    struct pollfd pfd = {.fd = pipefd[0], .events = POLLIN};
    int ret = poll(&pfd, 1, remaining_ms);

    if (ret < 0) {
      if (errno == EINTR) continue;
      break;
    }
    if (ret == 0) break; // Timeout

    const ssize_t read_size = read(pipefd[0], tmp, sizeof(tmp));
    if (read_size < 0) {
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
      break;
    }
    if (read_size == 0) {
      success = true;
      break; // EOF
    }

    const size_t add_size = (size_t)read_size;
    const size_t needed = size + add_size + (null_terminate ? 1 : 0);
    if (needed > capacity) {
      size_t next_capacity = capacity == 0 ? 2048 : capacity * 2;
      while (next_capacity < needed) next_capacity *= 2;

      uint8_t *next = lvkw_context_realloc(&ctx->linux_base.base, buffer, capacity, next_capacity);
      if (!next) {
        if (buffer) lvkw_context_free(&ctx->linux_base.base, buffer);
        close(pipefd[0]);
        return false;
      }

      buffer = next;
      capacity = next_capacity;
    }

    memcpy(buffer + size, tmp, add_size);
    size += add_size;
  }

  close(pipefd[0]);

  if (!success) {
    if (buffer) lvkw_context_free(&ctx->linux_base.base, buffer);
    return false;
  }


  if (null_terminate && buffer) {
    buffer[size] = '\0';
  }

  *out_data = buffer;
  *out_size = size;
  return true;
}

static void _receive_wl_data_offer(LVKW_Context_WL *ctx, void *offer, const char *mime, int fd) {
  lvkw_wl_data_offer_receive(ctx, (struct wl_data_offer *)offer, mime, fd);
}

static void _receive_primary_selection_offer(LVKW_Context_WL *ctx, void *offer, const char *mime, int fd) {
  lvkw_zwp_primary_selection_offer_v1_receive(ctx, (struct zwp_primary_selection_offer_v1 *)offer, mime, fd);
}

bool _lvkw_wayland_read_data_offer(LVKW_Context_WL *ctx, struct wl_data_offer *offer,
                                   const char *mime_type, void **out_data, size_t *out_size,
                                   bool null_terminate) {
  return _read_offer_generic(ctx, offer, mime_type, _receive_wl_data_offer, out_data, out_size, null_terminate);
}

bool _lvkw_wayland_read_primary_offer(LVKW_Context_WL *ctx, struct zwp_primary_selection_offer_v1 *offer,
                                       const char *mime_type, void **out_data, size_t *out_size,
                                       bool null_terminate) {
  return _read_offer_generic(ctx, offer, mime_type, _receive_primary_selection_offer, out_data, out_size, null_terminate);
}

static void _selection_register_transfer(LVKW_Context_WL *ctx, LVKW_Window_WL *window,
                                         LVKW_DataExchangeTarget target, const char *mime_type,
                                         void *user_tag, void *offer,
                                         void (*receive_fn)(LVKW_Context_WL *, void *, const char *, int)) {
  int pipefd[2];
  if (pipe(pipefd) != 0) {
    LVKW_Event evt = {0};
    evt.data_ready.status = LVKW_ERROR;
    evt.data_ready.user_tag = user_tag;
    evt.data_ready.target = target;
    evt.data_ready.mime_type = _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache, &ctx->linux_base.base, mime_type);
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_DATA_READY, (LVKW_Window *)window, &evt);
    return;
  }

  receive_fn(ctx, offer, mime_type, pipefd[1]);
  close(pipefd[1]);
  lvkw_wl_display_flush(ctx, ctx->wl.display);

  int flags = fcntl(pipefd[0], F_GETFL, 0);
  fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

  LVKW_WaylandTransfer *transfer = lvkw_context_alloc(&ctx->linux_base.base, sizeof(LVKW_WaylandTransfer));
  if (!transfer) {
    close(pipefd[0]);
    return;
  }

  memset(transfer, 0, sizeof(*transfer));
  transfer->fd = pipefd[0];
  transfer->target = target;
  transfer->mime_type = _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache, &ctx->linux_base.base, mime_type);
  transfer->window = window;
  transfer->user_tag = user_tag;
  transfer->next = ctx->pending_transfers;
  ctx->pending_transfers = transfer;
}

LVKW_Status lvkw_wnd_pullTextAsync_WL(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                       void *user_tag) {
  LVKW_API_VALIDATE(data_pullTextAsync, window, target, user_tag);
  LVKW_WaylandSelectionHandler handler = {0};
  if (!_wayland_selection_acquire(window, target, &handler)) return LVKW_ERROR;

  return lvkw_wnd_pullDataAsync_WL(window, target, "text/plain;charset=utf-8", user_tag);
}

LVKW_Status lvkw_wnd_pullDataAsync_WL(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                       const char *mime_type, void *user_tag) {
  LVKW_API_VALIDATE(data_pullDataAsync, window, target, mime_type, user_tag);
  LVKW_WaylandSelectionHandler handler = {0};
  if (!_wayland_selection_acquire(window, target, &handler)) return LVKW_ERROR;

  LVKW_Window_WL *wl_window = (LVKW_Window_WL *)window;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)wl_window->base.prv.ctx_base;
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];

  void *offer = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) ? (void *)state->offer : (void *)state->primary_offer;
  if (!offer) return LVKW_ERROR;

  bool has_mime = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) 
      ? _lvkw_wayland_offer_meta_has_mime(ctx, state->offer, mime_type)
      : _lvkw_wayland_primary_offer_meta_has_mime(ctx, state->primary_offer, mime_type);

  if (!has_mime) return LVKW_ERROR;

  void (*receive_fn)(LVKW_Context_WL *, void *, const char *, int) = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) ? _receive_wl_data_offer : _receive_primary_selection_offer;
  _selection_register_transfer(ctx, wl_window, target, mime_type, user_tag, offer, receive_fn);

  return LVKW_SUCCESS;
}

int _lvkw_wayland_get_transfer_poll_fds(LVKW_Context_WL *ctx, struct pollfd *pfds, int max_count) {
  int count = 0;
  LVKW_WaylandTransfer *curr = ctx->pending_transfers;
  while (curr && count < max_count) {
    pfds[count].fd = curr->fd;
    pfds[count].events = POLLIN;
    curr = curr->next;
    count++;
  }
  return count;
}

void _lvkw_wayland_process_transfers(LVKW_Context_WL *ctx, const struct pollfd *pfds,
                                     int pfds_offset) {
  LVKW_WaylandTransfer **curr_ptr = &ctx->pending_transfers;
  int i = 0;

  while (*curr_ptr) {
    LVKW_WaylandTransfer *transfer = *curr_ptr;
    bool ready = (pfds[pfds_offset + i].revents & POLLIN) != 0;
    bool error = (pfds[pfds_offset + i].revents & (POLLERR | POLLNVAL | POLLHUP)) != 0;
    bool done = false;

    if (ready) {
      uint8_t tmp[4096];
      ssize_t n = read(transfer->fd, tmp, sizeof(tmp));
      if (n > 0) {
        if (transfer->size + (size_t)n > transfer->capacity) {
          size_t next_cap = transfer->capacity == 0 ? 4096 : transfer->capacity * 2;
          while (next_cap < transfer->size + (size_t)n) next_cap *= 2;
          uint8_t *next_buf = lvkw_context_realloc(&ctx->linux_base.base, transfer->buffer, transfer->capacity, next_cap);
          if (next_buf) {
            transfer->buffer = next_buf;
            transfer->capacity = next_cap;
          } else {
            error = true;
          }
        }
        if (!error) {
          memcpy(transfer->buffer + transfer->size, tmp, (size_t)n);
          transfer->size += (size_t)n;
        }
      } else if (n == 0) {
        done = true;
      } else if (errno != EAGAIN && errno != EINTR) {
        error = true;
      }
    }

    if (done || error) {
      LVKW_Event evt = {0};
      evt.data_ready.status = error ? LVKW_ERROR : LVKW_SUCCESS;
      evt.data_ready.user_tag = transfer->user_tag;
      evt.data_ready.target = transfer->target;
      evt.data_ready.mime_type = transfer->mime_type;
      
      if (!error && transfer->size > 0) {
        evt.data_ready.data = lvkw_event_queue_transient_intern_sized(&ctx->linux_base.base.prv.event_queue, (const char *)transfer->buffer, transfer->size);
        evt.data_ready.size = transfer->size;
      }

      lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_DATA_READY, (LVKW_Window *)transfer->window, &evt);

      close(transfer->fd);
      if (transfer->buffer) lvkw_context_free(&ctx->linux_base.base, transfer->buffer);
      *curr_ptr = transfer->next;
      lvkw_context_free(&ctx->linux_base.base, transfer);
    } else {
      curr_ptr = &transfer->next;
      i++;
    }
  }
}

static LVKW_Status _selection_try_get_data(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target,
                                           const char *mime_type, bool report_missing) {
  _selection_invalidate_read_cache(ctx, target);
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];

  LVKW_WaylandClipboardMime *owned = _selection_find_owned_mime(ctx, target, mime_type);
  if (owned) {
    if (!_selection_copy_into_read_cache(ctx, target, owned->bytes, owned->size, false)) return LVKW_ERROR;
    return LVKW_SUCCESS;
  }

  bool has_offer = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) ? (state->offer != NULL) : (state->primary_offer != NULL);
  if (!has_offer) {
    if (report_missing) {
      LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                 "No selection is available for the target");
    }
    return LVKW_ERROR;
  }

  bool has_mime = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) 
      ? _lvkw_wayland_offer_meta_has_mime(ctx, state->offer, mime_type)
      : _lvkw_wayland_primary_offer_meta_has_mime(ctx, state->primary_offer, mime_type);

  if (!has_mime) {
    if (report_missing) {
      LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                                 "Requested MIME type not present in selection");
    }
    return LVKW_ERROR;
  }

  void *offer_data = NULL;
  size_t offer_size = 0;
  bool read_success = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD)
      ? _lvkw_wayland_read_data_offer(ctx, state->offer, mime_type, &offer_data, &offer_size, false)
      : _lvkw_wayland_read_primary_offer(ctx, state->primary_offer, mime_type, &offer_data, &offer_size, false);

  if (!read_success) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Selection transfer failed");
    return LVKW_ERROR;
  }

  const bool copied = _selection_copy_into_read_cache(ctx, target, offer_data, offer_size, false);
  if (offer_data) lvkw_context_free(&ctx->linux_base.base, offer_data);
  return copied ? LVKW_SUCCESS : LVKW_ERROR;
}

static void _selection_source_send_generic(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target,
                                          const char *mime_type, int32_t fd) {
  if (!ctx || !mime_type) {
    close(fd);
    return;
  }

  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
  LVKW_WaylandClipboardMime *mime = NULL;
  for (uint32_t i = 0; i < state->owned_mime_count; ++i) {
    if (strcmp(state->owned_mimes[i].mime_type, mime_type) == 0) {
      mime = &state->owned_mimes[i];
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
    if (written > 0) {
      bytes += (size_t)written;
      remaining -= (size_t)written;
    } else if (written < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        struct pollfd pfd = {.fd = fd, .events = POLLOUT};
        if (poll(&pfd, 1, 100) > 0) continue;
      }
      break;
    } else {
      break;
    }
  }

  close(fd);
}

static void _selection_source_cancelled_generic(LVKW_Context_WL *ctx, LVKW_DataExchangeTarget target) {
  if (!ctx) return;
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];

  if (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    if (state->source) {
      lvkw_wl_data_source_destroy(ctx, state->source);
      state->source = NULL;
    }
  } else {
    if (state->primary_source) {
      lvkw_zwp_primary_selection_source_v1_destroy(ctx, state->primary_source);
      state->primary_source = NULL;
    }
  }

  _selection_clear_owned_data_generic(ctx, target);
}

static void _clipboard_data_source_send(void *data, struct wl_data_source *source,
                                        const char *mime_type, int32_t fd) {
  _selection_source_send_generic((LVKW_Context_WL *)data, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, mime_type, fd);
  (void)source;
}

static void _clipboard_data_source_cancelled(void *data, struct wl_data_source *source) {
  _selection_source_cancelled_generic((LVKW_Context_WL *)data, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD);
  (void)source;
}

static void _primary_selection_source_send(void *data, struct zwp_primary_selection_source_v1 *source,
                                           const char *mime_type, int32_t fd) {
  _selection_source_send_generic((LVKW_Context_WL *)data, LVKW_DATA_EXCHANGE_TARGET_PRIMARY, mime_type, fd);
  (void)source;
}

static void _primary_selection_source_cancelled(void *data, struct zwp_primary_selection_source_v1 *source) {
  _selection_source_cancelled_generic((LVKW_Context_WL *)data, LVKW_DATA_EXCHANGE_TARGET_PRIMARY);
  (void)source;
}

static const struct zwp_primary_selection_source_v1_listener _primary_selection_source_listener = {
    .send = _primary_selection_source_send,
    .cancelled = _primary_selection_source_cancelled,
};

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

LVKW_Status lvkw_wnd_setClipboardText_WL(LVKW_Window *window, const char *text) {
  return lvkw_wnd_pushText_WL(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, text);
}

LVKW_Status lvkw_wnd_setClipboardData_WL(LVKW_Window *window, const LVKW_ClipboardData *data,
                                         uint32_t count) {
  return lvkw_wnd_pushData_WL(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, (const LVKW_DataBuffer *)data, count);
}

LVKW_Status lvkw_wnd_getClipboardText_WL(LVKW_Window *window, const char **out_text) {
  return lvkw_wnd_pullText_WL(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, out_text);
}

LVKW_Status lvkw_wnd_getClipboardData_WL(LVKW_Window *window, const char *mime_type,
                                         const void **out_data, size_t *out_size) {
  return lvkw_wnd_pullData_WL(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, mime_type, out_data, out_size);
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_WL(LVKW_Window *window, const char ***out_mime_types,
                                              uint32_t *count) {
  return lvkw_wnd_listBufferMimeTypes_WL(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, out_mime_types, count);
}

LVKW_Status lvkw_wnd_pushText_WL(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                 const char *text) {
  LVKW_API_VALIDATE(data_pushText, window, target, text);
  LVKW_WaylandSelectionHandler handler = {0};
  if (!_wayland_selection_acquire(window, target, &handler)) return LVKW_ERROR;
  (void)handler;

  const size_t text_size = strlen(text);
  const LVKW_DataBuffer items[2] = {
      {.mime_type = "text/plain;charset=utf-8", .data = text, .size = text_size},
      {.mime_type = "text/plain", .data = text, .size = text_size},
  };
  return lvkw_wnd_pushData_WL(window, target, items, 2);
}

LVKW_Status lvkw_wnd_pullText_WL(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                 const char **out_text) {
  LVKW_API_VALIDATE(data_pullText, window, target, out_text);
  LVKW_WaylandSelectionHandler handler = {0};
  if (!_wayland_selection_acquire(window, target, &handler)) return LVKW_ERROR;
  (void)handler;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;

  LVKW_Status status = _selection_try_get_data(ctx, target, "text/plain;charset=utf-8", false);
  if (status != LVKW_SUCCESS) {
    status = _selection_try_get_data(ctx, target, "text/plain", false);
  }
  if (status != LVKW_SUCCESS) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                               "Selection does not provide text/plain data");
    return LVKW_ERROR;
  }

  if (!_selection_ensure_read_cache_nul(ctx, target)) {
    return LVKW_ERROR;
  }

  *out_text = (const char *)ctx->input.selections[target].read_cache;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_pushData_WL(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                 const LVKW_DataBuffer *data, uint32_t count) {
  LVKW_API_VALIDATE(data_pushData, window, target, data, count);
  LVKW_WaylandSelectionHandler handler = {0};
  if (!_wayland_selection_acquire(window, target, &handler)) return LVKW_ERROR;
  (void)handler;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];

  if (ctx->input.clipboard_serial == 0) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_PRECONDITION_FAILURE,
                               "No valid input serial available for selection ownership");
    return LVKW_ERROR;
  }

  _selection_invalidate_mime_query(ctx, target);

  if (count == 0) {
    if (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
      lvkw_wl_data_device_set_selection(ctx, ctx->input.data_device, NULL, ctx->input.clipboard_serial);
    } else {
      lvkw_zwp_primary_selection_device_v1_set_selection(ctx, ctx->input.primary_selection_device, NULL, ctx->input.clipboard_serial);
    }
    lvkw_wl_display_flush(ctx, ctx->wl.display);
    _lvkw_wayland_check_error(ctx);
    if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

    if (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
      if (state->source) {
        lvkw_wl_data_source_destroy(ctx, state->source);
        state->source = NULL;
      }
    } else {
      if (state->primary_source) {
        lvkw_zwp_primary_selection_source_v1_destroy(ctx, state->primary_source);
        state->primary_source = NULL;
      }
    }
    _selection_clear_owned_data_generic(ctx, target);
    return LVKW_SUCCESS;
  }

  LVKW_WaylandClipboardMime *owned_copy =
      lvkw_context_alloc(&ctx->linux_base.base, sizeof(LVKW_WaylandClipboardMime) * count);
  if (!owned_copy) return LVKW_ERROR;
  memset(owned_copy, 0, sizeof(LVKW_WaylandClipboardMime) * count);

  for (uint32_t i = 0; i < count; ++i) {
    owned_copy[i].mime_type = _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache, &ctx->linux_base.base, data[i].mime_type);
    if (!owned_copy[i].mime_type) {
      for (uint32_t j = 0; j < i; ++j) lvkw_context_free(&ctx->linux_base.base, owned_copy[j].bytes);
      lvkw_context_free(&ctx->linux_base.base, owned_copy);
      return LVKW_ERROR;
    }
    owned_copy[i].size = data[i].size;
    if (data[i].size > 0) {
      owned_copy[i].bytes = lvkw_context_alloc(&ctx->linux_base.base, data[i].size);
      if (!owned_copy[i].bytes) {
        for (uint32_t j = 0; j < i; ++j) lvkw_context_free(&ctx->linux_base.base, owned_copy[j].bytes);
        lvkw_context_free(&ctx->linux_base.base, owned_copy);
        return LVKW_ERROR;
      }
      memcpy(owned_copy[i].bytes, data[i].data, data[i].size);
    }
  }

  if (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    struct wl_data_source *source = lvkw_wl_data_device_manager_create_data_source(
        ctx, ctx->protocols.opt.wl_data_device_manager);
    if (!source) goto error_cleanup;
    lvkw_wl_data_source_add_listener(ctx, source, &_clipboard_data_source_listener, ctx);
    for (uint32_t i = 0; i < count; ++i) {
      lvkw_wl_data_source_offer(ctx, source, owned_copy[i].mime_type);
    }
    lvkw_wl_data_device_set_selection(ctx, ctx->input.data_device, source, ctx->input.clipboard_serial);
    if (state->source) lvkw_wl_data_source_destroy(ctx, state->source);
    state->source = source;
  } else {
    struct zwp_primary_selection_source_v1 *source = lvkw_zwp_primary_selection_device_manager_v1_create_source(
        ctx, ctx->protocols.opt.zwp_primary_selection_device_manager_v1);
    if (!source) goto error_cleanup;
    lvkw_zwp_primary_selection_source_v1_add_listener(ctx, source, &_primary_selection_source_listener, ctx);
    for (uint32_t i = 0; i < count; ++i) {
      lvkw_zwp_primary_selection_source_v1_offer(ctx, source, owned_copy[i].mime_type);
    }
    lvkw_zwp_primary_selection_device_v1_set_selection(ctx, ctx->input.primary_selection_device, source, ctx->input.clipboard_serial);
    if (state->primary_source) lvkw_zwp_primary_selection_source_v1_destroy(ctx, state->primary_source);
    state->primary_source = source;
  }

  lvkw_wl_display_flush(ctx, ctx->wl.display);
  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  _selection_clear_owned_data_generic(ctx, target);
  state->owned_mimes = owned_copy;
  state->owned_mime_count = count;
  return LVKW_SUCCESS;

error_cleanup:
  for (uint32_t i = 0; i < count; ++i) lvkw_context_free(&ctx->linux_base.base, owned_copy[i].bytes);
  lvkw_context_free(&ctx->linux_base.base, owned_copy);
  LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                             "Failed to create selection source");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_pullData_WL(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                 const char *mime_type, const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(data_pullData, window, target, mime_type, out_data, out_size);
  LVKW_WaylandSelectionHandler handler = {0};
  if (!_wayland_selection_acquire(window, target, &handler)) return LVKW_ERROR;
  (void)handler;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;
  LVKW_Status status = _selection_try_get_data(ctx, target, mime_type, true);
  if (status != LVKW_SUCCESS) return status;

  *out_data = ctx->input.selections[target].read_cache;
  *out_size = ctx->input.selections[target].read_cache_size;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_listBufferMimeTypes_WL(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                            const char ***out_mime_types, uint32_t *count) {
  LVKW_API_VALIDATE(data_listBufferMimeTypes, window, target, out_mime_types, count);
  LVKW_WaylandSelectionHandler handler = {0};
  if (!_wayland_selection_acquire(window, target, &handler)) return LVKW_ERROR;
  (void)handler;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)((LVKW_Window_WL *)window)->base.prv.ctx_base;
  LVKW_WaylandSelectionState *state = &ctx->input.selections[target];

  _selection_invalidate_mime_query(ctx, target);

  const char **src = NULL;
  uint32_t src_count = 0;
  if (state->owned_mime_count > 0) {
    src_count = state->owned_mime_count;
    const char **owned_mimes = lvkw_context_alloc(&ctx->linux_base.base, sizeof(const char *) * src_count);
    if (!owned_mimes) return LVKW_ERROR;
    for (uint32_t i = 0; i < src_count; ++i) {
      owned_mimes[i] = state->owned_mimes[i].mime_type;
    }
    state->mime_query_ptr = owned_mimes;
    state->mime_query_count = src_count;
  }
  else {
    bool has_offer = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) ? (state->offer != NULL) : (state->primary_offer != NULL);
    if (has_offer) {
      LVKW_WaylandDataOffer *meta = (target == LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD)
          ? _lvkw_wayland_offer_meta_get(ctx, state->offer)
          : _lvkw_wayland_primary_offer_meta_get(ctx, state->primary_offer);
      
      if (meta && meta->mime_count > 0) {
        src = meta->mime_types;
        src_count = meta->mime_count;

        const char **query_mimes = lvkw_context_alloc(&ctx->linux_base.base, sizeof(const char *) * src_count);
        if (!query_mimes) return LVKW_ERROR;
        memcpy(query_mimes, src, sizeof(const char *) * src_count);
        state->mime_query_ptr = query_mimes;
        state->mime_query_count = src_count;
      }
    }
  }

  *count = state->mime_query_count;
  if (out_mime_types) *out_mime_types = state->mime_query_ptr;
  return LVKW_SUCCESS;
}
