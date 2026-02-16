// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>

#include "dlib/X11.h"
#include "dlib/Xi.h"
#include "dlib/Xrandr.h"
#include "dlib/Xss.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "controller/lvkw_controller_internal.h"
#endif

LVKW_Status _lvkw_wnd_setCursor_X11(LVKW_Window *window_handle, LVKW_Cursor *cursor);

static LVKW_ModifierFlags _lvkw_x11_get_modifiers(unsigned int state) {
  LVKW_ModifierFlags mods = 0;
  if (state & ShiftMask) mods |= LVKW_MODIFIER_SHIFT;
  if (state & ControlMask) mods |= LVKW_MODIFIER_CONTROL;
  if (state & Mod1Mask) mods |= LVKW_MODIFIER_ALT;
  if (state & Mod4Mask) mods |= LVKW_MODIFIER_META;
  if (state & LockMask) mods |= LVKW_MODIFIER_CAPS_LOCK;
  // X11 doesn't have a standard NumLock mask in the core protocol (it's usually one of Mod2-Mod5)
  return mods;
}

static LVKW_Key _lvkw_x11_get_key(LVKW_Context_X11 *ctx, XKeyEvent *ev) {
  if (ctx->linux_base.xkb.state) {
    xkb_keysym_t sym = lvkw_xkb_state_key_get_one_sym(ctx, ctx->linux_base.xkb.state, ev->keycode);
    return lvkw_linux_translate_keysym(sym);
  }
  KeySym sym = lvkw_XLookupKeysym(ctx, ev, 0);
  return lvkw_linux_translate_keysym((xkb_keysym_t)sym);
}

static void _lvkw_x11_process_event(LVKW_Context_X11 *ctx, XEvent *xev) {
  LVKW_Window_X11 *window = NULL;
  if (lvkw_XFindContext(ctx, ctx->display, xev->xany.window, ctx->window_context,
                        (XPointer *)&window) != 0) {
    window = NULL;
  }

  switch (xev->type) {
    case KeyPress:
    case KeyRelease: {
      if (!window) break;
      LVKW_Event ev = {0};
      ev.key.key = _lvkw_x11_get_key(ctx, &xev->xkey);
      ev.key.state = (xev->type == KeyPress) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
      ev.key.modifiers = _lvkw_x11_get_modifiers(xev->xkey.state);
      lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_KEY,
                            (LVKW_Window *)window, &ev);
      break;
    }

    case ButtonPress:
    case ButtonRelease: {
      if (!window) break;
      LVKW_Event ev = {0};
      ev.mouse_button.button = _lvkw_x11_translate_button(xev->xbutton.button);
      
      // Handle scroll wheel events (buttons 4-7)
      if (xev->xbutton.button >= 4 && xev->xbutton.button <= 7) {
        if (xev->type == ButtonPress) {
            LVKW_Event sev = {0};
            if (xev->xbutton.button == 4) sev.mouse_scroll.delta.y = 1.0f;
            else if (xev->xbutton.button == 5) sev.mouse_scroll.delta.y = -1.0f;
            else if (xev->xbutton.button == 6) sev.mouse_scroll.delta.x = -1.0f;
            else if (xev->xbutton.button == 7) sev.mouse_scroll.delta.x = 1.0f;
            
            lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_SCROLL,
                                               (LVKW_Window *)window, &sev);
        }
        break;
      }
      
      if (ev.mouse_button.button != (LVKW_MouseButton)0xFFFFFFFF) {
          ev.mouse_button.state = (xev->type == ButtonPress) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          ev.mouse_button.modifiers = _lvkw_x11_get_modifiers(xev->xbutton.state);
          lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_BUTTON,
                                (LVKW_Window *)window, &ev);
      }
      break;
    }

    case MotionNotify: {
      if (!window) break;
      LVKW_Event ev = {0};
      ev.mouse_motion.position.x = (LVKW_Scalar)xev->xmotion.x / ctx->scale;
      ev.mouse_motion.position.y = (LVKW_Scalar)xev->xmotion.y / ctx->scale;
      // TODO: delta and raw_delta
      lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_MOTION,
                                         (LVKW_Window *)window, &ev);
      break;
    }

    case EnterNotify: {
      if (!window) break;
      if (window->cursor_mode != LVKW_CURSOR_LOCKED) {
        _lvkw_wnd_setCursor_X11((LVKW_Window *)window, window->cursor);
      }
      break;
    }

    case LeaveNotify: {
      break;
    }

    case ConfigureNotify: {
      if (!window) break;
      uint32_t new_w = (uint32_t)((LVKW_Scalar)xev->xconfigure.width / ctx->scale);
      uint32_t new_h = (uint32_t)((LVKW_Scalar)xev->xconfigure.height / ctx->scale);
      
      if (new_w != window->size.x || new_h != window->size.y) {
          window->size.x = new_w;
          window->size.y = new_h;
          
          LVKW_Event ev = {0};
          ev.resized.geometry.logicalSize = window->size;
          ev.resized.geometry.pixelSize.x = xev->xconfigure.width;
          ev.resized.geometry.pixelSize.y = xev->xconfigure.height;
          
          lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_RESIZED,
                                             (LVKW_Window *)window, &ev);
      }
      break;
    }

    case MapNotify: {
      if (!window) break;
      if (!(window->base.pub.flags & LVKW_WND_STATE_READY)) {
          window->base.pub.flags |= LVKW_WND_STATE_READY;
          LVKW_Event ev = {0};
          lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_READY,
                                (LVKW_Window *)window, &ev);
      }
      break;
    }

    case ClientMessage: {
      if (!window) break;
      if (xev->xclient.message_type == ctx->wm_protocols) {
        Atom protocol = (Atom)xev->xclient.data.l[0];
        if (protocol == ctx->wm_delete_window) {
          LVKW_Event ev = {0};
          lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_CLOSE_REQUESTED,
                                (LVKW_Window *)window, &ev);
        } else if (protocol == ctx->wm_take_focus) {
          lvkw_XSetInputFocus(ctx, ctx->display, window->window, RevertToParent, (Time)xev->xclient.data.l[1]);
        } else if (protocol == ctx->net_wm_ping) {
          XEvent reply = *xev;
          reply.xclient.window = DefaultRootWindow(ctx->display);
          lvkw_XSendEvent(ctx, ctx->display, reply.xclient.window, False,
                          SubstructureNotifyMask | SubstructureRedirectMask, &reply);
        }
      }
      break;
    }

    case FocusIn:
    case FocusOut: {
      if (!window) break;
      bool focused = (xev->type == FocusIn);
      bool old_focused = (window->base.pub.flags & LVKW_WND_STATE_FOCUSED) != 0;
      if (focused != old_focused) {
          if (focused) window->base.pub.flags |= LVKW_WND_STATE_FOCUSED;
          else window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_FOCUSED;
          
          LVKW_Event ev = {0};
          ev.focus.focused = focused;
          lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_FOCUS,
                                (LVKW_Window *)window, &ev);
      }
      break;
    }

    default: {
      if (ctx->randr_available && (xev->type == ctx->randr_event_base + RRScreenChangeNotify ||
                                   xev->type == ctx->randr_event_base + RRNotify)) {
          _lvkw_x11_update_monitors(ctx);
      }
      break;
    }
  }
}

