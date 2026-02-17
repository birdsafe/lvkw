// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xatom.h>

#include "dlib/X11.h"
#include "dlib/Xi.h"
#include "dlib/Xrandr.h"
#include "dlib/Xss.h"
#include "lvkw/lvkw.h"
#include "api_constraints.h"
#include "x11_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "controller/controller_internal.h"
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

static bool _lvkw_x11_has_wm_state_atom(LVKW_Context_X11 *ctx, LVKW_Window_X11 *window, Atom atom) {
  Atom actual_type = None;
  int actual_format = 0;
  unsigned long count = 0;
  unsigned long bytes_after = 0;
  unsigned char *data = NULL;

  if (lvkw_XGetWindowProperty(ctx, ctx->display, window->window, ctx->net_wm_state, 0, 32, False,
                              XA_ATOM, &actual_type, &actual_format, &count, &bytes_after,
                              &data) != Success) {
    return false;
  }

  bool found = false;
  if (actual_type == XA_ATOM && actual_format == 32 && data) {
    const Atom *atoms = (const Atom *)data;
    for (unsigned long i = 0; i < count; ++i) {
      if (atoms[i] == atom) {
        found = true;
        break;
      }
    }
  }

  if (data) lvkw_XFree(ctx, data);
  return found;
}

static void _lvkw_x11_update_idle_state(LVKW_Context_X11 *ctx) {
  if (!ctx->xss_available) return;

  uint64_t now_ms = _lvkw_get_timestamp_ms();
  if (ctx->next_idle_probe_ms != 0 && now_ms < ctx->next_idle_probe_ms) return;
  ctx->next_idle_probe_ms = now_ms + (uint64_t)ctx->idle_poll_interval_ms;

  XScreenSaverInfo info;
  memset(&info, 0, sizeof(info));
  if (!lvkw_XScreenSaverQueryInfo(ctx, ctx->display, DefaultRootWindow(ctx->display), &info)) return;

  bool is_idle = info.state != ScreenSaverOff;
  if (is_idle == ctx->is_idle) return;

  ctx->is_idle = is_idle;
  LVKW_Event evt = {0};
  evt.idle.timeout_ms = 0;
  evt.idle.is_idle = is_idle;
  lvkw_event_queue_push_with_mask(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                  ctx->linux_base.base.prv.pump_event_mask,
                                  LVKW_EVENT_TYPE_IDLE_STATE_CHANGED, NULL, &evt);
}

static const LVKW_X11ClipboardMime *_lvkw_x11_find_owned_mime_by_atom(const LVKW_Context_X11 *ctx,
                                                                       Atom target) {
  for (uint32_t i = 0; i < ctx->clipboard_owned_mime_count; ++i) {
    if (ctx->clipboard_owned_mimes[i].atom == target) {
      return &ctx->clipboard_owned_mimes[i];
    }
  }
  return NULL;
}

static void _lvkw_x11_process_xi_event(LVKW_Context_X11 *ctx, XEvent *xev) {
  if (ctx->xi_opcode < 0 || xev->xcookie.extension != ctx->xi_opcode) return;
  if (xev->xcookie.evtype != XI_RawMotion) return;
  if (!lvkw_XGetEventData(ctx, ctx->display, &xev->xcookie)) return;

  XIRawEvent *raw = (XIRawEvent *)xev->xcookie.data;
  if (raw) {
    LVKW_Scalar dx = 0;
    LVKW_Scalar dy = 0;
    double *raw_values = raw->raw_values;
    for (int axis = 0; axis < raw->valuators.mask_len * 8; ++axis) {
      if (!XIMaskIsSet(raw->valuators.mask, axis)) continue;
      double value = *raw_values++;
      if (axis == 0) dx = (LVKW_Scalar)value;
      else if (axis == 1) dy = (LVKW_Scalar)value;
    }
    ctx->pending_raw_delta.x += dx;
    ctx->pending_raw_delta.y += dy;
    ctx->has_pending_raw_delta = true;
  }

  lvkw_XFreeEventData(ctx, ctx->display, &xev->xcookie);
}

