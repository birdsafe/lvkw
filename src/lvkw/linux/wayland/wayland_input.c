// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#define _GNU_SOURCE
#include <linux/input-event-codes.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "lvkw_api_constraints.h"
#include "lvkw_assume.h"
#include "lvkw_linux_internal.h"
#include "lvkw_mem_internal.h"
#include "lvkw_wayland_internal.h"

static void _map_text_input_type(LVKW_TextInputType type, uint32_t *out_hint, uint32_t *out_purpose) {
  uint32_t hint = ZWP_TEXT_INPUT_V3_CONTENT_HINT_NONE;
  uint32_t purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL;

  switch (type) {
    case LVKW_TEXT_INPUT_TYPE_TEXT:
      purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL;
      break;
    case LVKW_TEXT_INPUT_TYPE_PASSWORD:
      hint = ZWP_TEXT_INPUT_V3_CONTENT_HINT_HIDDEN_TEXT |
             ZWP_TEXT_INPUT_V3_CONTENT_HINT_SENSITIVE_DATA;
      purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_PASSWORD;
      break;
    case LVKW_TEXT_INPUT_TYPE_EMAIL:
      purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_EMAIL;
      break;
    case LVKW_TEXT_INPUT_TYPE_NUMERIC:
      hint = ZWP_TEXT_INPUT_V3_CONTENT_HINT_NONE;
      purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NUMBER;
      break;
    case LVKW_TEXT_INPUT_TYPE_NONE:
    default:
      purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL;
      break;
  }

  *out_hint = hint;
  *out_purpose = purpose;
}

static uint32_t _clamp_signed_to_u32_len(int32_t value, uint32_t len) {
  if (value < 0) return len;
  if ((uint32_t)value > len) return len;
  return (uint32_t)value;
}

static bool _is_text_input_v3_active(const LVKW_Context_WL *ctx, const LVKW_Window_WL *window) {
  if (!ctx->input.text_input || !window) return false;
  return window->text_input_type != LVKW_TEXT_INPUT_TYPE_NONE;
}

void _lvkw_wayland_sync_text_input_state(LVKW_Context_WL *ctx, LVKW_Window_WL *window) {
  if (!ctx->input.text_input) return;

  if (!window || window->text_input_type == LVKW_TEXT_INPUT_TYPE_NONE) {
    lvkw_zwp_text_input_v3_disable(ctx, ctx->input.text_input);
    lvkw_zwp_text_input_v3_commit(ctx, ctx->input.text_input);
    return;
  }

  uint32_t hint = ZWP_TEXT_INPUT_V3_CONTENT_HINT_NONE;
  uint32_t purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL;
  _map_text_input_type(window->text_input_type, &hint, &purpose);

  lvkw_zwp_text_input_v3_enable(ctx, ctx->input.text_input);
  lvkw_zwp_text_input_v3_set_content_type(ctx, ctx->input.text_input, hint, purpose);
  lvkw_zwp_text_input_v3_set_cursor_rectangle(
      ctx, ctx->input.text_input, (int32_t)window->text_input_rect.origin.x,
      (int32_t)window->text_input_rect.origin.y,
      (int32_t)window->text_input_rect.size.x, (int32_t)window->text_input_rect.size.y);
  lvkw_zwp_text_input_v3_commit(ctx, ctx->input.text_input);
}

/* wl_keyboard */

static void _keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format,
                                    int fd, uint32_t size) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(&ctx->base, ctx != NULL, "Context handle must not be NULL in keymap handler");
  LVKW_CTX_ASSUME(&ctx->base, keyboard != NULL, "Keyboard must not be NULL in keymap handler");

  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
    close(fd);
    return;
  }

  char *map_str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (map_str == MAP_FAILED) {
    close(fd);
    return;
  }

  struct xkb_keymap *keymap = lvkw_xkb_keymap_new_from_string(
      ctx, ctx->input.xkb.ctx, map_str, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_str, size);
  close(fd);

  if (!keymap) {
    LVKW_REPORT_CTX_DIAGNOSTIC(data, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Failed to compile keymap");
    return;
  }

  struct xkb_state *state = lvkw_xkb_state_new(ctx, keymap);
  if (!state) {
    LVKW_REPORT_CTX_DIAGNOSTIC(data, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Failed to create xkb state");
    lvkw_xkb_keymap_unref(ctx, keymap);
    return;
  }

  if (ctx->input.xkb.keymap) lvkw_xkb_keymap_unref(ctx, ctx->input.xkb.keymap);
  if (ctx->input.xkb.state) lvkw_xkb_state_unref(ctx, ctx->input.xkb.state);

  ctx->input.xkb.keymap = keymap;
  ctx->input.xkb.state = state;

  ctx->input.xkb.mod_indices.shift =
      lvkw_xkb_keymap_mod_get_index(ctx, keymap, XKB_MOD_NAME_SHIFT);
  ctx->input.xkb.mod_indices.ctrl =
      lvkw_xkb_keymap_mod_get_index(ctx, keymap, XKB_MOD_NAME_CTRL);
  ctx->input.xkb.mod_indices.alt =
      lvkw_xkb_keymap_mod_get_index(ctx, keymap, XKB_MOD_NAME_ALT);
  ctx->input.xkb.mod_indices.super =
      lvkw_xkb_keymap_mod_get_index(ctx, keymap, XKB_MOD_NAME_LOGO);
  ctx->input.xkb.mod_indices.caps =
      lvkw_xkb_keymap_mod_get_index(ctx, keymap, XKB_MOD_NAME_CAPS);
  ctx->input.xkb.mod_indices.num =
      lvkw_xkb_keymap_mod_get_index(ctx, keymap, XKB_MOD_NAME_NUM);
}

static LVKW_Window_WL *_surface_to_live_window(LVKW_Context_WL *ctx, struct wl_surface *surface) {
  if (!surface) return NULL;
  LVKW_Window_WL *window = lvkw_wl_proxy_get_user_data(ctx, (struct wl_proxy *)surface);
  if (!window) return NULL;

  LVKW_Window_Base *curr = ctx->base.prv.window_list;
  while (curr) {
    if ((LVKW_Window_WL *)curr == window) return window;
    curr = curr->prv.next;
  }

  return NULL;
}

static void _keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                   struct wl_surface *surface, struct wl_array *keys) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(data, ctx != NULL, "Context handle must not be NULL in keyboard enter handler");
  LVKW_CTX_ASSUME(data, keyboard != NULL, "Keyboard must not be NULL in keyboard enter handler");
  LVKW_CTX_ASSUME(data, surface != NULL, "Surface must not be NULL in keyboard enter handler");

  ctx->input.clipboard_serial = serial;
  ctx->input.keyboard_focus = _surface_to_live_window(ctx, surface);
  if (ctx->input.keyboard_focus) {
    _lvkw_wayland_sync_text_input_state(ctx, ctx->input.keyboard_focus);
  }
}

