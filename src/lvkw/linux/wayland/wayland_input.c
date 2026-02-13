// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#define _GNU_SOURCE
#include <linux/input-event-codes.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "dlib/wayland-cursor.h"
#include "lvkw_api_constraints.h"
#include "lvkw_assume.h"
#include "lvkw_linux_internal.h"
#include "lvkw_mem_internal.h"
#include "lvkw_wayland_internal.h"

/* wl_keyboard */

static void _keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size) {
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

  struct xkb_keymap *keymap =
      xkb_keymap_new_from_string(ctx->input.xkb.ctx, map_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_str, size);
  close(fd);

  if (!keymap) {
    LVKW_REPORT_CTX_DIAGNOSTIC(data, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "Failed to compile keymap");
    return;
  }

  struct xkb_state *state = xkb_state_new(keymap);
  if (!state) {
    LVKW_REPORT_CTX_DIAGNOSTIC(data, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "Failed to create xkb state");
    xkb_keymap_unref(keymap);
    return;
  }

  if (ctx->input.xkb.keymap) xkb_keymap_unref(ctx->input.xkb.keymap);
  if (ctx->input.xkb.state) xkb_state_unref(ctx->input.xkb.state);

  ctx->input.xkb.keymap = keymap;
  ctx->input.xkb.state = state;

  ctx->input.xkb.mod_indices.shift = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
  ctx->input.xkb.mod_indices.ctrl = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
  ctx->input.xkb.mod_indices.alt = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
  ctx->input.xkb.mod_indices.super = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_LOGO);
  ctx->input.xkb.mod_indices.caps = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CAPS);
  ctx->input.xkb.mod_indices.num = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_NUM);
}

static void _keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                   struct wl_surface *surface, struct wl_array *keys) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(data, ctx != NULL, "Context handle must not be NULL in keyboard enter handler");
  LVKW_CTX_ASSUME(data, keyboard != NULL, "Keyboard must not be NULL in keyboard enter handler");
  LVKW_CTX_ASSUME(data, surface != NULL, "Surface must not be NULL in keyboard enter handler");

  ctx->input.keyboard_focus = wl_surface_get_user_data(surface);
  LVKW_CTX_ASSUME(&ctx->base, ctx->input.keyboard_focus != NULL,
                  "Keyboard focus surface must have associated window user data");
}

static void _keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                   struct wl_surface *surface) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(&ctx->base, ctx != NULL, "Context handle must not be NULL in keyboard leave handler");
  LVKW_CTX_ASSUME(&ctx->base, keyboard != NULL, "Keyboard must not be NULL in keyboard leave handler");

  ctx->input.keyboard_focus = NULL;
}

static void _keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key,
                                 uint32_t state) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(&ctx->base, ctx != NULL, "Context handle must not be NULL in key handler");
  LVKW_CTX_ASSUME(&ctx->base, keyboard != NULL, "Keyboard must not be NULL in key handler");

  if (!ctx->input.keyboard_focus) return;

  uint32_t keysym = XKB_KEY_NoSymbol;
  uint32_t modifiers = 0;
  if (ctx->input.xkb.state) {
    keysym = xkb_state_key_get_one_sym(ctx->input.xkb.state, key + 8);

    xkb_mod_mask_t mask = xkb_state_serialize_mods(ctx->input.xkb.state, XKB_STATE_MODS_EFFECTIVE);

    if (ctx->input.xkb.mod_indices.shift != XKB_MOD_INVALID && (mask & (1 << ctx->input.xkb.mod_indices.shift)))
      modifiers |= LVKW_MODIFIER_SHIFT;
    if (ctx->input.xkb.mod_indices.ctrl != XKB_MOD_INVALID && (mask & (1 << ctx->input.xkb.mod_indices.ctrl)))
      modifiers |= LVKW_MODIFIER_CONTROL;
    if (ctx->input.xkb.mod_indices.alt != XKB_MOD_INVALID && (mask & (1 << ctx->input.xkb.mod_indices.alt)))
      modifiers |= LVKW_MODIFIER_ALT;
    if (ctx->input.xkb.mod_indices.super != XKB_MOD_INVALID && (mask & (1 << ctx->input.xkb.mod_indices.super)))
      modifiers |= LVKW_MODIFIER_SUPER;
    if (ctx->input.xkb.mod_indices.caps != XKB_MOD_INVALID && (mask & (1 << ctx->input.xkb.mod_indices.caps)))
      modifiers |= LVKW_MODIFIER_CAPS_LOCK;
    if (ctx->input.xkb.mod_indices.num != XKB_MOD_INVALID && (mask & (1 << ctx->input.xkb.mod_indices.num)))
      modifiers |= LVKW_MODIFIER_NUM_LOCK;
  }

  LVKW_Event evt = {0};

  evt.key.key = lvkw_linux_translate_keysym(keysym);
  evt.key.state = (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
  evt.key.modifiers = modifiers;

  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_KEY, ctx->input.keyboard_focus, &evt);
}

