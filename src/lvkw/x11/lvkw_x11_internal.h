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
} LVKW_Window_X11;

LVKW_Status lvkw_createContext_X11(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
void lvkw_destroyContext_X11(LVKW_Context *handle);
void *lvkw_context_getUserData_X11(LVKW_Context *ctx);
void lvkw_context_getVulkanInstanceExtensions_X11(LVKW_Context *ctx, uint32_t *count,
                                                   const char **out_extensions);
LVKW_Status lvkw_context_pollEvents_X11(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                                LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_context_waitEvents_X11(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                                LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_context_setIdleTimeout_X11(LVKW_Context *ctx, uint32_t timeout_ms);

void _lvkw_x11_check_error(LVKW_Context_X11 *ctx);

LVKW_Status lvkw_context_createWindow_X11(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                          LVKW_Window **out_window);
void lvkw_destroyWindow_X11(LVKW_Window *handle);
LVKW_Status lvkw_window_createVkSurface_X11(LVKW_Window *window_handle, VkInstance instance,
                                                  VkSurfaceKHR *out_surface);
LVKW_Status lvkw_window_getFramebufferSize_X11(LVKW_Window *window_handle, LVKW_Size *out_size);
void *lvkw_window_getUserData_X11(LVKW_Window *window);
LVKW_Status lvkw_window_setFullscreen_X11(LVKW_Window *window_handle, bool enabled);
LVKW_Status lvkw_window_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode);
LVKW_Status lvkw_window_setCursorShape_X11(LVKW_Window *window_handle, LVKW_CursorShape shape);
LVKW_Status lvkw_window_requestFocus_X11(LVKW_Window *window_handle);

LVKW_MouseButton _lvkw_x11_translate_button(unsigned int button);

#endif