static void _keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                   struct wl_surface *surface) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(&ctx->base, ctx != NULL,
                  "Context handle must not be NULL in keyboard leave handler");
  LVKW_CTX_ASSUME(&ctx->base, keyboard != NULL,
                  "Keyboard must not be NULL in keyboard leave handler");

  _lvkw_wayland_sync_text_input_state(ctx, NULL);
  ctx->input.keyboard_focus = NULL;
}

static void _keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                 uint32_t time, uint32_t key, uint32_t state) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(&ctx->base, ctx != NULL, "Context handle must not be NULL in key handler");
  LVKW_CTX_ASSUME(&ctx->base, keyboard != NULL, "Keyboard must not be NULL in key handler");

  ctx->input.clipboard_serial = serial;
  if (!ctx->input.keyboard_focus) return;

  uint32_t modifiers = 0;
  if (ctx->input.xkb.state) {
    lvkw_xkb_state_key_get_one_sym(ctx, ctx->input.xkb.state, key + 8);

    xkb_mod_mask_t mask =
        lvkw_xkb_state_serialize_mods(ctx, ctx->input.xkb.state, XKB_STATE_MODS_EFFECTIVE);

    if (ctx->input.xkb.mod_indices.shift != XKB_MOD_INVALID &&
        (mask & (1 << ctx->input.xkb.mod_indices.shift)))
      modifiers |= LVKW_MODIFIER_SHIFT;
    if (ctx->input.xkb.mod_indices.ctrl != XKB_MOD_INVALID &&
        (mask & (1 << ctx->input.xkb.mod_indices.ctrl)))
      modifiers |= LVKW_MODIFIER_CONTROL;
    if (ctx->input.xkb.mod_indices.alt != XKB_MOD_INVALID &&
        (mask & (1 << ctx->input.xkb.mod_indices.alt)))
      modifiers |= LVKW_MODIFIER_ALT;
    if (ctx->input.xkb.mod_indices.super != XKB_MOD_INVALID &&
        (mask & (1 << ctx->input.xkb.mod_indices.super)))
      modifiers |= LVKW_MODIFIER_META;
    if (ctx->input.xkb.mod_indices.caps != XKB_MOD_INVALID &&
        (mask & (1 << ctx->input.xkb.mod_indices.caps)))
      modifiers |= LVKW_MODIFIER_CAPS_LOCK;
    if (ctx->input.xkb.mod_indices.num != XKB_MOD_INVALID &&
        (mask & (1 << ctx->input.xkb.mod_indices.num)))
      modifiers |= LVKW_MODIFIER_NUM_LOCK;
  }

  LVKW_Event evt = {0};

  evt.key.key = lvkw_linux_translate_keycode(key);
  evt.key.state = (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? LVKW_BUTTON_STATE_PRESSED
                                                           : LVKW_BUTTON_STATE_RELEASED;
  evt.key.modifiers = modifiers;

  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_KEY,
                        (LVKW_Window *)ctx->input.keyboard_focus, &evt);

  if (state == WL_KEYBOARD_KEY_STATE_PRESSED && ctx->input.xkb.state &&
      !_is_text_input_v3_active(ctx, ctx->input.keyboard_focus)) {
    char buffer[64];
    int len = lvkw_xkb_state_key_get_utf8(ctx, ctx->input.xkb.state, key + 8, buffer, sizeof(buffer));

    if (len > 0) {
      LVKW_Event text_evt = {0};
      text_evt.text_input.text = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, buffer);
      text_evt.text_input.length = (uint32_t)len;
      lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_TEXT_INPUT,
                            (LVKW_Window *)ctx->input.keyboard_focus, &text_evt);
    }
  }
}

static void _keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                       uint32_t mods_depressed, uint32_t mods_latched,
                                       uint32_t mods_locked, uint32_t group) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_CTX_ASSUME(&ctx->base, ctx != NULL, "Context handle must not be NULL in modifiers handler");

  if (ctx->input.xkb.state) {
    lvkw_xkb_state_update_mask(ctx, ctx->input.xkb.state, mods_depressed, mods_latched,
                               mods_locked, 0, 0, group);
  }
}

static void _keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard, int32_t rate,
                                         int32_t delay) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  ctx->input.repeat.rate = rate;
  ctx->input.repeat.delay = delay;
}

static const struct wl_keyboard_listener _keyboard_listener = {
    .keymap = _keyboard_handle_keymap,
    .enter = _keyboard_handle_enter,
    .leave = _keyboard_handle_leave,
    .key = _keyboard_handle_key,
    .modifiers = _keyboard_handle_modifiers,
    .repeat_info = _keyboard_handle_repeat_info,
};

/* zwp_text_input_v3 */

static void _text_input_handle_enter(void *data, struct zwp_text_input_v3 *text_input,
                                     struct wl_surface *surface) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  ctx->input.keyboard_focus = _surface_to_live_window(ctx, surface);
  if (ctx->input.keyboard_focus) {
    _lvkw_wayland_sync_text_input_state(ctx, ctx->input.keyboard_focus);
  }
  (void)text_input;
}
static void _text_input_handle_leave(void *data, struct zwp_text_input_v3 *text_input,
                                     struct wl_surface *surface) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = _surface_to_live_window(ctx, surface);
  if (!window) window = ctx->input.keyboard_focus;

  if (window && ctx->input.text_input_pending.preedit_length > 0) {
    LVKW_Event evt = {0};
    evt.text_composition.text = "";
    evt.text_composition.length = 0;
    evt.text_composition.cursor_index = 0;
    evt.text_composition.selection_length = 0;
    lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_TEXT_COMPOSITION,
                          (LVKW_Window *)window, &evt);
  }

  memset(&ctx->input.text_input_pending, 0, sizeof(ctx->input.text_input_pending));
  (void)text_input;
}
static void _text_input_handle_preedit_string(void *data, struct zwp_text_input_v3 *text_input,
                                              const char *text, int32_t cursor_begin,
                                              int32_t cursor_end) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  const char *safe_text = text ? text : "";
  const char *stored = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, safe_text);
  uint32_t len = (uint32_t)strlen(safe_text);

  ctx->input.text_input_pending.preedit_text = stored;
  ctx->input.text_input_pending.preedit_length = len;
  ctx->input.text_input_pending.preedit_cursor_begin = cursor_begin;
  ctx->input.text_input_pending.preedit_cursor_end = cursor_end;
  ctx->input.text_input_pending.preedit_dirty = true;
  (void)text_input;
}
static void _text_input_handle_commit_string(void *data, struct zwp_text_input_v3 *text_input,
                                             const char *text) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  const char *safe_text = text ? text : "";
  const char *stored = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, safe_text);

  ctx->input.text_input_pending.commit_text = stored;
  ctx->input.text_input_pending.commit_length = (uint32_t)strlen(safe_text);
  ctx->input.text_input_pending.commit_dirty = true;
  (void)text_input;
}
static void _text_input_handle_delete_surrounding_text(void *data,
                                                       struct zwp_text_input_v3 *text_input,
                                                       uint32_t before_length,
                                                       uint32_t after_length) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  ctx->input.text_input_pending.delete_before_length = before_length;
  ctx->input.text_input_pending.delete_after_length = after_length;
  ctx->input.text_input_pending.delete_dirty = true;
  (void)text_input;
}
static void _text_input_handle_done(void *data, struct zwp_text_input_v3 *text_input,
                                    uint32_t serial) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = ctx->input.keyboard_focus;
  if (!window) {
    memset(&ctx->input.text_input_pending, 0, sizeof(ctx->input.text_input_pending));
    return;
  }

  if (ctx->input.text_input_pending.preedit_dirty || ctx->input.text_input_pending.delete_dirty) {
    LVKW_Event composition_evt = {0};
    const uint32_t len = ctx->input.text_input_pending.preedit_length;
    uint32_t begin = _clamp_signed_to_u32_len(ctx->input.text_input_pending.preedit_cursor_begin, len);
    uint32_t end = _clamp_signed_to_u32_len(ctx->input.text_input_pending.preedit_cursor_end, len);

    if (ctx->input.text_input_pending.delete_dirty && len > 0) {
      uint32_t cursor = begin;
      if (cursor > len) cursor = len;

      uint32_t del_before = ctx->input.text_input_pending.delete_before_length;
      uint32_t del_after = ctx->input.text_input_pending.delete_after_length;
      if (del_before > cursor) del_before = cursor;
      if (del_after > (len - cursor)) del_after = (len - cursor);

      begin = cursor - del_before;
      end = cursor + del_after;
    }

    if (end < begin) {
      uint32_t tmp = begin;
      begin = end;
      end = tmp;
    }

    composition_evt.text_composition.text = ctx->input.text_input_pending.preedit_text
                                                ? ctx->input.text_input_pending.preedit_text
                                                : "";
    composition_evt.text_composition.length = len;
    composition_evt.text_composition.cursor_index = begin;
    composition_evt.text_composition.selection_length = end - begin;
    lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_TEXT_COMPOSITION,
                          (LVKW_Window *)window, &composition_evt);
  }

  if (ctx->input.text_input_pending.commit_dirty &&
      ctx->input.text_input_pending.commit_length > 0) {
    LVKW_Event text_evt = {0};
    text_evt.text_input.text = ctx->input.text_input_pending.commit_text;
    text_evt.text_input.length = ctx->input.text_input_pending.commit_length;
    lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_TEXT_INPUT,
                          (LVKW_Window *)window, &text_evt);
  }

  memset(&ctx->input.text_input_pending, 0, sizeof(ctx->input.text_input_pending));
  (void)text_input;
  (void)serial;
}

