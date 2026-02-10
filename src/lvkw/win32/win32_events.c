#include <windowsx.h>

#include "lvkw_api_checks.h"
#include "lvkw_win32_internal.h"

void _lvkw_win32_update_cursor_clip(LVKW_Window_Win32 *window) {
  if (window->cursor_mode == LVKW_CURSOR_LOCKED && GetFocus() == window->hwnd) {
    RECT rect;
    GetClientRect(window->hwnd, &rect);
    ClientToScreen(window->hwnd, (LPPOINT)&rect.left);
    ClientToScreen(window->hwnd, (LPPOINT)&rect.right);
    ClipCursor(&rect);
  }
  else {
    ClipCursor(NULL);
  }
}

LVKW_ModifierFlags _lvkw_win32_get_modifiers(void) {
  LVKW_ModifierFlags mods = 0;
  if (GetKeyState(VK_SHIFT) & 0x8000) mods |= LVKW_MODIFIER_SHIFT;
  if (GetKeyState(VK_CONTROL) & 0x8000) mods |= LVKW_MODIFIER_CONTROL;
  if (GetKeyState(VK_MENU) & 0x8000) mods |= LVKW_MODIFIER_ALT;
  if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) mods |= LVKW_MODIFIER_SUPER;
  if (GetKeyState(VK_CAPITAL) & 1) mods |= LVKW_MODIFIER_CAPS_LOCK;
  if (GetKeyState(VK_NUMLOCK) & 1) mods |= LVKW_MODIFIER_NUM_LOCK;
  return mods;
}

