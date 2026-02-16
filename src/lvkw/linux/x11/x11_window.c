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

static LVKW_Status _lvkw_wnd_setFullscreen_X11(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode);

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
  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_Window_X11 *window = (LVKW_Window_X11 *)_ctx_alloc(ctx, sizeof(LVKW_Window_X11));
  if (!window) return LVKW_ERROR;
  memset(window, 0, sizeof(*window));
#ifdef LVKW_INDIRECT_BACKEND
  window->base.prv.backend = &_lvkw_x11_backend;
#endif
  window->base.prv.ctx_base = &ctx->linux_base.base;
  window->base.pub.userdata = create_info->userdata;
  window->size = create_info->attributes.logicalSize;
  window->cursor = create_info->attributes.cursor;
  window->transparent = create_info->transparent;

  uint32_t pixel_width = (uint32_t)((LVKW_Scalar)create_info->attributes.logicalSize.x * ctx->scale);
  uint32_t pixel_height = (uint32_t)((LVKW_Scalar)create_info->attributes.logicalSize.y * ctx->scale);

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
                   ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | EnterWindowMask |
                   LeaveWindowMask;

  window->window = lvkw_XCreateWindow(
      ctx, ctx->display, RootWindow(ctx->display, screen), 0, 0, pixel_width, pixel_height, 0, depth,
      InputOutput, visual, CWColormap | CWBackPixel | CWBorderPixel | CWEventMask, &swa);

  if (!window->window) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
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

  _lvkw_wnd_setCursor_X11((LVKW_Window *)window, window->cursor);

  lvkw_XMapWindow(ctx, ctx->display, window->window);

  // Add to context window list
  _lvkw_window_list_add(&ctx->linux_base.base, &window->base);

  window->base.pub.flags |= LVKW_WND_STATE_READY;
  {
    LVKW_Event ev = {0};
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_READY,
                          (LVKW_Window *)window, &ev);
  }

  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_destroy_X11(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  // Remove from window list
  _lvkw_window_list_remove(&ctx->linux_base.base, &window->base);

  if (ctx->locked_window == window) {
    ctx->locked_window = NULL;
    lvkw_XUngrabPointer(ctx, ctx->display, CurrentTime);
  }

  lvkw_event_queue_remove_window_events(&ctx->linux_base.base.prv.event_queue, window_handle);

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

  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  PFN_vkGetInstanceProcAddr vk_loader = (PFN_vkGetInstanceProcAddr)ctx->linux_base.base.prv.vk_loader;

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

  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getGeometry_X11(LVKW_Window *window_handle,
                                     LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  const LVKW_Window_X11 *window = (const LVKW_Window_X11 *)window_handle;

  const LVKW_Context_X11 *ctx = (const LVKW_Context_X11 *)window->base.prv.ctx_base;

  out_geometry->logicalSize = window->size;
  out_geometry->pixelSize.x = (int32_t)((LVKW_Scalar)window->size.x * ctx->scale);
  out_geometry->pixelSize.y = (int32_t)((LVKW_Scalar)window->size.y * ctx->scale);

  return LVKW_SUCCESS;
}

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

    uint32_t pixel_width = (uint32_t)((LVKW_Scalar)attributes->logicalSize.x * ctx->scale);

    uint32_t pixel_height = (uint32_t)((LVKW_Scalar)attributes->logicalSize.y * ctx->scale);
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
  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_X11(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

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
  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WND_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  if (window->cursor_mode == mode) return LVKW_SUCCESS;

  Display *dpy = ctx->display;

  if (mode == LVKW_CURSOR_LOCKED) {
    uint32_t phys_w = (uint32_t)((LVKW_Scalar)window->size.x * ctx->scale);
    uint32_t phys_h = (uint32_t)((LVKW_Scalar)window->size.y * ctx->scale);

    lvkw_XGrabPointer(ctx, dpy, window->window, True,
                      ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
                      GrabModeAsync, window->window, ctx->hidden_cursor, CurrentTime);

    lvkw_XWarpPointer(ctx, dpy, None, window->window, 0, 0, 0, 0, (int)(phys_w / 2),
                      (int)(phys_h / 2));

    window->last_x = (LVKW_Scalar)(phys_w / 2.0);

    window->last_y = (LVKW_Scalar)(phys_h / 2.0);

    ctx->locked_window = window;
  }

  else {
    lvkw_XUngrabPointer(ctx, dpy, CurrentTime);

    _lvkw_wnd_setCursor_X11(window_handle, window->cursor);

    if (ctx->locked_window == window) ctx->locked_window = NULL;
  }

  window->cursor_mode = mode;

  return LVKW_SUCCESS;
}

LVKW_Status _lvkw_wnd_setCursor_X11(LVKW_Window *window_handle, LVKW_Cursor *cursor_handle) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;
  LVKW_Cursor_X11 *cursor = (LVKW_Cursor_X11 *)cursor_handle;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_UNKNOWN, "Setting X11 cursor");

  window->cursor = cursor_handle;

  if (window->cursor_mode != LVKW_CURSOR_LOCKED) {
    if (cursor) {
      lvkw_XDefineCursor(ctx, ctx->display, window->window, cursor->cursor);
    } else {
      lvkw_XUndefineCursor(ctx, ctx->display, window->window);
    }
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_requestFocus_X11(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->linux_base.base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
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

LVKW_Status lvkw_ctx_getStandardCursor_X11(LVKW_Context *ctx_handle, LVKW_CursorShape shape,
                                           LVKW_Cursor **out_cursor) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  *out_cursor = NULL;

  if (shape < 1 || shape > 12) return LVKW_ERROR_INVALID_USAGE;

  LVKW_Cursor_X11 *cursor = (LVKW_Cursor_X11*)&ctx->linux_base.base.prv.standard_cursors[shape];

  if (cursor->cursor == None) {
    const char *name = NULL;
    unsigned int fallback_shape = 0;

    switch (shape) {
      case LVKW_CURSOR_SHAPE_DEFAULT:
        name = "left_ptr";
        fallback_shape = 68;  // XC_left_ptr
        break;
      case LVKW_CURSOR_SHAPE_HELP:
        name = "help";
        fallback_shape = 92;  // XC_question_arrow
        break;
      case LVKW_CURSOR_SHAPE_HAND:
        name = "hand2";
        fallback_shape = 58;  // XC_hand2
        break;
      case LVKW_CURSOR_SHAPE_WAIT:
        name = "watch";
        fallback_shape = 150;  // XC_watch
        break;
      case LVKW_CURSOR_SHAPE_CROSSHAIR:
        name = "crosshair";
        fallback_shape = 34;  // XC_crosshair
        break;
      case LVKW_CURSOR_SHAPE_TEXT:
        name = "xterm";
        fallback_shape = 152;  // XC_xterm
        break;
      case LVKW_CURSOR_SHAPE_MOVE:
        name = "fleur";
        fallback_shape = 52;  // XC_fleur
        break;
      case LVKW_CURSOR_SHAPE_NOT_ALLOWED:
        name = "crossed_circle";
        fallback_shape = 0;  // XC_X_cursor (for now still 0 until I confirm value)
        break;
      case LVKW_CURSOR_SHAPE_EW_RESIZE:
        name = "sb_h_double_arrow";
        fallback_shape = 108;  // XC_sb_h_double_arrow
        break;
      case LVKW_CURSOR_SHAPE_NS_RESIZE:
        name = "sb_v_double_arrow";
        fallback_shape = 116;  // XC_sb_v_double_arrow
        break;
      case LVKW_CURSOR_SHAPE_NESW_RESIZE:
        name = "size_bdiag";
        fallback_shape = 12;  // XC_bottom_left_corner
        break;
      case LVKW_CURSOR_SHAPE_NWSE_RESIZE:
        name = "size_fdiag";
        fallback_shape = 14;  // XC_bottom_right_corner
        break;
    }

    if (name && ctx->dlib.xcursor.base.available) {
      cursor->cursor = lvkw_XcursorLibraryLoadCursor(ctx, ctx->display, name);
    }

    if (cursor->cursor == None && fallback_shape != 0) {
      cursor->cursor = lvkw_XCreateFontCursor(ctx, ctx->display, fallback_shape);
    }

    if (cursor->cursor == None) {
      return LVKW_ERROR;
    }
  }

  *out_cursor = (LVKW_Cursor *)cursor;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_createCursor_X11(LVKW_Context *ctx_handle, const LVKW_CursorCreateInfo *create_info,
                                      LVKW_Cursor **out_cursor) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  *out_cursor = NULL;

  if (!ctx->dlib.xcursor.base.available) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                               "Custom cursors require libXcursor");
    return LVKW_ERROR;
  }

  XcursorImage *image = lvkw_XcursorImageCreate(ctx, create_info->size.x, create_info->size.y);
  if (!image) return LVKW_ERROR;

  image->xhot = create_info->hotSpot.x;
  image->yhot = create_info->hotSpot.y;
  image->delay = 0;

  memcpy(image->pixels, create_info->pixels,
         (size_t)create_info->size.x * (size_t)create_info->size.y * sizeof(uint32_t));

  Cursor cursor_id = lvkw_XcursorImageLoadCursor(ctx, ctx->display, image);
  lvkw_XcursorImageDestroy(ctx, image);

  if (cursor_id == None) return LVKW_ERROR;

  LVKW_Cursor_X11 *cursor = (LVKW_Cursor_X11 *)_ctx_alloc(ctx, sizeof(LVKW_Cursor_X11));
  if (!cursor) {
    lvkw_XFreeCursor(ctx, ctx->display, cursor_id);
    return LVKW_ERROR;
  }

  memset(cursor, 0, sizeof(*cursor));
  cursor->base.prv.ctx_base = &ctx->linux_base.base;
#ifdef LVKW_INDIRECT_BACKEND
  cursor->base.prv.backend = &_lvkw_x11_backend;
#endif
  cursor->cursor = cursor_id;

  *out_cursor = (LVKW_Cursor *)cursor;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_cursor_destroy_X11(LVKW_Cursor *cursor_handle) {
  if (!cursor_handle) return LVKW_SUCCESS;
  LVKW_Cursor_X11 *cursor = (LVKW_Cursor_X11 *)cursor_handle;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)cursor->base.prv.ctx_base;

  if (cursor->base.pub.flags & LVKW_CURSOR_FLAG_SYSTEM) return LVKW_SUCCESS;

  lvkw_XFreeCursor(ctx, ctx->display, cursor->cursor);
  _ctx_free(ctx, cursor);

  return LVKW_SUCCESS;
}
