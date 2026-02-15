// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <string.h>

#include "dlib/X11.h"
#include "dlib/Xcursor.h"
#include "dlib/Xss.h"
#include "lvkw/lvkw-core.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_x11_backend;
#endif

static Visual *_lvkw_x11_find_alpha_visual(LVKW_Context_X11 *ctx, Display *dpy, int screen,
                                           int *out_depth) {
  XVisualInfo vinfo_template;
  vinfo_template.screen = screen;
  vinfo_template.depth = 32;
  vinfo_template.class = TrueColor;

  int nitems;
  XVisualInfo *vinfo_list = lvkw_XGetVisualInfo(
      ctx, dpy, VisualScreenMask | VisualDepthMask | VisualClassMask, &vinfo_template, &nitems);

  Visual *visual = NULL;
  if (vinfo_list && nitems > 0) {
    visual = vinfo_list[0].visual;
    *out_depth = 32;
  }

  if (vinfo_list) lvkw_XFree(ctx, vinfo_list);
  return visual;
}

LVKW_Status lvkw_ctx_createWindow_X11(LVKW_Context *ctx_handle,
                                      const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  *out_window_handle = NULL;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_Window_X11 *window = (LVKW_Window_X11 *)_ctx_alloc(ctx, sizeof(LVKW_Window_X11));
  if (!window) return LVKW_ERROR;
  memset(window, 0, sizeof(*window));
#ifdef LVKW_INDIRECT_BACKEND
  window->base.prv.backend = &_lvkw_x11_backend;
#endif
  window->base.prv.ctx_base = &ctx->base;
  window->base.pub.userdata = create_info->userdata;
  window->size = create_info->attributes.logicalSize;
  window->cursor = create_info->attributes.cursor;
  window->transparent = create_info->transparent;

  uint32_t pixel_width = (uint32_t)((double)create_info->attributes.logicalSize.x * ctx->scale);
  uint32_t pixel_height = (uint32_t)((double)create_info->attributes.logicalSize.y * ctx->scale);

  int screen = DefaultScreen(ctx->display);
  Visual *visual = NULL;
  int depth = 0;

  if (window->transparent) {
    visual = _lvkw_x11_find_alpha_visual(ctx, ctx->display, screen, &depth);
  }

  if (!visual) {
    visual = DefaultVisual(ctx->display, screen);
    depth = DefaultDepth(ctx->display, screen);
  }

  window->colormap = lvkw_XCreateColormap(ctx, ctx->display, RootWindow(ctx->display, screen),
                                          visual, AllocNone);

  XSetWindowAttributes swa;
  swa.colormap = window->colormap;
  swa.background_pixel = 0;
  swa.border_pixel = 0;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
                   ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;

  window->window = lvkw_XCreateWindow(
      ctx, ctx->display, RootWindow(ctx->display, screen), 0, 0, pixel_width, pixel_height, 0, depth,
      InputOutput, visual, CWColormap | CWBackPixel | CWBorderPixel | CWEventMask, &swa);

  if (!window->window) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "XCreateWindow failed");
    lvkw_XFreeColormap(ctx, ctx->display, window->colormap);
    _ctx_free(ctx, window);
    return LVKW_ERROR;
  }

  lvkw_XStoreName(ctx, ctx->display, window->window,
                  create_info->attributes.title ? create_info->attributes.title : "Lvkw");

  if (create_info->app_id) {
    XClassHint *hint = lvkw_XAllocClassHint(ctx);
    if (hint) {
      hint->res_name = (char *)create_info->app_id;
      hint->res_class = (char *)create_info->app_id;
      lvkw_XSetClassHint(ctx, ctx->display, window->window, hint);
      lvkw_XFree(ctx, hint);
    }
  }

  Atom protocols[] = {ctx->wm_delete_window, ctx->wm_take_focus, ctx->net_wm_ping};
  lvkw_XSetWMProtocols(ctx, ctx->display, window->window, protocols, 3);

  lvkw_XSaveContext(ctx, ctx->display, window->window, ctx->window_context, (XPointer)window);

  lvkw_XMapWindow(ctx, ctx->display, window->window);

  // Add to context window list
  _lvkw_window_list_add(&ctx->base, &window->base);

  window->base.pub.flags |= LVKW_WND_STATE_READY;

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_destroy_X11(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  // Remove from window list
  _lvkw_window_list_remove(&ctx->base, &window->base);

  if (ctx->locked_window == window) {
    ctx->locked_window = NULL;
    lvkw_XUngrabPointer(ctx, ctx->display, CurrentTime);
  }

  lvkw_event_queue_remove_window_events(&ctx->base.prv.event_queue, window_handle);

  lvkw_XDeleteContext(ctx, ctx->display, window->window, ctx->window_context);
  lvkw_XDestroyWindow(ctx, ctx->display, window->window);
  lvkw_XFreeColormap(ctx, ctx->display, window->colormap);
  _ctx_free(ctx, window);
  return LVKW_SUCCESS;
}