static void _lvkw_x11_process_event(LVKW_Context_X11 *ctx, XEvent *xev) {
  const uint32_t mask = ctx->linux_base.base.prv.pump_event_mask;
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
      lvkw_event_queue_push_with_mask(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                      mask, LVKW_EVENT_TYPE_KEY, (LVKW_Window *)window, &ev);

      if (xev->type == KeyPress && ctx->linux_base.xkb.state) {
        char buffer[64];
        int len =
            lvkw_xkb_state_key_get_utf8(ctx, ctx->linux_base.xkb.state, xev->xkey.keycode, buffer, sizeof(buffer));
        if (len > 0) {
          LVKW_Event text_evt = {0};
          text_evt.text_input.text =
              lvkw_event_queue_transient_intern_sized(&ctx->linux_base.base.prv.event_queue, buffer, (size_t)len);
          text_evt.text_input.length = (uint32_t)len;
          lvkw_event_queue_push_with_mask(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                          mask, LVKW_EVENT_TYPE_TEXT_INPUT, (LVKW_Window *)window,
                                          &text_evt);
        }
      }
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
            if (xev->xbutton.button == 4) {
              sev.mouse_scroll.delta.y = 1.0f;
              sev.mouse_scroll.steps.y = 1;
            }
            else if (xev->xbutton.button == 5) {
              sev.mouse_scroll.delta.y = -1.0f;
              sev.mouse_scroll.steps.y = -1;
            }
            else if (xev->xbutton.button == 6) {
              sev.mouse_scroll.delta.x = -1.0f;
              sev.mouse_scroll.steps.x = -1;
            }
            else if (xev->xbutton.button == 7) {
              sev.mouse_scroll.delta.x = 1.0f;
              sev.mouse_scroll.steps.x = 1;
            }
            
            lvkw_event_queue_push_compressible_with_mask(
                &ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, mask,
                LVKW_EVENT_TYPE_MOUSE_SCROLL, (LVKW_Window *)window, &sev);
        }
        break;
      }
      
      if (ev.mouse_button.button != (LVKW_MouseButton)0xFFFFFFFF) {
          ev.mouse_button.state = (xev->type == ButtonPress) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
          ev.mouse_button.modifiers = _lvkw_x11_get_modifiers(xev->xbutton.state);
          lvkw_event_queue_push_with_mask(&ctx->linux_base.base,
                                          &ctx->linux_base.base.prv.event_queue, mask,
                                          LVKW_EVENT_TYPE_MOUSE_BUTTON, (LVKW_Window *)window, &ev);
      }
      break;
    }

    case MotionNotify: {
      if (!window) break;
      LVKW_Event ev = {0};
      ev.mouse_motion.position.x = (LVKW_Scalar)xev->xmotion.x / ctx->scale;
      ev.mouse_motion.position.y = (LVKW_Scalar)xev->xmotion.y / ctx->scale;

      if (window->last_cursor_set) {
        ev.mouse_motion.delta.x = ev.mouse_motion.position.x - window->last_x;
        ev.mouse_motion.delta.y = ev.mouse_motion.position.y - window->last_y;
      }

      window->last_x = ev.mouse_motion.position.x;
      window->last_y = ev.mouse_motion.position.y;
      window->last_cursor_set = true;

      if (ctx->locked_window == window && window->cursor_mode == LVKW_CURSOR_LOCKED &&
          ctx->has_pending_raw_delta) {
        ev.mouse_motion.raw_delta = ctx->pending_raw_delta;
        ctx->pending_raw_delta.x = 0;
        ctx->pending_raw_delta.y = 0;
        ctx->has_pending_raw_delta = false;
      }

      lvkw_event_queue_push_compressible_with_mask(
          &ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, mask,
          LVKW_EVENT_TYPE_MOUSE_MOTION, (LVKW_Window *)window, &ev);
      break;
    }

    case EnterNotify: {
      if (!window) break;
      window->last_x = (LVKW_Scalar)xev->xcrossing.x / ctx->scale;
      window->last_y = (LVKW_Scalar)xev->xcrossing.y / ctx->scale;
      window->last_cursor_set = true;
      if (window->cursor_mode != LVKW_CURSOR_LOCKED) {
        _lvkw_wnd_setCursor_X11((LVKW_Window *)window, window->cursor);
      }
      break;
    }

    case LeaveNotify: {
      if (!window) break;
      window->last_cursor_set = false;
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
          ev.resized.geometry.logical_size = window->size;
          ev.resized.geometry.pixel_size.x = xev->xconfigure.width;
          ev.resized.geometry.pixel_size.y = xev->xconfigure.height;
          
          lvkw_event_queue_push_compressible_with_mask(
              &ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, mask,
              LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window, &ev);
      }
      break;
    }

    case PropertyNotify: {
      if (!window) break;
      if (xev->xproperty.atom != ctx->net_wm_state) break;

      bool was_maximized = (window->base.pub.flags & LVKW_WINDOW_STATE_MAXIMIZED) != 0;
      bool is_maximized =
          _lvkw_x11_has_wm_state_atom(ctx, window, ctx->net_wm_state_maximized_vert) &&
          _lvkw_x11_has_wm_state_atom(ctx, window, ctx->net_wm_state_maximized_horz);
      if (was_maximized != is_maximized) {
        if (is_maximized) window->base.pub.flags |= LVKW_WINDOW_STATE_MAXIMIZED;
        else window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_MAXIMIZED;

        LVKW_Event ev = {0};
        ev.maximized.maximized = is_maximized;
        lvkw_event_queue_push_with_mask(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                        mask, LVKW_EVENT_TYPE_WINDOW_MAXIMIZED,
                                        (LVKW_Window *)window, &ev);
      }

      bool is_fullscreen =
          _lvkw_x11_has_wm_state_atom(ctx, window, ctx->net_wm_state_fullscreen);
      if (is_fullscreen) window->base.pub.flags |= LVKW_WINDOW_STATE_FULLSCREEN;
      else window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_FULLSCREEN;
      break;
    }

    case MapNotify: {
      if (!window) break;
      if (!(window->base.pub.flags & LVKW_WINDOW_STATE_READY)) {
          window->base.pub.flags |= LVKW_WINDOW_STATE_READY;
          LVKW_Event ev = {0};
          lvkw_event_queue_push_with_mask(&ctx->linux_base.base,
                                          &ctx->linux_base.base.prv.event_queue, mask,
                                          LVKW_EVENT_TYPE_WINDOW_READY, (LVKW_Window *)window,
                                          &ev);
      }
      break;
    }

    case ClientMessage: {
      if (!window) break;
      if (xev->xclient.message_type == ctx->wm_protocols) {
        Atom protocol = (Atom)xev->xclient.data.l[0];
        if (protocol == ctx->wm_delete_window) {
          LVKW_Event ev = {0};
          lvkw_event_queue_push_with_mask(&ctx->linux_base.base,
                                          &ctx->linux_base.base.prv.event_queue, mask,
                                          LVKW_EVENT_TYPE_CLOSE_REQUESTED, (LVKW_Window *)window,
                                          &ev);
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

    case SelectionRequest: {
      XSelectionRequestEvent *req = &xev->xselectionrequest;
      if (req->selection != ctx->clipboard) break;

      XEvent reply;
      memset(&reply, 0, sizeof(reply));
      reply.xselection.type = SelectionNotify;
      reply.xselection.display = req->display;
      reply.xselection.requestor = req->requestor;
      reply.xselection.selection = req->selection;
      reply.xselection.target = req->target;
      reply.xselection.time = req->time;
      reply.xselection.property = None;

      Atom property = (req->property != None) ? req->property : req->target;

      if (req->target == ctx->targets) {
        uint32_t atom_count = ctx->clipboard_owned_mime_count + 3;
        Atom *atoms = (Atom *)_ctx_alloc(ctx, sizeof(Atom) * atom_count);
        if (atoms) {
          uint32_t out = 0;
          atoms[out++] = ctx->targets;
          atoms[out++] = ctx->utf8_string;
          atoms[out++] = XA_STRING;
          for (uint32_t i = 0; i < ctx->clipboard_owned_mime_count; ++i) {
            atoms[out++] = ctx->clipboard_owned_mimes[i].atom;
          }
          lvkw_XChangeProperty(ctx, ctx->display, req->requestor, property, XA_ATOM, 32,
                               PropModeReplace, (const unsigned char *)atoms, (int)out);
          reply.xselection.property = property;
          _ctx_free(ctx, atoms);
        }
      } else {
        const LVKW_X11ClipboardMime *mime = _lvkw_x11_find_owned_mime_by_atom(ctx, req->target);
        if (!mime && (req->target == ctx->utf8_string || req->target == ctx->text_atom ||
                      req->target == XA_STRING)) {
          mime = _lvkw_x11_find_owned_mime_by_atom(ctx, ctx->utf8_string);
          if (!mime) mime = _lvkw_x11_find_owned_mime_by_atom(ctx, XA_STRING);
        }
        if (mime) {
          Atom type = (req->target == XA_STRING) ? XA_STRING : req->target;
          lvkw_XChangeProperty(ctx, ctx->display, req->requestor, property, type, 8,
                               PropModeReplace, (const unsigned char *)mime->bytes, (int)mime->size);
          reply.xselection.property = property;
        }
      }

      lvkw_XSendEvent(ctx, ctx->display, req->requestor, False, 0, &reply);
      break;
    }

    case SelectionClear: {
      if (xev->xselectionclear.selection == ctx->clipboard) {
        ctx->clipboard_owner_window = 0;
      }
      break;
    }

    case FocusIn:
    case FocusOut: {
      if (!window) break;
      bool focused = (xev->type == FocusIn);
      bool old_focused = (window->base.pub.flags & LVKW_WINDOW_STATE_FOCUSED) != 0;
      if (!focused) window->last_cursor_set = false;
      if (focused != old_focused) {
          if (focused) window->base.pub.flags |= LVKW_WINDOW_STATE_FOCUSED;
          else window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_FOCUSED;
          
          LVKW_Event ev = {0};
          ev.focus.focused = focused;
          lvkw_event_queue_push_with_mask(&ctx->linux_base.base,
                                          &ctx->linux_base.base.prv.event_queue, mask,
                                          LVKW_EVENT_TYPE_FOCUS, (LVKW_Window *)window, &ev);
      }
      break;
    }

    default: {
      if (xev->type == GenericEvent) {
        _lvkw_x11_process_xi_event(ctx, xev);
      }
      if (ctx->randr_available && (xev->type == ctx->randr_event_base + RRScreenChangeNotify ||
                                   xev->type == ctx->randr_event_base + RRNotify)) {
          _lvkw_x11_update_monitors(ctx);
      }
      break;
    }
  }
}

LVKW_Status lvkw_ctx_pumpEvents_X11(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  ctx->linux_base.base.prv.pump_event_mask =
      LVKW_ATOMIC_LOAD_RELAXED(&ctx->linux_base.base.prv.event_mask);

  uint64_t start_time = (timeout_ms != LVKW_NEVER && timeout_ms > 0) ? _lvkw_get_timestamp_ms() : 0;

  for (;;) {
    _lvkw_x11_update_idle_state(ctx);
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

    struct pollfd pfds[2];
    nfds_t pfd_count = 1;
    pfds[0].fd = lvkw_XConnectionNumber(ctx, ctx->display);
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;

    if (ctx->wake_pipe_read >= 0) {
      pfds[1].fd = ctx->wake_pipe_read;
      pfds[1].events = POLLIN;
      pfds[1].revents = 0;
      pfd_count = 2;
    }

    int ret = poll(pfds, pfd_count, poll_timeout);
    if (ret <= 0) break;

    if (pfd_count > 1 && (pfds[1].revents & POLLIN)) {
      char buf[64];
      while (read(ctx->wake_pipe_read, buf, sizeof(buf)) > 0) {
      }
      break;
    }
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_commitEvents_X11(LVKW_Context *ctx_handle) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  lvkw_event_queue_begin_gather(&ctx->linux_base.base.prv.event_queue);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  lvkw_event_queue_note_commit_success(&ctx->linux_base.base.prv.event_queue);
  return LVKW_SUCCESS;
}

void _lvkw_x11_push_event_cb(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                             const LVKW_Event *evt) {
  LVKW_Context_X11 *ctx_x11 = (LVKW_Context_X11 *)ctx;
  if (type == LVKW_EVENT_TYPE_MOUSE_MOTION || type == LVKW_EVENT_TYPE_MOUSE_SCROLL ||
      type == LVKW_EVENT_TYPE_WINDOW_RESIZED) {
    lvkw_event_queue_push_compressible_with_mask(
        &ctx_x11->linux_base.base, &ctx_x11->linux_base.base.prv.event_queue,
        ctx_x11->linux_base.base.prv.pump_event_mask, type, window, evt);
  } else {
    lvkw_event_queue_push_with_mask(&ctx_x11->linux_base.base,
                                    &ctx_x11->linux_base.base.prv.event_queue,
                                    ctx_x11->linux_base.base.prv.pump_event_mask, type, window,
                                    evt);
  }
}

LVKW_Status lvkw_ctx_postEvent_X11(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                   const LVKW_Event *evt) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_Event empty_evt = {0};
  if (!evt) evt = &empty_evt;
  
  if (!lvkw_event_queue_push_external(&ctx->linux_base.base.prv.event_queue, type, window, evt)) {
    return LVKW_ERROR;
  }

  if (ctx->wake_pipe_write >= 0) {
    const char wake = 1;
    (void)write(ctx->wake_pipe_write, &wake, 1);
  }

  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_X11(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  lvkw_event_queue_scan(&ctx->linux_base.base.prv.event_queue, event_mask, callback, userdata);

  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  return LVKW_SUCCESS;
}
