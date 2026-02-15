// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_linux_internal.h"

LVKW_Key lvkw_linux_translate_keysym(xkb_keysym_t keysym) {
  switch (keysym) {
    case XKB_KEY_space:
      return LVKW_KEY_SPACE;
    case XKB_KEY_apostrophe:
      return LVKW_KEY_APOSTROPHE;
    case XKB_KEY_comma:
      return LVKW_KEY_COMMA;
    case XKB_KEY_minus:
      return LVKW_KEY_MINUS;
    case XKB_KEY_period:
      return LVKW_KEY_PERIOD;
    case XKB_KEY_slash:
      return LVKW_KEY_SLASH;
    case XKB_KEY_0:
      return LVKW_KEY_0;
    case XKB_KEY_1:
      return LVKW_KEY_1;
    case XKB_KEY_2:
      return LVKW_KEY_2;
    case XKB_KEY_3:
      return LVKW_KEY_3;
    case XKB_KEY_4:
      return LVKW_KEY_4;
    case XKB_KEY_5:
      return LVKW_KEY_5;
    case XKB_KEY_6:
      return LVKW_KEY_6;
    case XKB_KEY_7:
      return LVKW_KEY_7;
    case XKB_KEY_8:
      return LVKW_KEY_8;
    case XKB_KEY_9:
      return LVKW_KEY_9;
    case XKB_KEY_semicolon:
      return LVKW_KEY_SEMICOLON;
    case XKB_KEY_equal:
      return LVKW_KEY_EQUAL;
    case XKB_KEY_a:
    case XKB_KEY_A:
      return LVKW_KEY_A;
    case XKB_KEY_b:
    case XKB_KEY_B:
      return LVKW_KEY_B;
    case XKB_KEY_c:
    case XKB_KEY_C:
      return LVKW_KEY_C;
    case XKB_KEY_d:
    case XKB_KEY_D:
      return LVKW_KEY_D;
    case XKB_KEY_e:
    case XKB_KEY_E:
      return LVKW_KEY_E;
    case XKB_KEY_f:
    case XKB_KEY_F:
      return LVKW_KEY_F;
    case XKB_KEY_g:
    case XKB_KEY_G:
      return LVKW_KEY_G;
    case XKB_KEY_h:
    case XKB_KEY_H:
      return LVKW_KEY_H;
    case XKB_KEY_i:
    case XKB_KEY_I:
      return LVKW_KEY_I;
    case XKB_KEY_j:
    case XKB_KEY_J:
      return LVKW_KEY_J;
    case XKB_KEY_k:
    case XKB_KEY_K:
      return LVKW_KEY_K;
    case XKB_KEY_l:
    case XKB_KEY_L:
      return LVKW_KEY_L;
    case XKB_KEY_m:
    case XKB_KEY_M:
      return LVKW_KEY_M;
    case XKB_KEY_n:
    case XKB_KEY_N:
      return LVKW_KEY_N;
    case XKB_KEY_o:
    case XKB_KEY_O:
      return LVKW_KEY_O;
    case XKB_KEY_p:
    case XKB_KEY_P:
      return LVKW_KEY_P;
    case XKB_KEY_q:
    case XKB_KEY_Q:
      return LVKW_KEY_Q;
    case XKB_KEY_r:
    case XKB_KEY_R:
      return LVKW_KEY_R;
    case XKB_KEY_s:
    case XKB_KEY_S:
      return LVKW_KEY_S;
    case XKB_KEY_t:
    case XKB_KEY_T:
      return LVKW_KEY_T;
    case XKB_KEY_u:
    case XKB_KEY_U:
      return LVKW_KEY_U;
    case XKB_KEY_v:
    case XKB_KEY_V:
      return LVKW_KEY_V;
    case XKB_KEY_w:
    case XKB_KEY_W:
      return LVKW_KEY_W;
    case XKB_KEY_x:
    case XKB_KEY_X:
      return LVKW_KEY_X;
    case XKB_KEY_y:
    case XKB_KEY_Y:
      return LVKW_KEY_Y;
    case XKB_KEY_z:
    case XKB_KEY_Z:
      return LVKW_KEY_Z;
    case XKB_KEY_bracketleft:
      return LVKW_KEY_LEFT_BRACKET;
    case XKB_KEY_backslash:
      return LVKW_KEY_BACKSLASH;
    case XKB_KEY_bracketright:
      return LVKW_KEY_RIGHT_BRACKET;
    case XKB_KEY_grave:
      return LVKW_KEY_GRAVE_ACCENT;
    case XKB_KEY_Escape:
      return LVKW_KEY_ESCAPE;
    case XKB_KEY_Return:
      return LVKW_KEY_ENTER;
    case XKB_KEY_Tab:
      return LVKW_KEY_TAB;
    case XKB_KEY_BackSpace:
      return LVKW_KEY_BACKSPACE;
    case XKB_KEY_Insert:
      return LVKW_KEY_INSERT;
    case XKB_KEY_Delete:
      return LVKW_KEY_DELETE;
    case XKB_KEY_Right:
      return LVKW_KEY_RIGHT;
    case XKB_KEY_Left:
      return LVKW_KEY_LEFT;
    case XKB_KEY_Down:
      return LVKW_KEY_DOWN;
    case XKB_KEY_Up:
      return LVKW_KEY_UP;
    case XKB_KEY_Page_Up:
      return LVKW_KEY_PAGE_UP;
    case XKB_KEY_Page_Down:
      return LVKW_KEY_PAGE_DOWN;
    case XKB_KEY_Home:
      return LVKW_KEY_HOME;
    case XKB_KEY_End:
      return LVKW_KEY_END;
    case XKB_KEY_Caps_Lock:
      return LVKW_KEY_CAPS_LOCK;
    case XKB_KEY_Scroll_Lock:
      return LVKW_KEY_SCROLL_LOCK;
    case XKB_KEY_Num_Lock:
      return LVKW_KEY_NUM_LOCK;
    case XKB_KEY_Print:
      return LVKW_KEY_PRINT_SCREEN;
    case XKB_KEY_Pause:
      return LVKW_KEY_PAUSE;
    case XKB_KEY_F1:
      return LVKW_KEY_F1;
    case XKB_KEY_F2:
      return LVKW_KEY_F2;
    case XKB_KEY_F3:
      return LVKW_KEY_F3;
    case XKB_KEY_F4:
      return LVKW_KEY_F4;
    case XKB_KEY_F5:
      return LVKW_KEY_F5;
    case XKB_KEY_F6:
      return LVKW_KEY_F6;
    case XKB_KEY_F7:
      return LVKW_KEY_F7;
    case XKB_KEY_F8:
      return LVKW_KEY_F8;
    case XKB_KEY_F9:
      return LVKW_KEY_F9;
    case XKB_KEY_F10:
      return LVKW_KEY_F10;
    case XKB_KEY_F11:
      return LVKW_KEY_F11;
    case XKB_KEY_F12:
      return LVKW_KEY_F12;
    case XKB_KEY_KP_0:
      return LVKW_KEY_KP_0;
    case XKB_KEY_KP_1:
      return LVKW_KEY_KP_1;
    case XKB_KEY_KP_2:
      return LVKW_KEY_KP_2;
    case XKB_KEY_KP_3:
      return LVKW_KEY_KP_3;
    case XKB_KEY_KP_4:
      return LVKW_KEY_KP_4;
    case XKB_KEY_KP_5:
      return LVKW_KEY_KP_5;
    case XKB_KEY_KP_6:
      return LVKW_KEY_KP_6;
    case XKB_KEY_KP_7:
      return LVKW_KEY_KP_7;
    case XKB_KEY_KP_8:
      return LVKW_KEY_KP_8;
    case XKB_KEY_KP_9:
      return LVKW_KEY_KP_9;
    case XKB_KEY_KP_Decimal:
      return LVKW_KEY_KP_DECIMAL;
    case XKB_KEY_KP_Divide:
      return LVKW_KEY_KP_DIVIDE;
    case XKB_KEY_KP_Multiply:
      return LVKW_KEY_KP_MULTIPLY;
    case XKB_KEY_KP_Subtract:
      return LVKW_KEY_KP_SUBTRACT;
    case XKB_KEY_KP_Add:
      return LVKW_KEY_KP_ADD;
    case XKB_KEY_KP_Enter:
      return LVKW_KEY_KP_ENTER;
    case XKB_KEY_KP_Equal:
      return LVKW_KEY_KP_EQUAL;
    case XKB_KEY_Shift_L:
      return LVKW_KEY_LEFT_SHIFT;
    case XKB_KEY_Control_L:
      return LVKW_KEY_LEFT_CONTROL;
    case XKB_KEY_Alt_L:
      return LVKW_KEY_LEFT_ALT;
    case XKB_KEY_Super_L:
      return LVKW_KEY_LEFT_META;
    case XKB_KEY_Shift_R:
      return LVKW_KEY_RIGHT_SHIFT;
    case XKB_KEY_Control_R:
      return LVKW_KEY_RIGHT_CONTROL;
    case XKB_KEY_Alt_R:
      return LVKW_KEY_RIGHT_ALT;
    case XKB_KEY_Super_R:
      return LVKW_KEY_RIGHT_META;
    case XKB_KEY_Menu:
      return LVKW_KEY_MENU;
    default:
      return LVKW_KEY_UNKNOWN;
  }
}