LVKW_Key _lvkw_win32_translate_key(WPARAM wParam, LPARAM lParam) {
  switch (wParam) {
    case VK_SPACE:
      return LVKW_KEY_SPACE;
    case VK_OEM_7:
      return LVKW_KEY_APOSTROPHE;
    case VK_OEM_COMMA:
      return LVKW_KEY_COMMA;
    case VK_OEM_MINUS:
      return LVKW_KEY_MINUS;
    case VK_OEM_PERIOD:
      return LVKW_KEY_PERIOD;
    case VK_OEM_2:
      return LVKW_KEY_SLASH;
    case '0':
      return LVKW_KEY_0;
    case '1':
      return LVKW_KEY_1;
    case '2':
      return LVKW_KEY_2;
    case '3':
      return LVKW_KEY_3;
    case '4':
      return LVKW_KEY_4;
    case '5':
      return LVKW_KEY_5;
    case '6':
      return LVKW_KEY_6;
    case '7':
      return LVKW_KEY_7;
    case '8':
      return LVKW_KEY_8;
    case '9':
      return LVKW_KEY_9;
    case VK_OEM_1:
      return LVKW_KEY_SEMICOLON;
    case VK_OEM_PLUS:
      return LVKW_KEY_EQUAL;
    case 'A':
      return LVKW_KEY_A;
    case 'B':
      return LVKW_KEY_B;
    case 'C':
      return LVKW_KEY_C;
    case 'D':
      return LVKW_KEY_D;
    case 'E':
      return LVKW_KEY_E;
    case 'F':
      return LVKW_KEY_F;
    case 'G':
      return LVKW_KEY_G;
    case 'H':
      return LVKW_KEY_H;
    case 'I':
      return LVKW_KEY_I;
    case 'J':
      return LVKW_KEY_J;
    case 'K':
      return LVKW_KEY_K;
    case 'L':
      return LVKW_KEY_L;
    case 'M':
      return LVKW_KEY_M;
    case 'N':
      return LVKW_KEY_N;
    case 'O':
      return LVKW_KEY_O;
    case 'P':
      return LVKW_KEY_P;
    case 'Q':
      return LVKW_KEY_Q;
    case 'R':
      return LVKW_KEY_R;
    case 'S':
      return LVKW_KEY_S;
    case 'T':
      return LVKW_KEY_T;
    case 'U':
      return LVKW_KEY_U;
    case 'V':
      return LVKW_KEY_V;
    case 'W':
      return LVKW_KEY_W;
    case 'X':
      return LVKW_KEY_X;
    case 'Y':
      return LVKW_KEY_Y;
    case 'Z':
      return LVKW_KEY_Z;
    case VK_OEM_4:
      return LVKW_KEY_LEFT_BRACKET;
    case VK_OEM_5:
      return LVKW_KEY_BACKSLASH;
    case VK_OEM_6:
      return LVKW_KEY_RIGHT_BRACKET;
    case VK_OEM_3:
      return LVKW_KEY_GRAVE_ACCENT;
    case VK_ESCAPE:
      return LVKW_KEY_ESCAPE;
    case VK_RETURN:
      return (lParam & 0x01000000) ? LVKW_KEY_KP_ENTER : LVKW_KEY_ENTER;
    case VK_TAB:
      return LVKW_KEY_TAB;
    case VK_BACK:
      return LVKW_KEY_BACKSPACE;
    case VK_INSERT:
      return LVKW_KEY_INSERT;
    case VK_DELETE:
      return LVKW_KEY_DELETE;
    case VK_RIGHT:
      return LVKW_KEY_RIGHT;
    case VK_LEFT:
      return LVKW_KEY_LEFT;
    case VK_DOWN:
      return LVKW_KEY_DOWN;
    case VK_UP:
      return LVKW_KEY_UP;
    case VK_PRIOR:
      return LVKW_KEY_PAGE_UP;
    case VK_NEXT:
      return LVKW_KEY_PAGE_DOWN;
    case VK_HOME:
      return LVKW_KEY_HOME;
    case VK_END:
      return LVKW_KEY_END;
    case VK_CAPITAL:
      return LVKW_KEY_CAPS_LOCK;
    case VK_SCROLL:
      return LVKW_KEY_SCROLL_LOCK;
    case VK_NUMLOCK:
      return LVKW_KEY_NUM_LOCK;
    case VK_SNAPSHOT:
      return LVKW_KEY_PRINT_SCREEN;
    case VK_PAUSE:
      return LVKW_KEY_PAUSE;
    case VK_F1:
      return LVKW_KEY_F1;
    case VK_F2:
      return LVKW_KEY_F2;
    case VK_F3:
      return LVKW_KEY_F3;
    case VK_F4:
      return LVKW_KEY_F4;
    case VK_F5:
      return LVKW_KEY_F5;
    case VK_F6:
      return LVKW_KEY_F6;
    case VK_F7:
      return LVKW_KEY_F7;
    case VK_F8:
      return LVKW_KEY_F8;
    case VK_F9:
      return LVKW_KEY_F9;
    case VK_F10:
      return LVKW_KEY_F10;
    case VK_F11:
      return LVKW_KEY_F11;
    case VK_F12:
      return LVKW_KEY_F12;
    case VK_NUMPAD0:
      return LVKW_KEY_KP_0;
    case VK_NUMPAD1:
      return LVKW_KEY_KP_1;
    case VK_NUMPAD2:
      return LVKW_KEY_KP_2;
    case VK_NUMPAD3:
      return LVKW_KEY_KP_3;
    case VK_NUMPAD4:
      return LVKW_KEY_KP_4;
    case VK_NUMPAD5:
      return LVKW_KEY_KP_5;
    case VK_NUMPAD6:
      return LVKW_KEY_KP_6;
    case VK_NUMPAD7:
      return LVKW_KEY_KP_7;
    case VK_NUMPAD8:
      return LVKW_KEY_KP_8;
    case VK_NUMPAD9:
      return LVKW_KEY_KP_9;
    case VK_DECIMAL:
      return LVKW_KEY_KP_DECIMAL;
    case VK_DIVIDE:
      return LVKW_KEY_KP_DIVIDE;
    case VK_MULTIPLY:
      return LVKW_KEY_KP_MULTIPLY;
    case VK_SUBTRACT:
      return LVKW_KEY_KP_SUBTRACT;
    case VK_ADD:
      return LVKW_KEY_KP_ADD;
    case VK_SHIFT: {
      UINT scancode = (lParam & 0x00ff0000) >> 16;
      return (scancode == MapVirtualKeyW(VK_RSHIFT, MAPVK_VK_TO_VSC)) ? LVKW_KEY_RIGHT_SHIFT : LVKW_KEY_LEFT_SHIFT;
    }
    case VK_CONTROL:
      return (lParam & 0x01000000) ? LVKW_KEY_RIGHT_CONTROL : LVKW_KEY_LEFT_CONTROL;
    case VK_MENU:
      return (lParam & 0x01000000) ? LVKW_KEY_RIGHT_ALT : LVKW_KEY_LEFT_ALT;
    case VK_LWIN:
      return LVKW_KEY_LEFT_SUPER;
    case VK_RWIN:
      return LVKW_KEY_RIGHT_SUPER;
    case VK_APPS:
      return LVKW_KEY_MENU;
    default:
      return LVKW_KEY_UNKNOWN;
  }
}

