#include <stdio.h>
#include <string.h>

#include "dlib/X11.h"
#include "dlib/Xi.h"  // IWYU pragma: keep
#include "dlib/Xss.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_x11_internal.h"

#define LVKW_X11_MAX_EVENTS 4096

typedef struct LVKW_EventDispatchContext_X11 {
  LVKW_EventCallback callback;
  void *userdata;
  LVKW_EventType evt_mask;
} LVKW_EventDispatchContext_X11;

static void _lvkw_x11_push_event(LVKW_Context_X11 *ctx, const LVKW_Event *evt) {
  if (!lvkw_event_queue_push(&ctx->base, &ctx->event_queue, evt)) {
    LVKW_REPORT_CTX_DIAGNOSIS(ctx, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "X11 event queue is full or allocation failed");
  }
}

static void _lvkw_x11_flush_event_pool(LVKW_Context_X11 *ctx, const LVKW_EventDispatchContext_X11 *dispatch) {
  LVKW_Event ev;
  while (lvkw_event_queue_pop(&ctx->event_queue, dispatch->evt_mask, &ev)) {
    dispatch->callback(&ev, dispatch->userdata);
  }
}

LVKW_ContextResult lvkw_context_pollEvents_X11(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                               LVKW_EventCallback callback, void *userdata) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  _lvkw_x11_check_error(ctx);
  if (ctx->base.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_EventDispatchContext_X11 dispatch = {
      .callback = callback,
      .userdata = userdata,
      .evt_mask = event_mask,
  };

  while (XPending(ctx->display)) {
    XEvent ev;
    XNextEvent(ctx->display, &ev);

    if (ev.type == ClientMessage) {
      if (ev.xclient.message_type == ctx->wm_protocols) {
        Atom protocol = (Atom)ev.xclient.data.l[0];
        if (protocol == ctx->wm_delete_window) {
          if (event_mask & LVKW_EVENT_TYPE_CLOSE_REQUESTED) {
            LVKW_Event lvkw_ev;
            lvkw_ev.type = LVKW_EVENT_TYPE_CLOSE_REQUESTED;

            LVKW_Window_X11 *window = NULL;
            if (XFindContext(ctx->display, ev.xclient.window, ctx->window_context, (XPointer *)&window) == 0) {
              lvkw_ev.close_requested.window = (LVKW_Window *)window;
              _lvkw_x11_push_event(ctx, &lvkw_ev);
            }
          }
        }

        else if (protocol == ctx->net_wm_ping) {
          XEvent reply = ev;

          reply.xclient.window = DefaultRootWindow(ctx->display);

          XSendEvent(ctx->display, reply.xclient.window, False, SubstructureNotifyMask | SubstructureRedirectMask,

                     &reply);
        }

        else if (protocol == ctx->wm_take_focus) {
          XSetInputFocus(ctx->display, ev.xclient.window, RevertToParent, (Time)ev.xclient.data.l[1]);
        }
      }
    }

    else if (ev.type == ConfigureNotify) {
      LVKW_Window_X11 *window = NULL;

      if (XFindContext(ctx->display, ev.xconfigure.window, ctx->window_context, (XPointer *)&window) == 0) {
        uint32_t logical_width = (uint32_t)((double)ev.xconfigure.width / ctx->scale);
        uint32_t logical_height = (uint32_t)((double)ev.xconfigure.height / ctx->scale);

        if (window->size.width != logical_width || window->size.height != logical_height) {
          window->size.width = logical_width;
          window->size.height = logical_height;

          if (event_mask & LVKW_EVENT_TYPE_WINDOW_RESIZED) {
            LVKW_Event lvkw_ev;
            lvkw_ev.type = LVKW_EVENT_TYPE_WINDOW_RESIZED;
            lvkw_ev.resized.window = (LVKW_Window *)window;
            lvkw_ev.resized.size = window->size;
            lvkw_ev.resized.framebufferSize.width = (uint32_t)ev.xconfigure.width;
            lvkw_ev.resized.framebufferSize.height = (uint32_t)ev.xconfigure.height;

            _lvkw_x11_push_event(ctx, &lvkw_ev);
          }
        }
      }
    }

    else if (ev.type == MapNotify) {
      if (event_mask & LVKW_EVENT_TYPE_WINDOW_READY) {
        LVKW_Window_X11 *window = NULL;

        if (XFindContext(ctx->display, ev.xmap.window, ctx->window_context, (XPointer *)&window) == 0) {
          LVKW_Event lvkw_ev;

          lvkw_ev.type = LVKW_EVENT_TYPE_WINDOW_READY;

          lvkw_ev.window_ready.window = (LVKW_Window *)window;

          _lvkw_x11_push_event(ctx, &lvkw_ev);
        }
      }
    }

    else if (ev.type == KeyPress || ev.type == KeyRelease) {
      if (event_mask & LVKW_EVENT_TYPE_KEY) {
        LVKW_Window_X11 *window = NULL;

        if (XFindContext(ctx->display, ev.xkey.window, ctx->window_context, (XPointer *)&window) == 0) {
          if (ev.type == KeyRelease && XPending(ctx->display)) {
            XEvent next_ev;
            XPeekEvent(ctx->display, &next_ev);
            if (next_ev.type == KeyPress && next_ev.xkey.time == ev.xkey.time &&
                next_ev.xkey.keycode == ev.xkey.keycode) {
              /* Consume the subsequent KeyPress event as well and skip both */
              XNextEvent(ctx->display, &ev);
              continue;
            }
          }

          LVKW_Key key = LVKW_KEY_UNKNOWN;
          if (ctx->xkb.state) {
            xkb_keysym_t sym = xkb_state_key_get_one_sym(ctx->xkb.state, ev.xkey.keycode);
            key = lvkw_linux_translate_keysym(sym);
          }
          else {
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            key = lvkw_linux_translate_keysym(keysym);
          }

          LVKW_Event lvkw_ev;

          lvkw_ev.type = LVKW_EVENT_TYPE_KEY;
          lvkw_ev.key.window = (LVKW_Window *)window;
          lvkw_ev.key.key = key;
          lvkw_ev.key.state = (ev.type == KeyPress) ? LVKW_KEY_STATE_PRESSED : LVKW_KEY_STATE_RELEASED;
          lvkw_ev.key.modifiers = 0;

          if (ev.xkey.state & ShiftMask) lvkw_ev.key.modifiers |= LVKW_MODIFIER_SHIFT;
          if (ev.xkey.state & ControlMask) lvkw_ev.key.modifiers |= LVKW_MODIFIER_CONTROL;
          if (ev.xkey.state & Mod1Mask) lvkw_ev.key.modifiers |= LVKW_MODIFIER_ALT;
          if (ev.xkey.state & Mod4Mask) lvkw_ev.key.modifiers |= LVKW_MODIFIER_SUPER;
          if (ev.xkey.state & LockMask) lvkw_ev.key.modifiers |= LVKW_MODIFIER_CAPS_LOCK;
          if (ev.xkey.state & Mod2Mask) lvkw_ev.key.modifiers |= LVKW_MODIFIER_NUM_LOCK;

          _lvkw_x11_push_event(ctx, &lvkw_ev);
        }
      }
    }
    else if (ev.type == MotionNotify) {
      if (event_mask & LVKW_EVENT_TYPE_MOUSE_MOTION) {
        LVKW_Window_X11 *window = NULL;

        if (XFindContext(ctx->display, ev.xmotion.window, ctx->window_context, (XPointer *)&window) == 0) {
          if (window->cursor_mode == LVKW_CURSOR_LOCKED && ctx->xi_opcode != -1) continue;

          LVKW_Event lvkw_ev;

          lvkw_ev.type = LVKW_EVENT_TYPE_MOUSE_MOTION;

          lvkw_ev.mouse_motion.window = (LVKW_Window *)window;

          lvkw_ev.mouse_motion.x = (double)ev.xmotion.x / ctx->scale;

          lvkw_ev.mouse_motion.y = (double)ev.xmotion.y / ctx->scale;

          if (window->cursor_mode == LVKW_CURSOR_LOCKED) {
            lvkw_ev.mouse_motion.dx = ((double)ev.xmotion.x - window->last_x) / ctx->scale;

            lvkw_ev.mouse_motion.dy = ((double)ev.xmotion.y - window->last_y) / ctx->scale;

            // Warp back to center to avoid hitting edges
            uint32_t phys_w = (uint32_t)((double)window->size.width * ctx->scale);
            uint32_t phys_h = (uint32_t)((double)window->size.height * ctx->scale);

            XWarpPointer(ctx->display, None, window->window, 0, 0, 0, 0, (int)(phys_w / 2), (int)(phys_h / 2));

            window->last_x = (double)(phys_w / 2.0);

            window->last_y = (double)(phys_h / 2.0);
          }

          else {
            lvkw_ev.mouse_motion.dx = 0;

            lvkw_ev.mouse_motion.dy = 0;
          }

          _lvkw_x11_push_event(ctx, &lvkw_ev);
        }
      }
    }

    else if (ev.type == ButtonPress || ev.type == ButtonRelease) {
      LVKW_Window_X11 *window = NULL;

      if (XFindContext(ctx->display, ev.xbutton.window, ctx->window_context, (XPointer *)&window) == 0) {
        if (ev.xbutton.button >= 4 && ev.xbutton.button <= 7) {
          // Scroll events

          if (ev.type == ButtonPress && (event_mask & LVKW_EVENT_TYPE_MOUSE_SCROLL)) {
            LVKW_Event lvkw_ev;

            lvkw_ev.type = LVKW_EVENT_TYPE_MOUSE_SCROLL;

            lvkw_ev.mouse_scroll.window = (LVKW_Window *)window;

            lvkw_ev.mouse_scroll.dx = 0;

            lvkw_ev.mouse_scroll.dy = 0;

            if (ev.xbutton.button == 4)

              lvkw_ev.mouse_scroll.dy = 1.0;

            else if (ev.xbutton.button == 5)

              lvkw_ev.mouse_scroll.dy = -1.0;

            else if (ev.xbutton.button == 6)

              lvkw_ev.mouse_scroll.dx = 1.0;

            else if (ev.xbutton.button == 7)

              lvkw_ev.mouse_scroll.dx = -1.0;

            _lvkw_x11_push_event(ctx, &lvkw_ev);
          }
        }

        else if (event_mask & LVKW_EVENT_TYPE_MOUSE_BUTTON) {
          LVKW_Event lvkw_ev;

          lvkw_ev.type = LVKW_EVENT_TYPE_MOUSE_BUTTON;

          lvkw_ev.mouse_button.window = (LVKW_Window *)window;

          lvkw_ev.mouse_button.state =

              (ev.type == ButtonPress) ? LVKW_MOUSE_BUTTON_STATE_PRESSED : LVKW_MOUSE_BUTTON_STATE_RELEASED;

          lvkw_ev.mouse_button.button = _lvkw_x11_translate_button(ev.xbutton.button);

          if (lvkw_ev.mouse_button.button == (LVKW_MouseButton)0xFFFFFFFF) {
            LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_UNKNOWN, "Unknown X11 button");

            return LVKW_ERROR_NOOP;
          }

          _lvkw_x11_push_event(ctx, &lvkw_ev);
        }
      }
    }

    else if (ev.type == GenericEvent && ev.xcookie.extension == ctx->xi_opcode) {
      if (XGetEventData(ctx->display, &ev.xcookie)) {
        if (ev.xcookie.evtype == XI_RawMotion) {
          XIRawEvent *re = (XIRawEvent *)ev.xcookie.data;

          LVKW_Window_X11 *window = ctx->locked_window;

          if (window && window->cursor_mode == LVKW_CURSOR_LOCKED) {
            if (event_mask & LVKW_EVENT_TYPE_MOUSE_MOTION) {
              LVKW_Event lvkw_ev;

              lvkw_ev.type = LVKW_EVENT_TYPE_MOUSE_MOTION;

              lvkw_ev.mouse_motion.window = (LVKW_Window *)window;

              lvkw_ev.mouse_motion.x = window->last_x;

              lvkw_ev.mouse_motion.y = window->last_y;

              lvkw_ev.mouse_motion.dx = 0;

              lvkw_ev.mouse_motion.dy = 0;

              double *val = re->raw_values;

              if (XIMaskIsSet(re->valuators.mask, 0)) {
                lvkw_ev.mouse_motion.dx = *val / ctx->scale;

                val++;
              }

              if (XIMaskIsSet(re->valuators.mask, 1)) {
                lvkw_ev.mouse_motion.dy = *val / ctx->scale;
              }

              _lvkw_x11_push_event(ctx, &lvkw_ev);
            }
          }
        }

        XFreeEventData(ctx->display, &ev.xcookie);
      }
    }
  }

  // Idle notification

  if (ctx->idle_timeout_ms > 0 && _lvkw_lib_xss.base.available) {
    XScreenSaverInfo *info = XScreenSaverAllocInfo();

    if (info) {
      if (XScreenSaverQueryInfo(ctx->display, DefaultRootWindow(ctx->display), info)) {
        bool currently_idle = info->idle >= ctx->idle_timeout_ms;

        if (currently_idle != ctx->is_idle) {
          ctx->is_idle = currently_idle;

          if (event_mask & LVKW_EVENT_TYPE_IDLE_NOTIFICATION) {
            LVKW_Event lvkw_ev;

            lvkw_ev.type = LVKW_EVENT_TYPE_IDLE_NOTIFICATION;

            lvkw_ev.idle.is_idle = ctx->is_idle;

            lvkw_ev.idle.window = NULL;

            lvkw_ev.idle.timeout_ms = ctx->idle_timeout_ms;

            _lvkw_x11_push_event(ctx, &lvkw_ev);
          }
        }
      }

      XFree(info);
    }
  }

  _lvkw_x11_flush_event_pool(ctx, &dispatch);

  _lvkw_x11_check_error(ctx);
  if (ctx->base.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_OK;
}