static void _keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial,
                                       uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
                                       uint32_t group) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_CTX_ASSUME(&ctx->base, ctx != NULL, "Context handle must not be NULL in modifiers handler");

  if (ctx->input.xkb.state) {
    xkb_state_update_mask(ctx->input.xkb.state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
  }
}

static void _keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard, int32_t rate, int32_t delay) {
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

/* wl_pointer */

static const char *_cursor_shape_to_name(LVKW_CursorShape shape) {
  switch (shape) {
    case LVKW_CURSOR_SHAPE_DEFAULT:
      return "left_ptr";
    case LVKW_CURSOR_SHAPE_HELP:
      return "help";
    case LVKW_CURSOR_SHAPE_POINTER:
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
      return 1; // default
    case LVKW_CURSOR_SHAPE_HELP:
      return 3; // help
    case LVKW_CURSOR_SHAPE_POINTER:
      return 4; // pointer
    case LVKW_CURSOR_SHAPE_WAIT:
      return 6; // wait
    case LVKW_CURSOR_SHAPE_CROSSHAIR:
      return 8; // crosshair
    case LVKW_CURSOR_SHAPE_TEXT:
      return 9; // text
    case LVKW_CURSOR_SHAPE_MOVE:
      return 13; // move
    case LVKW_CURSOR_SHAPE_NOT_ALLOWED:
      return 15; // not-allowed
    case LVKW_CURSOR_SHAPE_EW_RESIZE:
      return 26; // ew-resize
    case LVKW_CURSOR_SHAPE_NS_RESIZE:
      return 27; // ns-resize
    case LVKW_CURSOR_SHAPE_NESW_RESIZE:
      return 28; // nesw-resize
    case LVKW_CURSOR_SHAPE_NWSE_RESIZE:
      return 29; // nwse-resize
    default:
      return 1;
  }
}

void _lvkw_wayland_update_cursor(LVKW_Context_WL *ctx, LVKW_Window_WL *window, uint32_t serial) {
  if (!ctx->input.pointer || ctx->input.pointer_focus != window) return;

  if (window->cursor_mode == LVKW_CURSOR_LOCKED) {
    wl_pointer_set_cursor(ctx->input.pointer, serial, NULL, 0, 0);
    return;
  }

  LVKW_CursorShape shape = LVKW_CURSOR_SHAPE_DEFAULT;
  LVKW_Cursor_WL *cursor_wl = (LVKW_Cursor_WL *)window->cursor;

  if (cursor_wl) {
    if (cursor_wl->base.pub.flags & LVKW_CURSOR_FLAG_SYSTEM) {
      shape = cursor_wl->shape;
    }
    else {
      wl_pointer_set_cursor(ctx->input.pointer, serial, ctx->wl.cursor_surface, cursor_wl->hotspot_x,
                            cursor_wl->hotspot_y);
      wl_surface_attach(ctx->wl.cursor_surface, cursor_wl->buffer, 0, 0);
      wl_surface_damage(ctx->wl.cursor_surface, 0, 0, cursor_wl->width, cursor_wl->height);
      wl_surface_commit(ctx->wl.cursor_surface);
      return;
    }
  }

  if (ctx->input.cursor_shape_device) {
    wp_cursor_shape_device_v1_set_shape(ctx->input.cursor_shape_device, serial, _cursor_shape_to_wp(shape));
  }
  else {
    const char *name = _cursor_shape_to_name(shape);
    struct wl_cursor *cursor = wl_cursor_theme_get_cursor(ctx->wl.cursor_theme, name);
    if (!cursor && strcmp(name, "left_ptr") != 0) {
      cursor = wl_cursor_theme_get_cursor(ctx->wl.cursor_theme, "left_ptr");
    }

    if (cursor) {
      struct wl_cursor_image *image = cursor->images[0];
      struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
      if (buffer) {
        wl_pointer_set_cursor(ctx->input.pointer, serial, ctx->wl.cursor_surface, (int32_t)image->hotspot_x,
                              (int32_t)image->hotspot_y);
        wl_surface_attach(ctx->wl.cursor_surface, buffer, 0, 0);
        wl_surface_damage(ctx->wl.cursor_surface, 0, 0, (int32_t)image->width, (int32_t)image->height);
        wl_surface_commit(ctx->wl.cursor_surface);
      }
    }
  }
}

static void _pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface,
                                  wl_fixed_t sx, wl_fixed_t sy) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  if (!surface) return;
  ctx->input.pointer_serial = serial;
  ctx->input.pointer_focus = wl_surface_get_user_data(surface);
  LVKW_CTX_ASSUME(&ctx->base, ctx->input.pointer_focus != NULL,
                  "Pointer focus surface must have associated window user data");

  _lvkw_wayland_update_cursor(ctx, ctx->input.pointer_focus, serial);
}