static void _lvkw_win32_send_event(LVKW_Context_Win32 *ctx, LVKW_Event *ev) {
  if (ctx->current_event_mask & ev->type) {
    ctx->current_event_callback(ev, ctx->current_event_userdata);
  }
}

LRESULT CALLBACK _lvkw_win32_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  LVKW_Window_Win32 *window = (LVKW_Window_Win32 *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
  if (!window) return DefWindowProcW(hwnd, msg, wParam, lParam);

  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)window->base.prv.ctx_base;
  if (!ctx->is_polling || !ctx->current_event_callback) return DefWindowProcW(hwnd, msg, wParam, lParam);

  LVKW_Event ev = {0};
  ev.common.window = (LVKW_Window *)window;

  switch (msg) {
    case WM_CLOSE:
      ev.type = LVKW_EVENT_TYPE_CLOSE_REQUESTED;
      _lvkw_win32_send_event(ctx, &ev);
      return 0;

    case WM_DPICHANGED: {
      // Suggested new window size for the new DPI
      RECT *const prcNewWindow = (RECT *)lParam;
      SetWindowPos(hwnd, NULL, prcNewWindow->left, prcNewWindow->top, prcNewWindow->right - prcNewWindow->left,
                   prcNewWindow->bottom - prcNewWindow->top, SWP_NOZORDER | SWP_NOACTIVATE);
      return 0;
    }

    case WM_SIZE: {
      uint32_t width = LOWORD(lParam);
      uint32_t height = HIWORD(lParam);
      window->size.width = width;
      window->size.height = height;

      ev.type = LVKW_EVENT_TYPE_WINDOW_RESIZED;
      ev.resized.size = window->size;
      ev.resized.framebufferSize = window->size;  // Physical pixels in DPI-aware mode
      _lvkw_win32_send_event(ctx, &ev);
      return 0;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      ev.type = LVKW_EVENT_TYPE_KEY;
      ev.key.key = _lvkw_win32_translate_key(wParam, lParam);
      ev.key.state = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED;
      ev.key.modifiers = _lvkw_win32_get_modifiers();
      _lvkw_win32_send_event(ctx, &ev);
      return 0;
    }

    case WM_MOUSEMOVE: {
      double x = (double)GET_X_LPARAM(lParam);
      double y = (double)GET_Y_LPARAM(lParam);

      if (!window->has_last_pos) {
        window->last_x = x;
        window->last_y = y;
        window->has_last_pos = true;
      }

      double dx = x - window->last_x;
      double dy = y - window->last_y;

      // If no motion (likely due to re-centering), skip event
      if (dx == 0.0 && dy == 0.0) return 0;

      ev.type = LVKW_EVENT_TYPE_MOUSE_MOTION;
      ev.mouse_motion.x = x;
      ev.mouse_motion.y = y;
      ev.mouse_motion.dx = dx;
      ev.mouse_motion.dy = dy;

      _lvkw_win32_send_event(ctx, &ev);

      // Update last pos
      window->last_x = x;
      window->last_y = y;

      // Re-center if locked
      if (window->cursor_mode == LVKW_CURSOR_LOCKED && GetFocus() == hwnd) {
        RECT rect;
        GetClientRect(hwnd, &rect);
        int centerX = (rect.right - rect.left) / 2;
        int centerY = (rect.bottom - rect.top) / 2;

        // If not at center, move to center
        if ((int)x != centerX || (int)y != centerY) {
          POINT pt = {centerX, centerY};
          ClientToScreen(hwnd, &pt);
          SetCursorPos(pt.x, pt.y);

          // Update last_x/y to the new center position so the next WM_MOUSEMOVE
          // (which will be at center) produces dx=0, dy=0 and is ignored.
          window->last_x = (double)centerX;
          window->last_y = (double)centerY;
        }
      }

      if (!window->cursor_in_client_area) {
        window->cursor_in_client_area = true;
        TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
        TrackMouseEvent(&tme);
      }

      return 0;
    }

    case WM_MOUSELEAVE:
      window->cursor_in_client_area = false;
      window->has_last_pos = false;
      return 0;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP: {
      ev.type = LVKW_EVENT_TYPE_MOUSE_BUTTON;
      if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
        ev.mouse_button.button = LVKW_MOUSE_BUTTON_LEFT;
      else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
        ev.mouse_button.button = LVKW_MOUSE_BUTTON_RIGHT;
      else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
        ev.mouse_button.button = LVKW_MOUSE_BUTTON_MIDDLE;
      else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
        ev.mouse_button.button = LVKW_MOUSE_BUTTON_4;
      else
        ev.mouse_button.button = LVKW_MOUSE_BUTTON_5;

      ev.mouse_button.state =
          (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN)
              ? LVKW_BUTTON_STATE_PRESSED
              : LVKW_BUTTON_STATE_RELEASED;

      _lvkw_win32_send_event(ctx, &ev);
      return 0;
    }

    case WM_MOUSEWHEEL: {
      ev.type = LVKW_EVENT_TYPE_MOUSE_SCROLL;
      ev.mouse_scroll.dx = 0;
      ev.mouse_scroll.dy = (double)GET_WHEEL_DELTA_WPARAM(wParam) / (double)WHEEL_DELTA;
      _lvkw_win32_send_event(ctx, &ev);
      return 0;
    }

    case WM_MOUSEHWHEEL: {
      ev.type = LVKW_EVENT_TYPE_MOUSE_SCROLL;
      ev.mouse_scroll.dx = (double)GET_WHEEL_DELTA_WPARAM(wParam) / (double)WHEEL_DELTA;
      ev.mouse_scroll.dy = 0;
      _lvkw_win32_send_event(ctx, &ev);
      return 0;
    }

    case WM_SETFOCUS:
      _lvkw_win32_update_cursor_clip(window);
      return 0;

    case WM_KILLFOCUS:
      ClipCursor(NULL);
      return 0;

    case WM_ERASEBKGND:
      return 1;
      break;

    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT) {
        if (window->cursor_mode == LVKW_CURSOR_LOCKED) {
          SetCursor(NULL);
        }
        else {
          SetCursor(window->current_hcursor);
        }
        return TRUE;
      }
      break;
  }

  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LVKW_ContextResult lvkw_context_pollEvents_Win32(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata) {
  return lvkw_context_waitEvents_Win32(ctx_handle, 0, event_mask, callback, userdata);
}

