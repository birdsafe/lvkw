// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_INTERNAL_H_INCLUDED
#define LVKW_X11_INTERNAL_H_INCLUDED

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "dlib/xkbcommon.h"
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"
#include "lvkw_linux_internal.h"

#define _ctx_alloc(ctx, size) lvkw_context_alloc(&(ctx)->base, size)
#define _ctx_free(ctx, ptr) lvkw_context_free(&(ctx)->base, ptr)

typedef struct LVKW_Window_X11 LVKW_Window_X11;

typedef struct LVKW_Context_X11 {
  LVKW_Context_Base base;

#ifdef LVKW_ENABLE_CONTROLLER
  LVKW_ControllerContext_Linux controller;
#endif

  Display *display;
  Atom wm_protocols;
  Atom wm_delete_window;
  XContext window_context;
  Cursor hidden_cursor;
  Atom net_wm_state;
  Atom net_wm_state_fullscreen;
  Atom net_active_window;
  Atom net_wm_ping;
  Atom wm_take_focus;
  uint32_t idle_timeout_ms;
  bool is_idle;
  int xi_opcode;
  double scale;
  LVKW_Window_X11 *locked_window;
  bool inhibit_idle;

  struct {
    struct xkb_context *ctx;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
  } xkb;

  // Event queue
  LVKW_EventQueue event_queue;
} LVKW_Context_X11;

typedef struct LVKW_Window_X11 {
  LVKW_Window_Base base;
  Window window;
  Colormap colormap;
  LVKW_LogicalVec size;
  LVKW_CursorMode cursor_mode;
  LVKW_Cursor *cursor;
  double last_x, last_y;
  bool transparent;
} LVKW_Window_X11;

LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info,
                                LVKW_Context **out_context);
LVKW_Status lvkw_ctx_destroy_X11(LVKW_Context *handle);
LVKW_Status lvkw_ctx_getVkExtensions_X11(LVKW_Context *ctx, uint32_t *count,
                                         const char *const **out_extensions);
LVKW_Status lvkw_ctx_pollEvents_X11(LVKW_Context *ctx, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_waitEvents_X11(LVKW_Context *ctx, uint32_t timeout_ms,
                                    LVKW_EventType event_mask, LVKW_EventCallback callback,
                                    void *userdata);
LVKW_Status lvkw_ctx_update_X11(LVKW_Context *ctx, uint32_t field_mask,
                                const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_X11(LVKW_Context *ctx, LVKW_Monitor **out_monitors,
                                     uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_X11(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                         LVKW_VideoMode *out_modes, uint32_t *count);
LVKW_Status lvkw_ctx_getTelemetry_X11(LVKW_Context *ctx, LVKW_TelemetryCategory category,
                                      void *out_data, bool reset);
LVKW_Status lvkw_ctx_createWindow_X11(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window);
LVKW_Status lvkw_wnd_destroy_X11(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_X11(LVKW_Window *window, VkInstance instance,
                                         VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_X11(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_X11(LVKW_Window *window, uint32_t field_mask,
                                const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_X11(LVKW_Window *window_handle);
LVKW_Status lvkw_wnd_setClipboardText_X11(LVKW_Window *window, const char *text);
LVKW_Status lvkw_wnd_getClipboardText_X11(LVKW_Window *window, const char **out_text);
LVKW_Status lvkw_wnd_setClipboardData_X11(LVKW_Window *window, const LVKW_ClipboardData *data,
                                          uint32_t count);
LVKW_Status lvkw_wnd_getClipboardData_X11(LVKW_Window *window, const char *mime_type,
                                          const void **out_data, size_t *out_size);
LVKW_Status lvkw_wnd_getClipboardMimeTypes_X11(LVKW_Window *window, const char ***out_mime_types,
                                               uint32_t *count);

LVKW_Cursor *lvkw_ctx_getStandardCursor_X11(LVKW_Context *ctx, LVKW_CursorShape shape);
LVKW_Status lvkw_ctx_createCursor_X11(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                      LVKW_Cursor **out_cursor);
LVKW_Status lvkw_cursor_destroy_X11(LVKW_Cursor *cursor);

void _lvkw_x11_check_error(LVKW_Context_X11 *ctx);
LVKW_MouseButton _lvkw_x11_translate_button(unsigned int button);
void _lvkw_x11_push_event(LVKW_Context_X11 *ctx, LVKW_EventType type, LVKW_Window *window,
                          const LVKW_Event *evt);

#endif