static const struct zwp_text_input_v3_listener _text_input_listener = {
    .enter = _text_input_handle_enter,
    .leave = _text_input_handle_leave,
    .preedit_string = _text_input_handle_preedit_string,
    .commit_string = _text_input_handle_commit_string,
    .delete_surrounding_text = _text_input_handle_delete_surrounding_text,
    .done = _text_input_handle_done,
};

/* wl_data_device / DnD */

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
  LVKW_WaylandDataOffer *meta = lvkw_context_alloc(&ctx->base, sizeof(LVKW_WaylandDataOffer));
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

  const char *interned = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, mime_type);
  if (!interned) return false;

  for (uint32_t i = 0; i < meta->mime_count; ++i) {
    if (strcmp(meta->mime_types[i], interned) == 0) return true;
  }

  if (meta->mime_count == meta->mime_capacity) {
    uint32_t next_capacity = meta->mime_capacity == 0 ? 8 : meta->mime_capacity * 2;
    const char **next =
        lvkw_context_realloc(&ctx->base, (void *)meta->mime_types,
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
      lvkw_context_free(&ctx->base, (void *)meta->mime_types);
    }
    lvkw_context_free(&ctx->base, meta);
    lvkw_wl_proxy_set_user_data(ctx, (struct wl_proxy *)offer, NULL);
  }
  lvkw_wl_data_offer_destroy(ctx, offer);
}

static LVKW_ModifierFlags _current_modifiers(const LVKW_Context_WL *ctx) {
  if (!ctx->input.xkb.state) return 0;

  LVKW_ModifierFlags modifiers = 0;
  xkb_mod_mask_t mask =
      lvkw_xkb_state_serialize_mods(ctx, ctx->input.xkb.state, XKB_STATE_MODS_EFFECTIVE);

  if (ctx->input.xkb.mod_indices.shift != XKB_MOD_INVALID &&
      (mask & (1 << ctx->input.xkb.mod_indices.shift)))
    modifiers |= LVKW_MODIFIER_SHIFT;
  if (ctx->input.xkb.mod_indices.ctrl != XKB_MOD_INVALID &&
      (mask & (1 << ctx->input.xkb.mod_indices.ctrl)))
    modifiers |= LVKW_MODIFIER_CONTROL;
  if (ctx->input.xkb.mod_indices.alt != XKB_MOD_INVALID &&
      (mask & (1 << ctx->input.xkb.mod_indices.alt)))
    modifiers |= LVKW_MODIFIER_ALT;
  if (ctx->input.xkb.mod_indices.super != XKB_MOD_INVALID &&
      (mask & (1 << ctx->input.xkb.mod_indices.super)))
    modifiers |= LVKW_MODIFIER_META;
  if (ctx->input.xkb.mod_indices.caps != XKB_MOD_INVALID &&
      (mask & (1 << ctx->input.xkb.mod_indices.caps)))
    modifiers |= LVKW_MODIFIER_CAPS_LOCK;
  if (ctx->input.xkb.mod_indices.num != XKB_MOD_INVALID &&
      (mask & (1 << ctx->input.xkb.mod_indices.num)))
    modifiers |= LVKW_MODIFIER_NUM_LOCK;

  return modifiers;
}

static uint32_t _dnd_action_to_wl(LVKW_DndAction action) {
  switch (action) {
    case LVKW_DND_ACTION_MOVE:
      return WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    case LVKW_DND_ACTION_LINK:
      return WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    case LVKW_DND_ACTION_NONE:
      return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    case LVKW_DND_ACTION_COPY:
    default:
      return WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
  }
}

static LVKW_DndAction _wl_action_to_dnd(uint32_t action) {
  if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) return LVKW_DND_ACTION_MOVE;
  if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) return LVKW_DND_ACTION_LINK;
  if (action & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) return LVKW_DND_ACTION_COPY;
  return LVKW_DND_ACTION_NONE;
}

static void _dnd_destroy_offer(LVKW_Context_WL *ctx, struct wl_data_offer *offer) {
  if (!offer) return;
  if (ctx->input.clipboard.selection_offer == offer) {
    ctx->input.clipboard.selection_offer = NULL;
  }
  _lvkw_wayland_offer_destroy(ctx, offer);
}

static bool _hex_value(char c, uint8_t *out) {
  if (c >= '0' && c <= '9') {
    *out = (uint8_t)(c - '0');
    return true;
  }
  if (c >= 'a' && c <= 'f') {
    *out = (uint8_t)(10 + (c - 'a'));
    return true;
  }
  if (c >= 'A' && c <= 'F') {
    *out = (uint8_t)(10 + (c - 'A'));
    return true;
  }
  return false;
}

