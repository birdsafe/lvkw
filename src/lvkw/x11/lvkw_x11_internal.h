#ifndef LVKW_X11_INTERNAL_H_INCLUDED
#define LVKW_X11_INTERNAL_H_INCLUDED

#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <vulkan/vulkan.h>

#include "dlib/xkbcommon.h"
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"
#include "lvkw_linux_internal.h"

#define _ctx_alloc(ctx, size) lvkw_context_alloc(&(ctx)->base, size)
#define _ctx_free(ctx, ptr) lvkw_context_free(&(ctx)->base, ptr)

typedef struct LVKW_Window_X11 LVKW_Window_X11;

typedef struct LVKW_Context_X11 {
  LVKW_Context_Base base;
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
  LVKW_Size size;
  LVKW_CursorMode cursor_mode;
  double last_x, last_y;
  bool transparent;
} LVKW_Window_X11;

LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
void lvkw_ctx_destroy_X11(LVKW_Context *handle);
const char *const *lvkw_ctx_getVkExtensions_X11(LVKW_Context *ctx, uint32_t *count);
LVKW_Status lvkw_ctx_pollEvents_X11(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                    void *userdata);
LVKW_Status lvkw_ctx_waitEvents_X11(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_update_X11(LVKW_Context *ctx, uint32_t field_mask,
                                                const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_X11(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors, uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_X11(LVKW_Context *ctx, LVKW_MonitorId monitor,
                                         LVKW_VideoMode *out_modes, uint32_t *count);
LVKW_Status lvkw_ctx_createWindow_X11(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window);
void lvkw_wnd_destroy_X11(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_X11(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_X11(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_X11(LVKW_Window *window, uint32_t field_mask,
                                                const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_X11(LVKW_Window *window_handle);

void _lvkw_x11_check_error(LVKW_Context_X11 *ctx);
LVKW_MouseButton _lvkw_x11_translate_button(unsigned int button);

#endif