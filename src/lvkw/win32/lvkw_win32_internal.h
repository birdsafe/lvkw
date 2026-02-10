#ifndef LVKW_WIN32_INTERNAL_H_INCLUDED
#define LVKW_WIN32_INTERNAL_H_INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

typedef struct LVKW_Window_Win32 LVKW_Window_Win32;

typedef struct LVKW_Context_Win32 {
  LVKW_Context_Base base;
  HINSTANCE hInstance;
  ATOM window_class_atom;

  // Event polling state
  LVKW_EventCallback current_event_callback;
  void *current_event_userdata;
  LVKW_EventType current_event_mask;
  bool is_polling;

  // Time tracking for idle events
  uint32_t last_event_time;
  uint32_t idle_timeout_ms;
  bool inhibit_idle;
} LVKW_Context_Win32;

typedef struct LVKW_Window_Win32 {
  LVKW_Window_Base base;

  HWND hwnd;

  bool is_fullscreen;
  RECT stored_rect;  // Used to restore window position/size when exiting
                     // fullscreen
  DWORD stored_style;
  DWORD stored_ex_style;

  LVKW_CursorMode cursor_mode;
  LVKW_CursorShape cursor_shape;
  HCURSOR current_hcursor;
  bool cursor_in_client_area;

  double last_x, last_y;
  bool has_last_pos;

  LVKW_Size size;
  bool transparent;
} LVKW_Window_Win32;

// Internal function prototypes
LRESULT CALLBACK _lvkw_win32_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper to update cursor clipping based on mode and focus
void _lvkw_win32_update_cursor_clip(LVKW_Window_Win32 *window);

// Helper to convert Win32 virtual key codes to LVKW_Key
LVKW_Key _lvkw_win32_translate_key(WPARAM wParam, LPARAM lParam);

// Helper to get modifiers
LVKW_ModifierFlags _lvkw_win32_get_modifiers(void);

LVKW_Status lvkw_ctx_create_Win32(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
void lvkw_ctx_destroy_Win32(LVKW_Context *handle);
void lvkw_ctx_getVkExtensions_Win32(LVKW_Context *ctx, uint32_t *count,
                                                     const char **out_extensions);
LVKW_Status lvkw_ctx_pollEvents_Win32(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                  LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_waitEvents_Win32(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                                  LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_updateAttributes_Win32(LVKW_Context *ctx, uint32_t field_mask,
                                                 const LVKW_ContextAttributes *attributes);

LVKW_Status lvkw_ctx_createWindow_Win32(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                            LVKW_Window **out_window);
void lvkw_wnd_destroy_Win32(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_Win32(LVKW_Window *window, VkInstance instance,
                                                    VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getFramebufferSize_Win32(LVKW_Window *window, LVKW_Size *out_size);
LVKW_Status lvkw_wnd_updateAttributes_Win32(LVKW_Window *window, uint32_t field_mask,
                                                 const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_Win32(LVKW_Window *window);

#endif  // LVKW_WIN32_INTERNAL_H_INCLUDED