static char *_decode_file_uri_path(LVKW_Context_WL *ctx, const char *uri) {
  if (strncmp(uri, "file://", 7) != 0) return NULL;

  const char *path = uri + 7;
  if (strncmp(path, "localhost/", 10) == 0) {
    path += 9;
  }

  size_t in_len = strlen(path);
  char *out = lvkw_context_alloc(&ctx->base, in_len + 1);
  if (!out) return NULL;

  size_t o = 0;
  for (size_t i = 0; i < in_len; i++) {
    if (path[i] == '%' && (i + 2) < in_len) {
      uint8_t hi = 0;
      uint8_t lo = 0;
      if (_hex_value(path[i + 1], &hi) && _hex_value(path[i + 2], &lo)) {
        out[o++] = (char)((hi << 4) | lo);
        i += 2;
        continue;
      }
    }
    out[o++] = path[i];
  }
  out[o] = '\0';
  return out;
}

static LVKW_WaylandDndPayload *_build_dnd_payload(LVKW_Context_WL *ctx, struct wl_data_offer *offer) {
  char *uri_list = NULL;
  size_t uri_list_size = 0;
  if (!_lvkw_wayland_read_data_offer(ctx, offer, "text/uri-list", (void **)&uri_list,
                                     &uri_list_size, true))
    return NULL;

  char **paths = NULL;
  uint16_t path_count = 0;
  uint16_t path_capacity = 0;

  for (char *line = uri_list, *next = NULL; line && *line; line = next) {
    next = strchr(line, '\n');
    if (next) {
      *next = '\0';
      next++;
    }

    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) {
      line[--len] = '\0';
    }

    if (len == 0 || line[0] == '#') continue;

    char *decoded = _decode_file_uri_path(ctx, line);
    if (!decoded) continue;

    if (path_count == path_capacity) {
      uint16_t new_capacity = path_capacity == 0 ? 4 : (uint16_t)(path_capacity * 2);
      char **new_paths = lvkw_context_realloc(&ctx->base, paths, sizeof(char *) * path_capacity,
                                              sizeof(char *) * new_capacity);
      if (!new_paths) {
        lvkw_context_free(&ctx->base, decoded);
        break;
      }
      paths = new_paths;
      path_capacity = new_capacity;
    }

    paths[path_count++] = decoded;
  }

  lvkw_context_free(&ctx->base, uri_list);

  if (path_count == 0) {
    if (paths) lvkw_context_free(&ctx->base, paths);
    return NULL;
  }

  LVKW_WaylandDndPayload *payload = lvkw_context_alloc(&ctx->base, sizeof(LVKW_WaylandDndPayload));
  if (!payload) {
    for (uint16_t i = 0; i < path_count; i++) lvkw_context_free(&ctx->base, paths[i]);
    lvkw_context_free(&ctx->base, paths);
    return NULL;
  }

  payload->paths = (const char **)paths;
  payload->path_count = path_count;
  payload->next = ctx->dnd_payloads;
  ctx->dnd_payloads = payload;

  return payload;
}

static void _emit_dnd_hover(LVKW_Context_WL *ctx, LVKW_Window_WL *window, bool entered) {
  LVKW_Window_Base *base = &window->base;
  if (entered) {
    base->prv.session_userdata = NULL;
    base->prv.current_action = LVKW_DND_ACTION_COPY;
  }
  base->prv.dnd_feedback.action = &base->prv.current_action;
  base->prv.dnd_feedback.session_userdata = &base->prv.session_userdata;

  LVKW_Event evt = {0};
  evt.dnd_hover.position = ctx->input.dnd.position;
  evt.dnd_hover.feedback = &base->prv.dnd_feedback;
  evt.dnd_hover.paths = ctx->input.dnd.payload ? ctx->input.dnd.payload->paths : NULL;
  evt.dnd_hover.path_count = ctx->input.dnd.payload ? ctx->input.dnd.payload->path_count : 0;
  evt.dnd_hover.modifiers = _current_modifiers(ctx);
  evt.dnd_hover.entered = entered;
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_DND_HOVER,
                        (LVKW_Window *)window, &evt);
}

static void _emit_dnd_leave(LVKW_Context_WL *ctx, LVKW_Window_WL *window) {
  LVKW_Event evt = {0};
  evt.dnd_leave.session_userdata = &window->base.prv.session_userdata;
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_DND_LEAVE,
                        (LVKW_Window *)window, &evt);
}

static void _emit_dnd_drop(LVKW_Context_WL *ctx, LVKW_Window_WL *window) {
  LVKW_Event evt = {0};
  evt.dnd_drop.position = ctx->input.dnd.position;
  evt.dnd_drop.session_userdata = &window->base.prv.session_userdata;
  evt.dnd_drop.paths = ctx->input.dnd.payload ? ctx->input.dnd.payload->paths : NULL;
  evt.dnd_drop.path_count = ctx->input.dnd.payload ? ctx->input.dnd.payload->path_count : 0;
  evt.dnd_drop.modifiers = _current_modifiers(ctx);
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_DND_DROP,
                        (LVKW_Window *)window, &evt);
}

static void _data_offer_handle_offer(void *data, struct wl_data_offer *offer, const char *mime_type) {
  LVKW_WaylandDataOffer *meta = (LVKW_WaylandDataOffer *)data;
  if (!meta || !meta->ctx) return;
  LVKW_Context_WL *ctx = meta->ctx;
  if (meta && mime_type) {
    _lvkw_wayland_offer_meta_append_mime(ctx, offer, mime_type);
  }
  if (meta && mime_type && strcmp(mime_type, "text/uri-list") == 0) {
    meta->has_uri_list = true;
  }
}

static void _data_offer_handle_source_actions(void *data, struct wl_data_offer *offer,
                                              uint32_t source_actions) {
  LVKW_WaylandDataOffer *meta = (LVKW_WaylandDataOffer *)data;
  if (!meta) return;
  meta->source_actions = source_actions;
  (void)offer;
}

static void _data_offer_handle_action(void *data, struct wl_data_offer *offer, uint32_t dnd_action) {
  LVKW_WaylandDataOffer *meta = (LVKW_WaylandDataOffer *)data;
  if (!meta) return;
  meta->selected_action = dnd_action;
  (void)offer;
}

static const struct wl_data_offer_listener _data_offer_listener = {
    .offer = _data_offer_handle_offer,
    .source_actions = _data_offer_handle_source_actions,
    .action = _data_offer_handle_action,
};

static void _data_device_handle_data_offer(void *data, struct wl_data_device *data_device,
                                           struct wl_data_offer *offer) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (!_lvkw_wayland_offer_meta_attach(ctx, offer)) return;
  LVKW_WaylandDataOffer *meta = _lvkw_wayland_offer_meta_get(ctx, offer);
  if (!meta) return;
  lvkw_wl_data_offer_add_listener(ctx, offer, &_data_offer_listener, meta);
}