static void _pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
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

static void _pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx,
                                   wl_fixed_t sy) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = ctx->input.pointer_focus;
  if (!window) return;

  LVKW_Event ev = {0};
  ev.mouse_motion.position.x = wl_fixed_to_double(sx);
  ev.mouse_motion.position.y = wl_fixed_to_double(sy);
  ev.mouse_motion.delta.x = 0;
  ev.mouse_motion.delta.y = 0;
  ev.mouse_motion.raw_delta.x = 0;
  ev.mouse_motion.raw_delta.y = 0;
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_MOUSE_MOTION, window, &ev);
}

static void _pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time,
                                   uint32_t button, uint32_t state) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = ctx->input.pointer_focus;
  if (!window) return;

  LVKW_MouseButton lvkw_button = _lvkw_pointer_button_to_lvkw(button);
  if (lvkw_button == (LVKW_MouseButton)0xFFFFFFFF) return;

  LVKW_Event ev = {0};
  ev.mouse_button.button = lvkw_button;
  ev.mouse_button.state =
      (state == WL_POINTER_BUTTON_STATE_PRESSED) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_MOUSE_BUTTON, window, &ev);
}

static void _pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis,
                                 wl_fixed_t value) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_Window_WL *window = ctx->input.pointer_focus;
  if (!window) return;

  LVKW_Event ev = {0};
  ev.mouse_scroll.delta.x = 0;
  ev.mouse_scroll.delta.y = 0;
  if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
    ev.mouse_scroll.delta.x = -wl_fixed_to_double(value);
  else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
    ev.mouse_scroll.delta.y = -wl_fixed_to_double(value);
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_MOUSE_SCROLL, window, &ev);
}

static void _pointer_handle_frame(void *data, struct wl_pointer *pointer) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  _lvkw_wayland_flush_event_pool(ctx);
}
static void _pointer_handle_axis_source(void *data, struct wl_pointer *pointer, uint32_t axis_source) {}
static void _pointer_handle_axis_stop(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis) {}
static void _pointer_handle_axis_discrete(void *data, struct wl_pointer *pointer, uint32_t axis, int32_t discrete) {}

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

static void _relative_pointer_handle_motion(void *data, struct zwp_relative_pointer_v1 *relative_pointer,
                                            uint32_t time_hi, uint32_t time_lo, wl_fixed_t dx, wl_fixed_t dy,
                                            wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_Event evt = {0};
  evt.mouse_motion.position.x = 0;
  evt.mouse_motion.position.y = 0;
  evt.mouse_motion.delta.x = wl_fixed_to_double(dx);
  evt.mouse_motion.delta.y = wl_fixed_to_double(dy);
  evt.mouse_motion.raw_delta.x = wl_fixed_to_double(dx_unaccel);
  evt.mouse_motion.raw_delta.y = wl_fixed_to_double(dy_unaccel);
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_MOUSE_MOTION, window,&evt);
}

static const struct zwp_relative_pointer_v1_listener _relative_pointer_listener = {
    .relative_motion = _relative_pointer_handle_motion,
};

/* zwp_locked_pointer_v1 */

static void _locked_pointer_handle_locked(void *data, struct zwp_locked_pointer_v1 *locked_pointer) {}
static void _locked_pointer_handle_unlocked(void *data, struct zwp_locked_pointer_v1 *locked_pointer) {}

static const struct zwp_locked_pointer_v1_listener _locked_pointer_listener = {
    .locked = _locked_pointer_handle_locked,
    .unlocked = _locked_pointer_handle_unlocked,
};

/* wl_seat */

