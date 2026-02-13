#ifndef LVKW_INPUT_H_INCLUDED
#define LVKW_INPUT_H_INCLUDED

#include "lvkw-core.h"

/**
 * @file lvkw-input.h
 * @brief Keyboard, mouse, and analog input definitions.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Tracks whether a button is currently pressed or released. */
typedef enum LVKW_ButtonState {
  LVKW_BUTTON_STATE_RELEASED = 0,
  LVKW_BUTTON_STATE_PRESSED = 1,
} LVKW_ButtonState;

/**
 * @brief Identifiers for physical keyboard keys.
 * @note These represent physical key locations (scancodes mapped to a stable set of identifiers)
 * rather than the character produced. They generally match standard US QWERTY labels.
 * See lvkw_keys.inc.h for the full list.
 */
typedef enum LVKW_Key {
  LVKW_KEY_UNKNOWN = 0,
  LVKW_KEY_SPACE = 32,
  LVKW_KEY_APOSTROPHE = 39,
  LVKW_KEY_COMMA = 44,
  LVKW_KEY_MINUS = 45,
  LVKW_KEY_PERIOD = 46,
  LVKW_KEY_SLASH = 47,
  LVKW_KEY_0 = 48,
  LVKW_KEY_1 = 49,
  LVKW_KEY_2 = 50,
  LVKW_KEY_3 = 51,
  LVKW_KEY_4 = 52,
  LVKW_KEY_5 = 53,
  LVKW_KEY_6 = 54,
  LVKW_KEY_7 = 55,
  LVKW_KEY_8 = 56,
  LVKW_KEY_9 = 57,
  LVKW_KEY_SEMICOLON = 59,
  LVKW_KEY_EQUAL = 61,
  LVKW_KEY_A = 65,
  LVKW_KEY_B = 66,
  LVKW_KEY_C = 67,
  LVKW_KEY_D = 68,
  LVKW_KEY_E = 69,
  LVKW_KEY_F = 70,
  LVKW_KEY_G = 71,
  LVKW_KEY_H = 72,
  LVKW_KEY_I = 73,
  LVKW_KEY_J = 74,
  LVKW_KEY_K = 75,
  LVKW_KEY_L = 76,
  LVKW_KEY_M = 77,
  LVKW_KEY_N = 78,
  LVKW_KEY_O = 79,
  LVKW_KEY_P = 80,
  LVKW_KEY_Q = 81,
  LVKW_KEY_R = 82,
  LVKW_KEY_S = 83,
  LVKW_KEY_T = 84,
  LVKW_KEY_U = 85,
  LVKW_KEY_V = 86,
  LVKW_KEY_W = 87,
  LVKW_KEY_X = 88,
  LVKW_KEY_Y = 89,
  LVKW_KEY_Z = 90,
  LVKW_KEY_LEFT_BRACKET = 91,
  LVKW_KEY_BACKSLASH = 92,
  LVKW_KEY_RIGHT_BRACKET = 93,
  LVKW_KEY_GRAVE_ACCENT = 96,
  LVKW_KEY_ESCAPE = 256,
  LVKW_KEY_ENTER = 257,
  LVKW_KEY_TAB = 258,
  LVKW_KEY_BACKSPACE = 259,
  LVKW_KEY_INSERT = 260,
  LVKW_KEY_DELETE = 261,
  LVKW_KEY_RIGHT = 262,
  LVKW_KEY_LEFT = 263,
  LVKW_KEY_DOWN = 264,
  LVKW_KEY_UP = 265,
  LVKW_KEY_PAGE_UP = 266,
  LVKW_KEY_PAGE_DOWN = 267,
  LVKW_KEY_HOME = 268,
  LVKW_KEY_END = 269,
  LVKW_KEY_CAPS_LOCK = 280,
  LVKW_KEY_SCROLL_LOCK = 281,
  LVKW_KEY_NUM_LOCK = 282,
  LVKW_KEY_PRINT_SCREEN = 283,
  LVKW_KEY_PAUSE = 284,
  LVKW_KEY_F1 = 290,
  LVKW_KEY_F2 = 291,
  LVKW_KEY_F3 = 292,
  LVKW_KEY_F4 = 293,
  LVKW_KEY_F5 = 294,
  LVKW_KEY_F6 = 295,
  LVKW_KEY_F7 = 296,
  LVKW_KEY_F8 = 297,
  LVKW_KEY_F9 = 298,
  LVKW_KEY_F10 = 299,
  LVKW_KEY_F11 = 300,
  LVKW_KEY_F12 = 301,
  LVKW_KEY_KP_0 = 320,
  LVKW_KEY_KP_1 = 321,
  LVKW_KEY_KP_2 = 322,
  LVKW_KEY_KP_3 = 323,
  LVKW_KEY_KP_4 = 324,
  LVKW_KEY_KP_5 = 325,
  LVKW_KEY_KP_6 = 326,
  LVKW_KEY_KP_7 = 327,
  LVKW_KEY_KP_8 = 328,
  LVKW_KEY_KP_9 = 329,
  LVKW_KEY_KP_DECIMAL = 330,
  LVKW_KEY_KP_DIVIDE = 331,
  LVKW_KEY_KP_MULTIPLY = 332,
  LVKW_KEY_KP_SUBTRACT = 333,
  LVKW_KEY_KP_ADD = 334,
  LVKW_KEY_KP_ENTER = 335,
  LVKW_KEY_KP_EQUAL = 336,
  LVKW_KEY_LEFT_SHIFT = 340,
  LVKW_KEY_LEFT_CONTROL = 341,
  LVKW_KEY_LEFT_ALT = 342,
  LVKW_KEY_LEFT_SUPER = 343,
  LVKW_KEY_RIGHT_SHIFT = 344,
  LVKW_KEY_RIGHT_CONTROL = 345,
  LVKW_KEY_RIGHT_ALT = 346,
  LVKW_KEY_RIGHT_SUPER = 347,
  LVKW_KEY_MENU = 348,

} LVKW_Key;

/** @brief Bitmask for modifier keys active during an event. */
typedef enum LVKW_ModifierFlags : uint8_t{
  LVKW_MODIFIER_SHIFT = 1 << 0,
  LVKW_MODIFIER_CONTROL = 1 << 1,
  LVKW_MODIFIER_ALT = 1 << 2,
  LVKW_MODIFIER_SUPER = 1 << 3,    ///< Windows key on Windows, Command key on macOS, Super on Linux.
  LVKW_MODIFIER_CAPS_LOCK = 1 << 4,
  LVKW_MODIFIER_NUM_LOCK = 1 << 5,
} LVKW_ModifierFlags;

/** @brief Identifiers for mouse buttons. */
typedef enum LVKW_MouseButton {
  LVKW_MOUSE_BUTTON_LEFT = 0,
  LVKW_MOUSE_BUTTON_RIGHT = 1,
  LVKW_MOUSE_BUTTON_MIDDLE = 2,
  LVKW_MOUSE_BUTTON_4 = 3,
  LVKW_MOUSE_BUTTON_5 = 4,
  LVKW_MOUSE_BUTTON_6 = 5,
  LVKW_MOUSE_BUTTON_7 = 6,
  LVKW_MOUSE_BUTTON_8 = 7,
} LVKW_MouseButton;

/** @brief State for a single continuous input axis (e.g., joystick or trigger). */
typedef struct LVKW_AnalogInputState {
  LVKW_real_t value;  ///< Normalized value: [-1, 1] for sticks, [0, 1] for triggers.
} LVKW_AnalogInputState;

#ifdef __cplusplus
}
#endif

#endif  // LVKW_INPUT_H_INCLUDED