static void _data_device_handle_enter(void *data, struct wl_data_device *data_device, uint32_t serial,
                                      struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y,
                                      struct wl_data_offer *offer) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = _surface_to_live_window(ctx, surface);

  if (ctx->input.dnd.offer && ctx->input.dnd.offer != offer) {
    _dnd_destroy_offer(ctx, ctx->input.dnd.offer);
  }

  ctx->input.dnd.offer = offer;
  ctx->input.dnd.window = window;
  ctx->input.dnd.serial = serial;
  ctx->input.dnd.position.x = (LVKW_Scalar)wl_fixed_to_scalar(x);
  ctx->input.dnd.position.y = (LVKW_Scalar)wl_fixed_to_scalar(y);
  ctx->input.dnd.payload = NULL;

  if (!window || !offer) return;
  if (!window->accept_dnd) {
    lvkw_wl_data_offer_accept(ctx, offer, serial, NULL);
    ctx->input.dnd.window = NULL;
    return;
  }

  LVKW_WaylandDataOffer *meta = _lvkw_wayland_offer_meta_get(ctx, offer);
  if (!meta || !meta->has_uri_list) {
    lvkw_wl_data_offer_accept(ctx, offer, serial, NULL);
    return;
  }

  ctx->input.dnd.payload = _build_dnd_payload(ctx, offer);
  if (!ctx->input.dnd.payload) {
    lvkw_wl_data_offer_accept(ctx, offer, serial, NULL);
    return;
  }

  uint32_t source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
                            WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE |
                            WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
  if (meta->source_actions != 0) {
    source_actions = meta->source_actions;
  }

  uint32_t preferred_action = _dnd_action_to_wl(window->base.prv.current_action);
  if ((source_actions & preferred_action) == 0) {
    if (source_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    }
    else if (source_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    }
    else if (source_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    }
    else {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    }

    window->base.prv.current_action = _wl_action_to_dnd(preferred_action);
  }

  lvkw_wl_data_offer_accept(ctx, offer, serial, "text/uri-list");
  lvkw_wl_data_offer_set_actions(ctx, offer, source_actions, preferred_action);

  _emit_dnd_hover(ctx, window, true);
}

static void _data_device_handle_leave(void *data, struct wl_data_device *data_device) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (ctx->input.dnd.window) {
    _emit_dnd_leave(ctx, ctx->input.dnd.window);
  }
  _dnd_destroy_offer(ctx, ctx->input.dnd.offer);
  memset(&ctx->input.dnd, 0, sizeof(ctx->input.dnd));
}

static void _data_device_handle_motion(void *data, struct wl_data_device *data_device,
                                       uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (!ctx->input.dnd.window || !ctx->input.dnd.offer) return;
  if (!ctx->input.dnd.window->accept_dnd) {
    lvkw_wl_data_offer_accept(ctx, ctx->input.dnd.offer, ctx->input.dnd.serial, NULL);
    return;
  }

  ctx->input.dnd.position.x = (LVKW_Scalar)wl_fixed_to_scalar(x);
  ctx->input.dnd.position.y = (LVKW_Scalar)wl_fixed_to_scalar(y);

  LVKW_WaylandDataOffer *meta = _lvkw_wayland_offer_meta_get(ctx, ctx->input.dnd.offer);
  uint32_t source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
                            WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE |
                            WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
  if (meta && meta->source_actions != 0) {
    source_actions = meta->source_actions;
  }

  uint32_t preferred_action = _dnd_action_to_wl(ctx->input.dnd.window->base.prv.current_action);
  if ((source_actions & preferred_action) == 0) {
    if (source_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    }
    else if (source_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    }
    else if (source_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    }
    else {
      preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    }
    ctx->input.dnd.window->base.prv.current_action = _wl_action_to_dnd(preferred_action);
  }

  if (meta && meta->selected_action != 0) {
    ctx->input.dnd.window->base.prv.current_action = _wl_action_to_dnd(meta->selected_action);
    preferred_action = _dnd_action_to_wl(ctx->input.dnd.window->base.prv.current_action);
  }

  lvkw_wl_data_offer_set_actions(
      ctx, ctx->input.dnd.offer, source_actions, preferred_action);

  _emit_dnd_hover(ctx, ctx->input.dnd.window, false);
}

static void _data_device_handle_drop(void *data, struct wl_data_device *data_device) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  bool should_finish = false;
  if (ctx->input.dnd.offer && ctx->input.dnd.window && ctx->input.dnd.window->accept_dnd &&
      ctx->input.dnd.payload &&
      ctx->input.dnd.window->base.prv.current_action != LVKW_DND_ACTION_NONE) {
    should_finish = true;
  }

  if (ctx->input.dnd.window && ctx->input.dnd.window->accept_dnd) {
    _emit_dnd_drop(ctx, ctx->input.dnd.window);
  }
  if (should_finish) {
    lvkw_wl_data_offer_finish(ctx, ctx->input.dnd.offer);
  }
  _dnd_destroy_offer(ctx, ctx->input.dnd.offer);
  memset(&ctx->input.dnd, 0, sizeof(ctx->input.dnd));
  (void)data_device;
}

static void _data_device_handle_selection(void *data, struct wl_data_device *data_device,
                                          struct wl_data_offer *offer) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  (void)data_device;

  if (ctx->input.clipboard.selection_offer &&
      ctx->input.clipboard.selection_offer != offer &&
      ctx->input.clipboard.selection_offer != ctx->input.dnd.offer) {
    _lvkw_wayland_offer_destroy(ctx, ctx->input.clipboard.selection_offer);
  }

  ctx->input.clipboard.selection_offer = offer;
}

static const struct wl_data_device_listener _data_device_listener = {
    .data_offer = _data_device_handle_data_offer,
    .enter = _data_device_handle_enter,
    .leave = _data_device_handle_leave,
    .motion = _data_device_handle_motion,
    .drop = _data_device_handle_drop,
    .selection = _data_device_handle_selection,
};

/* wl_pointer */

static const char *_cursor_shape_to_name(LVKW_CursorShape shape) {
  switch (shape) {
    case LVKW_CURSOR_SHAPE_DEFAULT:
      return "left_ptr";
    case LVKW_CURSOR_SHAPE_HELP:
      return "help";
    case LVKW_CURSOR_SHAPE_HAND:
      return "hand2";
    case LVKW_CURSOR_SHAPE_WAIT:
      return "watch";
    case LVKW_CURSOR_SHAPE_CROSSHAIR:
      return "crosshair";
    case LVKW_CURSOR_SHAPE_TEXT:
      return "xterm";
    case LVKW_CURSOR_SHAPE_MOVE:
      return "move";
    case LVKW_CURSOR_SHAPE_NOT_ALLOWED:
      return "not-allowed";
    case LVKW_CURSOR_SHAPE_EW_RESIZE:
      return "ew-resize";
    case LVKW_CURSOR_SHAPE_NS_RESIZE:
      return "ns-resize";
    case LVKW_CURSOR_SHAPE_NESW_RESIZE:
      return "nesw-resize";
    case LVKW_CURSOR_SHAPE_NWSE_RESIZE:
      return "nwse-resize";
    default:
      return "left_ptr";
  }
}