static void _seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_CTX_ASSUME(data, ctx != NULL, "Context handle must not be NULL in seat capabilities handler");
  LVKW_CTX_ASSUME(data, seat != NULL, "Seat must not be NULL in seat capabilities handler");

  if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !ctx->input.keyboard) {
    ctx->input.keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(ctx->input.keyboard, &_keyboard_listener, ctx);
  }

  else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && ctx->input.keyboard) {
    wl_keyboard_destroy(ctx->input.keyboard);
    ctx->input.keyboard = NULL;
  }

  if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !ctx->input.pointer) {
    ctx->input.pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(ctx->input.pointer, &_pointer_listener, ctx);

    if (ctx->protocols.opt.wp_cursor_shape_manager_v1) {
      ctx->input.cursor_shape_device =
          wp_cursor_shape_manager_v1_get_pointer(ctx->protocols.opt.wp_cursor_shape_manager_v1, ctx->input.pointer);
    }
  }
  else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && ctx->input.pointer) {
    if (ctx->input.cursor_shape_device) {
      wp_cursor_shape_device_v1_destroy(ctx->input.cursor_shape_device);
      ctx->input.cursor_shape_device = NULL;
    }
    wl_pointer_destroy(ctx->input.pointer);
    ctx->input.pointer = NULL;
  }
}

static void _seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {}

const struct wl_seat_listener _lvkw_wayland_seat_listener = {
    .capabilities = _seat_handle_capabilities,
    .name = _seat_handle_name,
};

LVKW_Status lvkw_wnd_setCursorMode_WL(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (window->cursor_mode == mode) return LVKW_SUCCESS;

  // Cleanup old mode
  if (window->cursor_mode == LVKW_CURSOR_LOCKED) {
    if (window->input.relative) {
      zwp_relative_pointer_v1_destroy(window->input.relative);
      window->input.relative = NULL;
    }
    if (window->input.locked) {
      zwp_locked_pointer_v1_destroy(window->input.locked);
      window->input.locked = NULL;
    }
  }

  window->cursor_mode = mode;

  if (mode == LVKW_CURSOR_LOCKED) {
    if (ctx->protocols.opt.zwp_pointer_constraints_v1 && ctx->protocols.opt.zwp_relative_pointer_manager_v1 &&
        ctx->input.pointer) {
      window->input.locked = zwp_pointer_constraints_v1_lock_pointer(ctx->protocols.opt.zwp_pointer_constraints_v1,
                                                                     window->wl.surface, ctx->input.pointer, NULL,
                                                                     ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
      zwp_locked_pointer_v1_add_listener(window->input.locked, &_locked_pointer_listener, window);

      window->input.relative = zwp_relative_pointer_manager_v1_get_relative_pointer(
          ctx->protocols.opt.zwp_relative_pointer_manager_v1, ctx->input.pointer);
      zwp_relative_pointer_v1_add_listener(window->input.relative, &_relative_pointer_listener, window);
    }
  }

  if (mode == LVKW_CURSOR_LOCKED && ctx->input.pointer_focus == window && ctx->input.pointer) {
    wl_pointer_set_cursor(ctx->input.pointer, ctx->input.pointer_serial, NULL, 0, 0);
  }
  else if (mode == LVKW_CURSOR_NORMAL && ctx->input.pointer_focus == window && ctx->input.pointer) {
    _lvkw_wayland_update_cursor(ctx, window, ctx->input.pointer_serial);
  }

    _lvkw_wayland_check_error(ctx);

    if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  

      return LVKW_SUCCESS;

  

    }

  

    

  

LVKW_Cursor *lvkw_ctx_getStandardCursor_WL(LVKW_Context *ctx_handle, LVKW_CursorShape shape) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;
  if (shape < 1 || shape > 12) return NULL;
  return (LVKW_Cursor *)&ctx->input.standard_cursors[shape];
}

  

    

  

    LVKW_Status lvkw_ctx_createCursor_WL(LVKW_Context *ctx_handle, const LVKW_CursorCreateInfo *create_info,

  

    

  

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

  

    

  

    

  

    

  

      struct wl_shm_pool *pool = wl_shm_create_pool(ctx->protocols.wl_shm, fd, (int32_t)size);

  

    

  

      cursor->buffer =

  

    

  

          wl_shm_pool_create_buffer(pool, 0, cursor->width, cursor->height, cursor->width * 4, WL_SHM_FORMAT_ARGB8888);

  

    

  

      wl_shm_pool_destroy(pool);

  

    

  

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

  

    

  

        wl_buffer_destroy(cursor->buffer);

  

    

  

      }

  

    

  

    

  

    

  

      lvkw_context_free(&ctx->base, cursor);

  

    

  

      return LVKW_SUCCESS;

  

    

  

    }

  

    

  