#include <linux/input-event-codes.h>

LVKW_Key lvkw_linux_translate_keycode(uint32_t keycode) {
  switch (keycode) {
    case KEY_SPACE: return LVKW_KEY_SPACE;
    case KEY_APOSTROPHE: return LVKW_KEY_APOSTROPHE;
    case KEY_COMMA: return LVKW_KEY_COMMA;
    case KEY_MINUS: return LVKW_KEY_MINUS;
    case KEY_DOT: return LVKW_KEY_PERIOD;
    case KEY_SLASH: return LVKW_KEY_SLASH;
    case KEY_0: return LVKW_KEY_0;
    case KEY_1: return LVKW_KEY_1;
    case KEY_2: return LVKW_KEY_2;
    case KEY_3: return LVKW_KEY_3;
    case KEY_4: return LVKW_KEY_4;
    case KEY_5: return LVKW_KEY_5;
    case KEY_6: return LVKW_KEY_6;
    case KEY_7: return LVKW_KEY_7;
    case KEY_8: return LVKW_KEY_8;
    case KEY_9: return LVKW_KEY_9;
    case KEY_SEMICOLON: return LVKW_KEY_SEMICOLON;
    case KEY_EQUAL: return LVKW_KEY_EQUAL;
    case KEY_A: return LVKW_KEY_A;
    case KEY_B: return LVKW_KEY_B;
    case KEY_C: return LVKW_KEY_C;
    case KEY_D: return LVKW_KEY_D;
    case KEY_E: return LVKW_KEY_E;
    case KEY_F: return LVKW_KEY_F;
    case KEY_G: return LVKW_KEY_G;
    case KEY_H: return LVKW_KEY_H;
    case KEY_I: return LVKW_KEY_I;
    case KEY_J: return LVKW_KEY_J;
    case KEY_K: return LVKW_KEY_K;
    case KEY_L: return LVKW_KEY_L;
    case KEY_M: return LVKW_KEY_M;
    case KEY_N: return LVKW_KEY_N;
    case KEY_O: return LVKW_KEY_O;
    case KEY_P: return LVKW_KEY_P;
    case KEY_Q: return LVKW_KEY_Q;
    case KEY_R: return LVKW_KEY_R;
    case KEY_S: return LVKW_KEY_S;
    case KEY_T: return LVKW_KEY_T;
    case KEY_U: return LVKW_KEY_U;
    case KEY_V: return LVKW_KEY_V;
    case KEY_W: return LVKW_KEY_W;
    case KEY_X: return LVKW_KEY_X;
    case KEY_Y: return LVKW_KEY_Y;
    case KEY_Z: return LVKW_KEY_Z;
    case KEY_LEFTBRACE: return LVKW_KEY_LEFT_BRACKET;
    case KEY_BACKSLASH: return LVKW_KEY_BACKSLASH;
    case KEY_RIGHTBRACE: return LVKW_KEY_RIGHT_BRACKET;
    case KEY_GRAVE: return LVKW_KEY_GRAVE_ACCENT;
    case KEY_ESC: return LVKW_KEY_ESCAPE;
    case KEY_ENTER: return LVKW_KEY_ENTER;
    case KEY_TAB: return LVKW_KEY_TAB;
    case KEY_BACKSPACE: return LVKW_KEY_BACKSPACE;
    case KEY_INSERT: return LVKW_KEY_INSERT;
    case KEY_DELETE: return LVKW_KEY_DELETE;
    case KEY_RIGHT: return LVKW_KEY_RIGHT;
    case KEY_LEFT: return LVKW_KEY_LEFT;
    case KEY_DOWN: return LVKW_KEY_DOWN;
    case KEY_UP: return LVKW_KEY_UP;
    case KEY_PAGEUP: return LVKW_KEY_PAGE_UP;
    case KEY_PAGEDOWN: return LVKW_KEY_PAGE_DOWN;
    case KEY_HOME: return LVKW_KEY_HOME;
    case KEY_END: return LVKW_KEY_END;
    case KEY_CAPSLOCK: return LVKW_KEY_CAPS_LOCK;
    case KEY_SCROLLLOCK: return LVKW_KEY_SCROLL_LOCK;
    case KEY_NUMLOCK: return LVKW_KEY_NUM_LOCK;
    case KEY_PRINT: return LVKW_KEY_PRINT_SCREEN;
    case KEY_PAUSE: return LVKW_KEY_PAUSE;
    case KEY_F1: return LVKW_KEY_F1;
    case KEY_F2: return LVKW_KEY_F2;
    case KEY_F3: return LVKW_KEY_F3;
    case KEY_F4: return LVKW_KEY_F4;
    case KEY_F5: return LVKW_KEY_F5;
    case KEY_F6: return LVKW_KEY_F6;
    case KEY_F7: return LVKW_KEY_F7;
    case KEY_F8: return LVKW_KEY_F8;
    case KEY_F9: return LVKW_KEY_F9;
    case KEY_F10: return LVKW_KEY_F10;
    case KEY_F11: return LVKW_KEY_F11;
    case KEY_F12: return LVKW_KEY_F12;
    case KEY_KP0: return LVKW_KEY_KP_0;
    case KEY_KP1: return LVKW_KEY_KP_1;
    case KEY_KP2: return LVKW_KEY_KP_2;
    case KEY_KP3: return LVKW_KEY_KP_3;
    case KEY_KP4: return LVKW_KEY_KP_4;
    case KEY_KP5: return LVKW_KEY_KP_5;
    case KEY_KP6: return LVKW_KEY_KP_6;
    case KEY_KP7: return LVKW_KEY_KP_7;
    case KEY_KP8: return LVKW_KEY_KP_8;
    case KEY_KP9: return LVKW_KEY_KP_9;
    case KEY_KPDOT: return LVKW_KEY_KP_DECIMAL;
    case KEY_KPSLASH: return LVKW_KEY_KP_DIVIDE;
    case KEY_KPASTERISK: return LVKW_KEY_KP_MULTIPLY;
    case KEY_KPMINUS: return LVKW_KEY_KP_SUBTRACT;
    case KEY_KPPLUS: return LVKW_KEY_KP_ADD;
    case KEY_KPENTER: return LVKW_KEY_KP_ENTER;
    case KEY_KPEQUAL: return LVKW_KEY_KP_EQUAL;
    case KEY_LEFTSHIFT: return LVKW_KEY_LEFT_SHIFT;
    case KEY_LEFTCTRL: return LVKW_KEY_LEFT_CONTROL;
    case KEY_LEFTALT: return LVKW_KEY_LEFT_ALT;
    case KEY_LEFTMETA: return LVKW_KEY_LEFT_META;
    case KEY_RIGHTSHIFT: return LVKW_KEY_RIGHT_SHIFT;
    case KEY_RIGHTCTRL: return LVKW_KEY_RIGHT_CONTROL;
    case KEY_RIGHTALT: return LVKW_KEY_RIGHT_ALT;
    case KEY_RIGHTMETA: return LVKW_KEY_RIGHT_META;
    case KEY_MENU: return LVKW_KEY_MENU;
    default: return LVKW_KEY_UNKNOWN;
  }
}