static uint32_t _cursor_shape_to_wp(LVKW_CursorShape shape) {
  switch (shape) {
    case LVKW_CURSOR_SHAPE_DEFAULT:
      return 1;  // default
    case LVKW_CURSOR_SHAPE_HELP:
      return 3;  // help
    case LVKW_CURSOR_SHAPE_HAND:
      return 4;  // pointer
    case LVKW_CURSOR_SHAPE_WAIT:
      return 6;  // wait
    case LVKW_CURSOR_SHAPE_CROSSHAIR:
      return 8;  // crosshair
    case LVKW_CURSOR_SHAPE_TEXT:
      return 9;  // text
    case LVKW_CURSOR_SHAPE_MOVE:
      return 13;  // move
    case LVKW_CURSOR_SHAPE_NOT_ALLOWED:
      return 15;  // not-allowed
    case LVKW_CURSOR_SHAPE_EW_RESIZE:
      return 26;  // ew-resize
    case LVKW_CURSOR_SHAPE_NS_RESIZE:
      return 27;  // ns-resize
    case LVKW_CURSOR_SHAPE_NESW_RESIZE:
      return 28;  // nesw-resize
    case LVKW_CURSOR_SHAPE_NWSE_RESIZE:
      return 29;  // nwse-resize
    default:
      return 1;
  }
}

void _lvkw_wayland_update_cursor(LVKW_Context_WL *ctx, LVKW_Window_WL *window, uint32_t serial) {
  if (!ctx->input.pointer || ctx->input.pointer_focus != window) return;

  if (window->cursor_mode == LVKW_CURSOR_LOCKED ||
      window->cursor_mode == LVKW_CURSOR_HIDDEN) {
    lvkw_wl_pointer_set_cursor(ctx, ctx->input.pointer, serial, NULL, 0, 0);
    return;
  }

  LVKW_CursorShape shape = LVKW_CURSOR_SHAPE_DEFAULT;
  LVKW_Cursor_WL *cursor_wl = (LVKW_Cursor_WL *)window->cursor;

  if (cursor_wl) {
    if (cursor_wl->base.pub.flags & LVKW_CURSOR_FLAG_SYSTEM) {
      shape = cursor_wl->shape;
    }
    else {
      lvkw_wl_pointer_set_cursor(ctx, ctx->input.pointer, serial, ctx->wl.cursor_surface,
                            cursor_wl->hotspot_x, cursor_wl->hotspot_y);
      lvkw_wl_surface_attach(ctx, ctx->wl.cursor_surface, cursor_wl->buffer, 0, 0);
      lvkw_wl_surface_damage(ctx, ctx->wl.cursor_surface, 0, 0, cursor_wl->width, cursor_wl->height);
      lvkw_wl_surface_commit(ctx, ctx->wl.cursor_surface);
      return;
    }
  }

  if (ctx->input.cursor_shape_device) {
    lvkw_wp_cursor_shape_device_v1_set_shape(ctx, ctx->input.cursor_shape_device, serial,
                                        _cursor_shape_to_wp(shape));
  }
  else {
    const char *name = _cursor_shape_to_name(shape);
    struct wl_cursor *cursor = lvkw_wl_cursor_theme_get_cursor(ctx, ctx->wl.cursor_theme, name);
    if (!cursor && strcmp(name, "left_ptr") != 0) {
      cursor = lvkw_wl_cursor_theme_get_cursor(ctx, ctx->wl.cursor_theme, "left_ptr");
    }

    if (cursor) {
      struct wl_cursor_image *image = cursor->images[0];
      struct wl_buffer *buffer = lvkw_wl_cursor_image_get_buffer(ctx, image);
      if (buffer) {
        lvkw_wl_pointer_set_cursor(ctx, ctx->input.pointer, serial, ctx->wl.cursor_surface,
                              (int32_t)image->hotspot_x, (int32_t)image->hotspot_y);
        lvkw_wl_surface_attach(ctx, ctx->wl.cursor_surface, buffer, 0, 0);
        lvkw_wl_surface_damage(ctx, ctx->wl.cursor_surface, 0, 0, (int32_t)image->width,
                          (int32_t)image->height);
        lvkw_wl_surface_commit(ctx, ctx->wl.cursor_surface);
      }
    }
  }
}

static void _pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                                  struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (!surface) return;
  ctx->input.pointer_serial = serial;
  ctx->input.clipboard_serial = serial;
  ctx->input.pointer_focus = _surface_to_live_window(ctx, surface);
  if (!ctx->input.pointer_focus) return;

  LVKW_Window_WL *window = ctx->input.pointer_focus;
  window->last_cursor_pos.x = (LVKW_Scalar)wl_fixed_to_scalar(sx);
  window->last_cursor_pos.y = (LVKW_Scalar)wl_fixed_to_scalar(sy);
  window->last_cursor_set = true;

  _lvkw_wayland_update_cursor(ctx, window, serial);
}

static void _pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
                                  struct wl_surface *surface) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (ctx->input.pointer_focus) {
    ctx->input.pointer_focus->last_cursor_set = false;
  }
  ctx->input.pointer_focus = NULL;
}

static LVKW_MouseButton _lvkw_pointer_button_to_lvkw(uint32_t button) {
  switch (button) {
    case BTN_LEFT:
      return LVKW_MOUSE_BUTTON_LEFT;
    case BTN_RIGHT:
      return LVKW_MOUSE_BUTTON_RIGHT;
    case BTN_MIDDLE:
      return LVKW_MOUSE_BUTTON_MIDDLE;
    case BTN_SIDE:
      return LVKW_MOUSE_BUTTON_4;
    case BTN_EXTRA:
      return LVKW_MOUSE_BUTTON_5;
    case BTN_FORWARD:
      return LVKW_MOUSE_BUTTON_6;
    case BTN_BACK:
      return LVKW_MOUSE_BUTTON_7;
    case BTN_TASK:
      return LVKW_MOUSE_BUTTON_8;
    default:
      return (LVKW_MouseButton)0xFFFFFFFF;
  }
}

static void _pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time,
                                   wl_fixed_t sx, wl_fixed_t sy) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = ctx->input.pointer_focus;
  if (!window) return;

  LVKW_Scalar x = (LVKW_Scalar)wl_fixed_to_scalar(sx);
  LVKW_Scalar y = (LVKW_Scalar)wl_fixed_to_scalar(sy);

  ctx->input.pending_pointer.mask |= LVKW_EVENT_TYPE_MOUSE_MOTION;
  LVKW_Event *ev = &ctx->input.pending_pointer.motion;
  memset(ev, 0, sizeof(*ev));
  ev->mouse_motion.position.x = x;
  ev->mouse_motion.position.y = y;

  if (window->last_cursor_set) {
    ev->mouse_motion.delta.x = x - window->last_cursor_pos.x;
    ev->mouse_motion.delta.y = y - window->last_cursor_pos.y;
  }

  window->last_cursor_pos.x = x;
  window->last_cursor_pos.y = y;
  window->last_cursor_set = true;
}