LVKW_ContextResult lvkw_context_waitEvents_Win32(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                                 LVKW_EventType event_mask, LVKW_EventCallback callback,
                                                 void *userdata) {
  LVKW_Context_Win32 *ctx = (LVKW_Context_Win32 *)ctx_handle;

  ctx->current_event_callback = callback;
  ctx->current_event_userdata = userdata;
  ctx->current_event_mask = event_mask;
  ctx->is_polling = true;

  if (timeout_ms > 0) {
    DWORD wait_result = MsgWaitForMultipleObjectsEx(0, NULL, timeout_ms, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
    if (wait_result == WAIT_TIMEOUT) {
      ctx->is_polling = false;
      ctx->current_event_callback = NULL;
      ctx->current_event_userdata = NULL;
      ctx->current_event_mask = 0;
      return LVKW_OK;
    }
  }

  MSG msg;
  while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
    ctx->last_event_time = GetTickCount();
  }

  if (ctx->idle_timeout_ms != LVKW_IDLE_NEVER) {
    uint32_t now = GetTickCount();
    if (now - ctx->last_event_time >= ctx->idle_timeout_ms) {
      LVKW_Window_Win32 *curr = ctx->window_list_head;
      while (curr) {
        LVKW_Event ev = {0};
        ev.type = LVKW_EVENT_TYPE_IDLE_NOTIFICATION;
        ev.idle.window = (LVKW_Window *)curr;
        ev.idle.is_idle = true;
        ev.idle.timeout_ms = ctx->idle_timeout_ms;

        if (ctx->current_event_mask & ev.type) {
          callback(&ev, userdata);
        }
        curr = curr->next;
      }
    }
  }

  ctx->is_polling = false;
  ctx->current_event_callback = NULL;
  ctx->current_event_userdata = NULL;
  ctx->current_event_mask = 0;

  return LVKW_OK;
}