LVKW_Status lvkw_ctx_syncEvents_X11(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  
  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  uint64_t start_time = (timeout_ms != LVKW_NEVER && timeout_ms > 0) ? _lvkw_get_timestamp_ms() : 0;

  for (;;) {
    while (lvkw_XPending(ctx, ctx->display)) {
      XEvent xev;
      lvkw_XNextEvent(ctx, ctx->display, &xev);
      _lvkw_x11_process_event(ctx, &xev);
    }

#ifdef LVKW_ENABLE_CONTROLLER
    _lvkw_ctrl_poll_Linux(&ctx->linux_base.base, &ctx->linux_base.controller);
#endif

    if (ctx->linux_base.base.prv.event_queue.active->count > 0) break;

    int poll_timeout = -1;
    if (timeout_ms != LVKW_NEVER) {
      if (timeout_ms == 0) {
        poll_timeout = 0;
      } else {
        uint64_t now = _lvkw_get_timestamp_ms();
        uint64_t elapsed = now - start_time;
        if (elapsed >= timeout_ms) {
          poll_timeout = 0;
        } else {
          poll_timeout = (int)(timeout_ms - elapsed);
        }
      }
    }

    if (poll_timeout == 0) break;

    struct pollfd pfd;
    pfd.fd = lvkw_XConnectionNumber(ctx, ctx->display);
    pfd.events = POLLIN;
    
    // In a real implementation, we might want to poll other FDs too (controllers, etc.)
    int ret = poll(&pfd, 1, poll_timeout);
    if (ret <= 0) break;
  }

  lvkw_event_queue_begin_gather(&ctx->linux_base.base.prv.event_queue);
  return LVKW_SUCCESS;
}

void _lvkw_x11_push_event_cb(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                             const LVKW_Event *evt) {
  LVKW_Context_X11 *ctx_x11 = (LVKW_Context_X11 *)ctx;
  if (type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
      type == LVKW_EVENT_TYPE_WINDOW_RESIZED) {
    lvkw_event_queue_push_compressible(&ctx_x11->linux_base.base, &ctx_x11->linux_base.base.prv.event_queue, type, window, evt);
  } else {
    lvkw_event_queue_push(&ctx_x11->linux_base.base, &ctx_x11->linux_base.base.prv.event_queue, type, window, evt);
  }
}

LVKW_Status lvkw_ctx_postEvent_X11(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                   const LVKW_Event *evt) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  LVKW_Event empty_evt = {0};
  if (!evt) evt = &empty_evt;
  
  if (!lvkw_event_queue_push_external(&ctx->linux_base.base.prv.event_queue, type, window, evt)) {
    return LVKW_ERROR;
  }
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_X11(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  lvkw_event_queue_scan(&ctx->linux_base.base.prv.event_queue, event_mask, callback, userdata);
  return LVKW_SUCCESS;
}