static void _pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial,
                                   uint32_t time, uint32_t button, uint32_t state) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  ctx->input.clipboard_serial = serial;
  LVKW_Window_WL *window = ctx->input.pointer_focus;
  if (!window) return;

  LVKW_MouseButton lvkw_button = _lvkw_pointer_button_to_lvkw(button);
  if (lvkw_button == (LVKW_MouseButton)0xFFFFFFFF) return;

  ctx->input.pending_pointer.mask |= LVKW_EVENT_TYPE_MOUSE_BUTTON;
  LVKW_Event *ev = &ctx->input.pending_pointer.button;
  memset(ev, 0, sizeof(*ev));
  ev->mouse_button.button = lvkw_button;
  ev->mouse_button.state = (state == WL_POINTER_BUTTON_STATE_PRESSED) ? LVKW_BUTTON_STATE_PRESSED
                                                                     : LVKW_BUTTON_STATE_RELEASED;
  ev->mouse_button.modifiers = _current_modifiers(ctx);
}

static void _pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t time,
                                 uint32_t axis, wl_fixed_t value) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = ctx->input.pointer_focus;
  if (!window) return;

  ctx->input.pending_pointer.mask |= LVKW_EVENT_TYPE_MOUSE_SCROLL;
  LVKW_Event *ev = &ctx->input.pending_pointer.scroll;
  if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
    ev->mouse_scroll.delta.x = -wl_fixed_to_scalar(value);
  else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
    ev->mouse_scroll.delta.y = -wl_fixed_to_scalar(value);
}

static void _pointer_handle_frame(void *data, struct wl_pointer *pointer) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = ctx->input.pointer_focus;
  if (!window) {
    ctx->input.pending_pointer.mask = 0;
    return;
  }

  if (ctx->input.pending_pointer.mask & LVKW_EVENT_TYPE_MOUSE_MOTION) {
    lvkw_event_queue_push_compressible(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_MOTION,
                                        (LVKW_Window *)window, &ctx->input.pending_pointer.motion);
  }
  if (ctx->input.pending_pointer.mask & LVKW_EVENT_TYPE_MOUSE_BUTTON) {
    lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_BUTTON,
                          (LVKW_Window *)window, &ctx->input.pending_pointer.button);
  }
  if (ctx->input.pending_pointer.mask & LVKW_EVENT_TYPE_MOUSE_SCROLL) {
    lvkw_event_queue_push_compressible(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_SCROLL,
                                        (LVKW_Window *)window, &ctx->input.pending_pointer.scroll);
    memset(&ctx->input.pending_pointer.scroll, 0, sizeof(ctx->input.pending_pointer.scroll));
  }

  ctx->input.pending_pointer.mask = 0;
}
static void _pointer_handle_axis_source(void *data, struct wl_pointer *pointer,
                                        uint32_t axis_source) {}
static void _pointer_handle_axis_stop(void *data, struct wl_pointer *pointer, uint32_t time,
                                      uint32_t axis) {}
static void _pointer_handle_axis_discrete(void *data, struct wl_pointer *pointer, uint32_t axis,
                                          int32_t discrete) {}

static const struct wl_pointer_listener _pointer_listener = {
    .enter = _pointer_handle_enter,
    .leave = _pointer_handle_leave,
    .motion = _pointer_handle_motion,
    .button = _pointer_handle_button,
    .axis = _pointer_handle_axis,
    .frame = _pointer_handle_frame,
    .axis_source = _pointer_handle_axis_source,
    .axis_stop = _pointer_handle_axis_stop,
    .axis_discrete = _pointer_handle_axis_discrete,
};

/* zwp_relative_pointer_v1 */

static void _relative_pointer_handle_motion(void *data,
                                            struct zwp_relative_pointer_v1 *relative_pointer,
                                            uint32_t time_hi, uint32_t time_lo, wl_fixed_t dx,
                                            wl_fixed_t dy, wl_fixed_t dx_unaccel,
                                            wl_fixed_t dy_unaccel) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_Event evt = {0};
  evt.mouse_motion.position.x = 0;
  evt.mouse_motion.position.y = 0;
  evt.mouse_motion.delta.x = wl_fixed_to_scalar(dx);
  evt.mouse_motion.delta.y = wl_fixed_to_scalar(dy);
  evt.mouse_motion.raw_delta.x = wl_fixed_to_scalar(dx_unaccel);
  evt.mouse_motion.raw_delta.y = wl_fixed_to_scalar(dy_unaccel);
  lvkw_event_queue_push_compressible(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_MOTION,
                                      (LVKW_Window *)window, &evt);
}

const struct zwp_relative_pointer_v1_listener _lvkw_wayland_relative_pointer_listener = {
    .relative_motion = _relative_pointer_handle_motion,
};

/* zwp_locked_pointer_v1 */

static void _locked_pointer_handle_locked(void *data,
                                          struct zwp_locked_pointer_v1 *locked_pointer) {}
static void _locked_pointer_handle_unlocked(void *data,
                                            struct zwp_locked_pointer_v1 *locked_pointer) {}

const struct zwp_locked_pointer_v1_listener _lvkw_wayland_locked_pointer_listener = {
    .locked = _locked_pointer_handle_locked,
    .unlocked = _locked_pointer_handle_unlocked,
};

/* wl_seat */