// Vulkan forward declarations
typedef enum VkResult {
  VK_SUCCESS = 0,
} VkResult;

#define VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR 1000004000
#define VK_NULL_HANDLE 0

typedef struct VkXlibSurfaceCreateInfoKHR {
  int sType;
  const void *pNext;
  uint32_t flags;
  Display *dpy;
  Window window;
} VkXlibSurfaceCreateInfoKHR;

typedef void (*PFN_vkVoidFunction)(void);
typedef PFN_vkVoidFunction (*PFN_vkGetInstanceProcAddr)(VkInstance instance, const char *pName);
typedef VkResult (*PFN_vkCreateXlibSurfaceKHR)(VkInstance instance,
                                               const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                               const void *pAllocator, VkSurfaceKHR *pSurface);

extern __attribute__((weak)) PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance,
                                                                      const char *pName);

LVKW_Status lvkw_wnd_createVkSurface_X11(LVKW_Window *window_handle, VkInstance instance,

                                         VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  *out_surface = VK_NULL_HANDLE;

  const LVKW_Window_X11 *window = (const LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  PFN_vkGetInstanceProcAddr vk_loader = (PFN_vkGetInstanceProcAddr)ctx->base.prv.vk_loader;

  // If no manual loader is provided, try to use the linked symbol (if available)
  if (!vk_loader) {
    vk_loader = vkGetInstanceProcAddr;
  }

  if (!vk_loader) {
    LVKW_REPORT_WIND_DIAGNOSTIC(
        &window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE,
        "No Vulkan loader available. Provide vk_loader in context tuning or link against "
        "Vulkan.");
    return LVKW_ERROR;
  }

  PFN_vkCreateXlibSurfaceKHR fpCreateXlibSurfaceKHR =
      (PFN_vkCreateXlibSurfaceKHR)vk_loader(instance, "vkCreateXlibSurfaceKHR");

  if (!fpCreateXlibSurfaceKHR) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE,
                                "vkCreateXlibSurfaceKHR not found");

    return LVKW_ERROR;
  }

  VkXlibSurfaceCreateInfoKHR createInfo = {

      .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,

      .dpy = ctx->display,

      .window = window->window,

  };

  if (fpCreateXlibSurfaceKHR(instance, &createInfo, NULL, out_surface) != VK_SUCCESS) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE,
                                "vkCreateXlibSurfaceKHR failure");

    return LVKW_ERROR;
  }

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getGeometry_X11(LVKW_Window *window_handle,
                                     LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  const LVKW_Window_X11 *window = (const LVKW_Window_X11 *)window_handle;

  const LVKW_Context_X11 *ctx = (const LVKW_Context_X11 *)window->base.prv.ctx_base;

  out_geometry->logicalSize = window->size;
  out_geometry->pixelSize.x = (int32_t)((double)window->size.x * ctx->scale);
  out_geometry->pixelSize.y = (int32_t)((double)window->size.y * ctx->scale);

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_X11(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode);
static LVKW_Status _lvkw_wnd_setCursor_X11(LVKW_Window *window_handle, LVKW_Cursor *cursor);

LVKW_Status lvkw_wnd_update_X11(LVKW_Window *window_handle, uint32_t field_mask,
                                const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  if (field_mask & LVKW_WND_ATTR_TITLE) {
    lvkw_XStoreName(ctx, ctx->display, window->window, attributes->title ? attributes->title : "Lvkw");
  }

  if (field_mask & LVKW_WND_ATTR_LOGICAL_SIZE) {
    window->size = attributes->logicalSize;

    uint32_t pixel_width = (uint32_t)((double)attributes->logicalSize.x * ctx->scale);

    uint32_t pixel_height = (uint32_t)((double)attributes->logicalSize.y * ctx->scale);
    lvkw_XResizeWindow(ctx, ctx->display, window->window, pixel_width, pixel_height);
  }

  if (field_mask & LVKW_WND_ATTR_FULLSCREEN) {
    _lvkw_wnd_setFullscreen_X11(window_handle, attributes->fullscreen);
  }

  if (field_mask & LVKW_WND_ATTR_CURSOR_MODE) {
    _lvkw_wnd_setCursorMode_X11(window_handle, attributes->cursor_mode);
  }

  if (field_mask & LVKW_WND_ATTR_CURSOR) {
    _lvkw_wnd_setCursor_X11(window_handle, attributes->cursor);
  }

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_X11(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  XEvent ev;

  memset(&ev, 0, sizeof(ev));

  ev.type = ClientMessage;

  ev.xclient.window = window->window;

  ev.xclient.message_type = ctx->net_wm_state;

  ev.xclient.format = 32;

  ev.xclient.data.l[0] = enabled ? 1 : 0;  // _NET_WM_STATE_ADD or _NET_WM_STATE_REMOVE

  ev.xclient.data.l[1] = (long)ctx->net_wm_state_fullscreen;

  ev.xclient.data.l[2] = 0;

  ev.xclient.data.l[3] = 1;  // source indication

  lvkw_XSendEvent(ctx, ctx->display, DefaultRootWindow(ctx->display), False,
                  SubstructureNotifyMask | StructureNotifyMask, &ev);

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  if (window->cursor_mode == mode) return LVKW_SUCCESS;

  Display *dpy = ctx->display;

  if (mode == LVKW_CURSOR_LOCKED) {
    uint32_t phys_w = (uint32_t)((double)window->size.x * ctx->scale);
    uint32_t phys_h = (uint32_t)((double)window->size.y * ctx->scale);

    lvkw_XGrabPointer(ctx, dpy, window->window, True,
                      ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
                      GrabModeAsync, window->window, ctx->hidden_cursor, CurrentTime);

    lvkw_XWarpPointer(ctx, dpy, None, window->window, 0, 0, 0, 0, (int)(phys_w / 2),
                      (int)(phys_h / 2));

    window->last_x = (double)(phys_w / 2.0);

    window->last_y = (double)(phys_h / 2.0);

    ctx->locked_window = window;
  }

  else {
    lvkw_XUngrabPointer(ctx, dpy, CurrentTime);

    lvkw_XUndefineCursor(ctx, dpy, window->window);

    if (ctx->locked_window == window) ctx->locked_window = NULL;
  }

  window->cursor_mode = mode;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursor_X11(LVKW_Window *window_handle, LVKW_Cursor *cursor) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  window->cursor = cursor;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_requestFocus_X11(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  XEvent ev;

  memset(&ev, 0, sizeof(ev));

  ev.type = ClientMessage;

  ev.xclient.window = window->window;

  ev.xclient.message_type = ctx->net_active_window;

  ev.xclient.format = 32;

  ev.xclient.data.l[0] = 1;  // source indication: application

  ev.xclient.data.l[1] = CurrentTime;

  ev.xclient.data.l[2] = 0;

  lvkw_XSendEvent(ctx, ctx->display, DefaultRootWindow(ctx->display), False,
                  SubstructureNotifyMask | SubstructureRedirectMask, &ev);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setClipboardText_X11(LVKW_Window *window, const char *text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on X11");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardText_X11(LVKW_Window *window, const char **out_text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on X11");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_setClipboardData_X11(LVKW_Window *window, const LVKW_ClipboardData *data,
                                          uint32_t count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on X11");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardData_X11(LVKW_Window *window, const char *mime_type,
                                          const void **out_data, size_t *out_size) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on X11");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_X11(LVKW_Window *window, const char ***out_mime_types,
                                               uint32_t *count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on X11");
  return LVKW_ERROR;
}

LVKW_Cursor *lvkw_ctx_getStandardCursor_X11(LVKW_Context *ctx_handle, LVKW_CursorShape shape) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  (void)ctx;
  (void)shape;

  return NULL;
}

LVKW_Status lvkw_ctx_createCursor_X11(LVKW_Context *ctx_handle, const LVKW_CursorCreateInfo *create_info,
                                      LVKW_Cursor **out_cursor) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  (void)ctx;
  (void)create_info;

  *out_cursor = NULL;

  return LVKW_ERROR;
}

LVKW_Status lvkw_cursor_destroy_X11(LVKW_Cursor *cursor) {
  if (!cursor) return LVKW_SUCCESS;
  LVKW_Cursor_Base *cursor_base = (LVKW_Cursor_Base *)cursor;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)cursor_base->prv.ctx_base;
  (void)ctx;

  return LVKW_SUCCESS;
}