static void _seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_CTX_ASSUME(data, ctx != NULL,
                  "Context handle must not be NULL in seat capabilities handler");
  LVKW_CTX_ASSUME(data, seat != NULL, "Seat must not be NULL in seat capabilities handler");

  if (!ctx->input.data_device && ctx->protocols.opt.wl_data_device_manager) {
    ctx->input.data_device = lvkw_wl_data_device_manager_get_data_device(
        ctx, ctx->protocols.opt.wl_data_device_manager, seat);
    if (ctx->input.data_device) {
      lvkw_wl_data_device_add_listener(ctx, ctx->input.data_device, &_data_device_listener, ctx);
    }
  }

  if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !ctx->input.keyboard) {
    ctx->input.keyboard = lvkw_wl_seat_get_keyboard(ctx, seat);
    lvkw_wl_keyboard_add_listener(ctx, ctx->input.keyboard, &_keyboard_listener, ctx);
    if (!ctx->input.text_input && ctx->protocols.opt.zwp_text_input_manager_v3) {
      ctx->input.text_input = lvkw_zwp_text_input_manager_v3_get_text_input(
          ctx, ctx->protocols.opt.zwp_text_input_manager_v3, seat);
      if (ctx->input.text_input) {
        lvkw_zwp_text_input_v3_add_listener(ctx, ctx->input.text_input, &_text_input_listener, ctx);
      }
    }
  }

  else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && ctx->input.keyboard) {
    if (ctx->input.text_input) {
      _lvkw_wayland_sync_text_input_state(ctx, NULL);
      lvkw_zwp_text_input_v3_destroy(ctx, ctx->input.text_input);
      ctx->input.text_input = NULL;
    }
    lvkw_wl_keyboard_destroy(ctx, ctx->input.keyboard);
    ctx->input.keyboard = NULL;
    ctx->input.keyboard_focus = NULL;
  }

  if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !ctx->input.pointer) {
    ctx->input.pointer = lvkw_wl_seat_get_pointer(ctx, seat);
    lvkw_wl_pointer_add_listener(ctx, ctx->input.pointer, &_pointer_listener, ctx);

    if (ctx->protocols.opt.wp_cursor_shape_manager_v1) {
      ctx->input.cursor_shape_device = lvkw_wp_cursor_shape_manager_v1_get_pointer(ctx, 
          ctx->protocols.opt.wp_cursor_shape_manager_v1, ctx->input.pointer);
    }

    LVKW_Window_Base *curr = ctx->base.prv.window_list;
    while (curr) {
      LVKW_Window_WL *window = (LVKW_Window_WL *)curr;
      if (window->cursor_mode == LVKW_CURSOR_LOCKED &&
          ctx->protocols.opt.zwp_pointer_constraints_v1 &&
          ctx->protocols.opt.zwp_relative_pointer_manager_v1 && !window->input.locked) {
        window->input.locked = lvkw_zwp_pointer_constraints_v1_lock_pointer(
            ctx, ctx->protocols.opt.zwp_pointer_constraints_v1, window->wl.surface,
            ctx->input.pointer, NULL, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        if (window->input.locked) {
          lvkw_zwp_locked_pointer_v1_add_listener(ctx, window->input.locked,
                                                  &_lvkw_wayland_locked_pointer_listener, window);
        }

        window->input.relative = lvkw_zwp_relative_pointer_manager_v1_get_relative_pointer(
            ctx, ctx->protocols.opt.zwp_relative_pointer_manager_v1, ctx->input.pointer);
        if (window->input.relative) {
          lvkw_zwp_relative_pointer_v1_add_listener(ctx, window->input.relative,
                                                    &_lvkw_wayland_relative_pointer_listener,
                                                    window);
        }
      }
      curr = curr->prv.next;
    }
  }
  else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && ctx->input.pointer) {
    LVKW_Window_Base *curr = ctx->base.prv.window_list;
    while (curr) {
      LVKW_Window_WL *window = (LVKW_Window_WL *)curr;
      if (window->input.relative) {
        lvkw_zwp_relative_pointer_v1_destroy(ctx, window->input.relative);
        window->input.relative = NULL;
      }
      if (window->input.locked) {
        lvkw_zwp_locked_pointer_v1_destroy(ctx, window->input.locked);
        window->input.locked = NULL;
      }
      curr = curr->prv.next;
    }

    if (ctx->input.cursor_shape_device) {
      lvkw_wp_cursor_shape_device_v1_destroy(ctx, ctx->input.cursor_shape_device);
      ctx->input.cursor_shape_device = NULL;
    }
    lvkw_wl_pointer_destroy(ctx, ctx->input.pointer);
    ctx->input.pointer = NULL;
  }

  if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) &&
      !(capabilities & WL_SEAT_CAPABILITY_POINTER) && ctx->input.data_device) {
    _dnd_destroy_offer(ctx, ctx->input.dnd.offer);
    memset(&ctx->input.dnd, 0, sizeof(ctx->input.dnd));
    lvkw_wl_data_device_destroy(ctx, ctx->input.data_device);
    ctx->input.data_device = NULL;
  }
}

static void _seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {}

const struct wl_seat_listener _lvkw_wayland_seat_listener = {
    .capabilities = _seat_handle_capabilities,
    .name = _seat_handle_name,
};

LVKW_Status lvkw_ctx_getStandardCursor_WL(LVKW_Context *ctx_handle, LVKW_CursorShape shape,
                                          LVKW_Cursor **out_cursor) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;
  *out_cursor = (LVKW_Cursor *)&ctx->input.standard_cursors[shape];
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_createCursor_WL(LVKW_Context *ctx_handle,
                                     const LVKW_CursorCreateInfo *create_info,
                                     LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx_handle, create_info, out_cursor);

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;
  LVKW_Cursor_WL *cursor = lvkw_context_alloc(&ctx->base, sizeof(LVKW_Cursor_WL));

  if (!cursor) return LVKW_ERROR;

  cursor->base.pub.flags = 0;
  cursor->base.prv.ctx_base = &ctx->base;
#ifdef LVKW_INDIRECT_BACKEND
  cursor->base.prv.backend = ctx->base.prv.backend;
#endif
  cursor->shape = (LVKW_CursorShape)0;
  cursor->width = (int32_t)create_info->size.x;
  cursor->height = (int32_t)create_info->size.y;
  cursor->hotspot_x = (int32_t)create_info->hotSpot.x;
  cursor->hotspot_y = (int32_t)create_info->hotSpot.y;

  size_t size = (size_t)(cursor->width * cursor->height * 4);

  // Use memfd_create for shared memory
  int fd = memfd_create("lvkw-cursor", MFD_CLOEXEC);
  if (fd < 0) {
    lvkw_context_free(&ctx->base, cursor);
    return LVKW_ERROR;
  }

  if (ftruncate(fd, (off_t)size) < 0) {
    close(fd);
    lvkw_context_free(&ctx->base, cursor);
    return LVKW_ERROR;
  }

  uint32_t *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close(fd);
    lvkw_context_free(&ctx->base, cursor);
    return LVKW_ERROR;
  }

  // Swizzle from RGBA to ARGB (Wayland's preferred format)
  // LVKW: R, G, B, A in memory (on little-endian, uint32_t is 0xAABBGGRR)
  // Wayland ARGB8888: B, G, R, A in memory (on little-endian, uint32_t is 0xAARRGGBB)
  for (int i = 0; i < cursor->width * cursor->height; ++i) {
    uint32_t rgba = create_info->pixels[i];
    uint32_t r = (rgba >> 0) & 0xFF;
    uint32_t g = (rgba >> 8) & 0xFF;
    uint32_t b = (rgba >> 16) & 0xFF;
    uint32_t a = (rgba >> 24) & 0xFF;
    data[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }

  munmap(data, size);

  struct wl_shm_pool *pool = lvkw_wl_shm_create_pool(ctx, ctx->protocols.wl_shm, fd, (int32_t)size);
  cursor->buffer =
      lvkw_wl_shm_pool_create_buffer(ctx, pool, 0, cursor->width, cursor->height, cursor->width * 4,
                                     WL_SHM_FORMAT_ARGB8888);

  lvkw_wl_shm_pool_destroy(ctx, pool);
  close(fd);

  if (!cursor->buffer) {
    lvkw_context_free(&ctx->base, cursor);
    return LVKW_ERROR;
  }

  *out_cursor = (LVKW_Cursor *)cursor;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_cursor_destroy_WL(LVKW_Cursor *cursor_handle) {
  LVKW_API_VALIDATE(cursor_destroy, cursor_handle);

  if (cursor_handle->flags & LVKW_CURSOR_FLAG_SYSTEM) return LVKW_SUCCESS;

  LVKW_Cursor_WL *cursor = (LVKW_Cursor_WL *)cursor_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)cursor->base.prv.ctx_base;

  if (cursor->buffer) {
    lvkw_wl_buffer_destroy(ctx, cursor->buffer);
  }

  lvkw_context_free(&ctx->base, cursor);
  return LVKW_SUCCESS;
}
