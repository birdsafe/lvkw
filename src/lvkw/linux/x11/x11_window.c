// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <string.h>

#include <X11/Xatom.h>

#include "dlib/X11.h"
#include "dlib/Xcursor.h"
#include "dlib/Xss.h"
#include "lvkw/c/core.h"
#include "lvkw/lvkw.h"
#include "api_constraints.h"
#include "x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_x11_backend;
#endif

static LVKW_Status _lvkw_wnd_setFullscreen_X11(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setMaximized_X11(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode);
LVKW_Status _lvkw_wnd_setCursor_X11(LVKW_Window *window_handle, LVKW_Cursor *cursor_handle);
static void _lvkw_wnd_apply_size_hints_X11(LVKW_Window_X11 *window);
static void _lvkw_wnd_apply_motif_hints_X11(LVKW_Window_X11 *window);
static void _lvkw_wnd_apply_dnd_awareness_X11(LVKW_Window_X11 *window);

typedef struct LVKW_MotifHints {
  long flags;
  long functions;
  long decorations;
  long input_mode;
  long status;
} LVKW_MotifHints;

#define LVKW_MWM_HINTS_FUNCTIONS (1u << 0)
#define LVKW_MWM_HINTS_DECORATIONS (1u << 1)
#define LVKW_MWM_FUNC_ALL (1u << 0)
#define LVKW_MWM_FUNC_RESIZE (1u << 1)
#define LVKW_MWM_FUNC_MAXIMIZE (1u << 4)

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
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_Window_X11 *window = (LVKW_Window_X11 *)_ctx_alloc(ctx, sizeof(LVKW_Window_X11));
  if (!window) return LVKW_ERROR;
  memset(window, 0, sizeof(*window));
#ifdef LVKW_INDIRECT_BACKEND
  window->base.prv.backend = &_lvkw_x11_backend;
#endif
  window->base.prv.ctx_base = &ctx->linux_base.base;
  window->base.pub.context = &ctx->linux_base.base.pub;
  window->base.pub.userdata = create_info->userdata;
  window->size = create_info->attributes.logical_size;
  window->min_size = create_info->attributes.min_size;
  window->max_size = create_info->attributes.max_size;
  window->aspect_ratio = create_info->attributes.aspect_ratio;
  window->is_resizable = create_info->attributes.resizable;
  window->is_decorated = create_info->attributes.decorated;
  window->mouse_passthrough = create_info->attributes.mouse_passthrough;
  window->accept_dnd = create_info->attributes.accept_dnd;
  window->text_input_type = create_info->attributes.text_input_type;
  window->text_input_rect = create_info->attributes.text_input_rect;
  window->monitor = create_info->attributes.monitor;
  window->cursor = create_info->attributes.cursor;
  window->transparent = create_info->transparent;

  if (window->text_input_type != LVKW_TEXT_INPUT_TYPE_NONE) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "X11 text input hints are not implemented yet");
  }

  uint32_t pixel_width = (uint32_t)((LVKW_Scalar)create_info->attributes.logical_size.x * ctx->scale);
  uint32_t pixel_height = (uint32_t)((LVKW_Scalar)create_info->attributes.logical_size.y * ctx->scale);

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
                   LeaveWindowMask | FocusChangeMask | PropertyChangeMask;

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
  _lvkw_wnd_apply_size_hints_X11(window);
  _lvkw_wnd_apply_motif_hints_X11(window);
  _lvkw_wnd_apply_dnd_awareness_X11(window);

  lvkw_XSaveContext(ctx, ctx->display, window->window, ctx->window_context, (XPointer)window);

  LVKW_Status status = _lvkw_wnd_setCursor_X11((LVKW_Window *)window, window->cursor);
  if (status != LVKW_SUCCESS) goto fail_window_create;
  status = _lvkw_wnd_setCursorMode_X11((LVKW_Window *)window, create_info->attributes.cursor_mode);
  if (status != LVKW_SUCCESS) goto fail_window_create;

  lvkw_XMapWindow(ctx, ctx->display, window->window);

  if (create_info->attributes.fullscreen) {
    status = _lvkw_wnd_setFullscreen_X11((LVKW_Window *)window, true);
    if (status != LVKW_SUCCESS) goto fail_window_create;
  }
  if (create_info->attributes.maximized) {
    status = _lvkw_wnd_setMaximized_X11((LVKW_Window *)window, true);
    if (status != LVKW_SUCCESS) goto fail_window_create;
  }

  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) {
    status = LVKW_ERROR_CONTEXT_LOST;
    goto fail_window_create;
  }
  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) {
    status = LVKW_ERROR_WINDOW_LOST;
    goto fail_window_create;
  }

  // Add to context window list
  _lvkw_window_list_add(&ctx->linux_base.base, &window->base);

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;

fail_window_create:
  lvkw_XDeleteContext(ctx, ctx->display, window->window, ctx->window_context);
  lvkw_XDestroyWindow(ctx, ctx->display, window->window);
  lvkw_XFreeColormap(ctx, ctx->display, window->colormap);
  _ctx_free(ctx, window);
  return status;
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

  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

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

  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getGeometry_X11(LVKW_Window *window_handle,
                                     LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  const LVKW_Window_X11 *window = (const LVKW_Window_X11 *)window_handle;

  const LVKW_Context_X11 *ctx = (const LVKW_Context_X11 *)window->base.prv.ctx_base;

  out_geometry->logical_size = window->size;
  out_geometry->pixel_size.x = (int32_t)((LVKW_Scalar)window->size.x * ctx->scale);
  out_geometry->pixel_size.y = (int32_t)((LVKW_Scalar)window->size.y * ctx->scale);
  out_geometry->origin.x = 0;
  out_geometry->origin.y = 0;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_update_X11(LVKW_Window *window_handle, uint32_t field_mask,
                                const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  if (field_mask & LVKW_WINDOW_ATTR_TITLE) {
    lvkw_XStoreName(ctx, ctx->display, window->window, attributes->title ? attributes->title : "Lvkw");
  }

  if (field_mask & LVKW_WINDOW_ATTR_LOGICAL_SIZE) {
    window->size = attributes->logical_size;

    uint32_t pixel_width = (uint32_t)((LVKW_Scalar)attributes->logical_size.x * ctx->scale);

    uint32_t pixel_height = (uint32_t)((LVKW_Scalar)attributes->logical_size.y * ctx->scale);
    lvkw_XResizeWindow(ctx, ctx->display, window->window, pixel_width, pixel_height);
  }

  if (field_mask & LVKW_WINDOW_ATTR_FULLSCREEN) {
    _lvkw_wnd_setFullscreen_X11(window_handle, attributes->fullscreen);
  }

  if (field_mask & LVKW_WINDOW_ATTR_MAXIMIZED) {
    _lvkw_wnd_setMaximized_X11(window_handle, attributes->maximized);
  }

  if (field_mask & LVKW_WINDOW_ATTR_MONITOR) {
    window->monitor = attributes->monitor;
    if (attributes->monitor) {
      LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                  "X11 monitor targeting is not implemented yet");
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_MIN_SIZE) {
    window->min_size = attributes->min_size;
    _lvkw_wnd_apply_size_hints_X11(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_MAX_SIZE) {
    window->max_size = attributes->max_size;
    _lvkw_wnd_apply_size_hints_X11(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_ASPECT_RATIO) {
    window->aspect_ratio = attributes->aspect_ratio;
    _lvkw_wnd_apply_size_hints_X11(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_RESIZABLE) {
    window->is_resizable = attributes->resizable;
    _lvkw_wnd_apply_size_hints_X11(window);
    _lvkw_wnd_apply_motif_hints_X11(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_DECORATED) {
    window->is_decorated = attributes->decorated;
    _lvkw_wnd_apply_motif_hints_X11(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_MOUSE_PASSTHROUGH) {
    if (attributes->mouse_passthrough) {
      LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                  "Mouse passthrough not implemented yet on X11");
    }
    window->mouse_passthrough = attributes->mouse_passthrough;
  }

  if (field_mask & LVKW_WINDOW_ATTR_ACCEPT_DND) {
    window->accept_dnd = attributes->accept_dnd;
    _lvkw_wnd_apply_dnd_awareness_X11(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE) {
    window->text_input_type = attributes->text_input_type;
    if (window->text_input_type != LVKW_TEXT_INPUT_TYPE_NONE) {
      LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                  "X11 text input hints are not implemented yet");
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_TEXT_INPUT_RECT) {
    window->text_input_rect = attributes->text_input_rect;
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "X11 text input rectangle hint is not implemented yet");
  }

  if (field_mask & LVKW_WINDOW_ATTR_CURSOR_MODE) {
    _lvkw_wnd_setCursorMode_X11(window_handle, attributes->cursor_mode);
  }

  if (field_mask & LVKW_WINDOW_ATTR_CURSOR) {
    _lvkw_wnd_setCursor_X11(window_handle, attributes->cursor);
  }

  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setMaximized_X11(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  XEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = ClientMessage;
  ev.xclient.window = window->window;
  ev.xclient.message_type = ctx->net_wm_state;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = enabled ? 1 : 0;  // _NET_WM_STATE_ADD/_NET_WM_STATE_REMOVE
  ev.xclient.data.l[1] = (long)ctx->net_wm_state_maximized_vert;
  ev.xclient.data.l[2] = (long)ctx->net_wm_state_maximized_horz;
  ev.xclient.data.l[3] = 1;  // source indication

  lvkw_XSendEvent(ctx, ctx->display, DefaultRootWindow(ctx->display), False,
                  SubstructureNotifyMask | SubstructureRedirectMask, &ev);

  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_SUCCESS;
}

static void _lvkw_wnd_apply_size_hints_X11(LVKW_Window_X11 *window) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;
  XSizeHints *hints = lvkw_XAllocSizeHints(ctx);
  if (!hints) return;
  memset(hints, 0, sizeof(*hints));

  uint32_t min_w = (uint32_t)(window->min_size.x * ctx->scale);
  uint32_t min_h = (uint32_t)(window->min_size.y * ctx->scale);
  uint32_t max_w = (uint32_t)(window->max_size.x * ctx->scale);
  uint32_t max_h = (uint32_t)(window->max_size.y * ctx->scale);

  if (!window->is_resizable) {
    min_w = (uint32_t)(window->size.x * ctx->scale);
    min_h = (uint32_t)(window->size.y * ctx->scale);
    max_w = min_w;
    max_h = min_h;
  }

  if (min_w > 0 && min_h > 0) {
    hints->flags |= PMinSize;
    hints->min_width = (int)min_w;
    hints->min_height = (int)min_h;
  }
  if (max_w > 0 && max_h > 0) {
    hints->flags |= PMaxSize;
    hints->max_width = (int)max_w;
    hints->max_height = (int)max_h;
  }
  if (window->aspect_ratio.numerator > 0 && window->aspect_ratio.denominator > 0) {
    hints->flags |= PAspect;
    hints->min_aspect.x = window->aspect_ratio.numerator;
    hints->min_aspect.y = window->aspect_ratio.denominator;
    hints->max_aspect.x = window->aspect_ratio.numerator;
    hints->max_aspect.y = window->aspect_ratio.denominator;
  }

  lvkw_XSetWMNormalHints(ctx, ctx->display, window->window, hints);
  lvkw_XFree(ctx, hints);
}

static void _lvkw_wnd_apply_motif_hints_X11(LVKW_Window_X11 *window) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;
  LVKW_MotifHints hints;
  memset(&hints, 0, sizeof(hints));
  hints.flags = LVKW_MWM_HINTS_DECORATIONS | LVKW_MWM_HINTS_FUNCTIONS;
  hints.decorations = window->is_decorated ? 1u : 0u;
  hints.functions = LVKW_MWM_FUNC_ALL;
  if (!window->is_resizable) {
    hints.functions &= (long)~(LVKW_MWM_FUNC_RESIZE | LVKW_MWM_FUNC_MAXIMIZE);
  }

  lvkw_XChangeProperty(ctx, ctx->display, window->window, ctx->motif_wm_hints,
                       ctx->motif_wm_hints, 32, PropModeReplace,
                       (const unsigned char *)&hints, (int)(sizeof(hints) / sizeof(long)));
}

static void _lvkw_wnd_apply_dnd_awareness_X11(LVKW_Window_X11 *window) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;
  if (window->accept_dnd) {
    const long version = 5;
    lvkw_XChangeProperty(ctx, ctx->display, window->window, ctx->xdnd_aware, XA_ATOM, 32,
                         PropModeReplace, (const unsigned char *)&version, 1);
  } else {
    lvkw_XDeleteProperty(ctx, ctx->display, window->window, ctx->xdnd_aware);
  }
}

static LVKW_Status _lvkw_wnd_setFullscreen_X11(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

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
                  SubstructureNotifyMask | SubstructureRedirectMask, &ev);

  _lvkw_x11_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

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

    window->last_x = window->size.x / (LVKW_Scalar)2.0;
    window->last_y = window->size.y / (LVKW_Scalar)2.0;
    window->last_cursor_set = true;
    ctx->pending_raw_delta.x = 0;
    ctx->pending_raw_delta.y = 0;
    ctx->has_pending_raw_delta = false;

    ctx->locked_window = window;
  }

  else {
    lvkw_XUngrabPointer(ctx, dpy, CurrentTime);

    _lvkw_wnd_setCursor_X11(window_handle, window->cursor);

    if (ctx->locked_window == window) ctx->locked_window = NULL;
    window->last_cursor_set = false;
    ctx->pending_raw_delta.x = 0;
    ctx->pending_raw_delta.y = 0;
    ctx->has_pending_raw_delta = false;
  }

  window->cursor_mode = mode;

  return LVKW_SUCCESS;
}

LVKW_Status _lvkw_wnd_setCursor_X11(LVKW_Window *window_handle, LVKW_Cursor *cursor_handle) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;
  LVKW_Cursor_Base *cursor = (LVKW_Cursor_Base *)cursor_handle;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_UNKNOWN, "Setting X11 cursor");

  window->cursor = cursor_handle;

  if (window->cursor_mode != LVKW_CURSOR_LOCKED) {
    if (cursor) {
      if (cursor->prv.ctx_base != &ctx->linux_base.base) {
        LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_INVALID_ARGUMENT,
                                    "Cursor does not belong to this context");
        return LVKW_ERROR_INVALID_USAGE;
      }

      Cursor cursor_id = (Cursor)cursor->prv.backend_data[0];
      if ((cursor->pub.flags & LVKW_CURSOR_FLAG_SYSTEM) && cursor_id == None) {
        LVKW_Cursor *resolved = NULL;
        if (lvkw_ctx_getStandardCursor_X11((LVKW_Context *)ctx, cursor->prv.shape, &resolved) == LVKW_SUCCESS &&
            resolved) {
          cursor = (LVKW_Cursor_Base *)resolved;
          cursor_id = (Cursor)cursor->prv.backend_data[0];
        }
      }

      if (cursor_id != None) {
        lvkw_XDefineCursor(ctx, ctx->display, window->window, cursor_id);
      }
      else {
        lvkw_XUndefineCursor(ctx, ctx->display, window->window);
      }
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

  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.flags & LVKW_WINDOW_STATE_LOST) return LVKW_ERROR_WINDOW_LOST;

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

static void _clipboard_invalidate_mime_query(LVKW_Context_X11 *ctx) {
  if (ctx->clipboard_mime_query_ptr) {
    lvkw_context_free(&ctx->linux_base.base, (void *)ctx->clipboard_mime_query_ptr);
    ctx->clipboard_mime_query_ptr = NULL;
  }
  ctx->clipboard_mime_query_count = 0;
}

static void _clipboard_clear_owned_data(LVKW_Context_X11 *ctx) {
  if (ctx->clipboard_owned_mimes) {
    for (uint32_t i = 0; i < ctx->clipboard_owned_mime_count; ++i) {
      lvkw_context_free(&ctx->linux_base.base, ctx->clipboard_owned_mimes[i].bytes);
    }
    lvkw_context_free(&ctx->linux_base.base, ctx->clipboard_owned_mimes);
    ctx->clipboard_owned_mimes = NULL;
    ctx->clipboard_owned_mime_count = 0;
  }
}

static bool _clipboard_ensure_read_cache_capacity(LVKW_Context_X11 *ctx, size_t capacity) {
  if (capacity <= ctx->clipboard_read_cache_capacity) return true;
  uint8_t *next =
      lvkw_context_realloc(&ctx->linux_base.base, ctx->clipboard_read_cache,
                           ctx->clipboard_read_cache_capacity, capacity);
  if (!next) return false;
  ctx->clipboard_read_cache = next;
  ctx->clipboard_read_cache_capacity = capacity;
  return true;
}

static bool _clipboard_copy_into_read_cache(LVKW_Context_X11 *ctx, const void *data, size_t size,
                                            bool add_nul) {
  const size_t need_size = size + (add_nul ? 1u : 0u);
  if (!_clipboard_ensure_read_cache_capacity(ctx, need_size)) return false;
  if (size > 0) memcpy(ctx->clipboard_read_cache, data, size);
  if (add_nul) ctx->clipboard_read_cache[size] = '\0';
  ctx->clipboard_read_cache_size = size;
  return true;
}

static bool _clipboard_append_mime_unique(LVKW_Context_X11 *ctx, const char **list, uint32_t capacity,
                                          uint32_t *count, const char *mime) {
  const char *interned =
      _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache, &ctx->linux_base.base, mime);
  if (!interned) return false;

  for (uint32_t i = 0; i < *count; ++i) {
    if (strcmp(list[i], interned) == 0) return true;
  }

  if (*count >= capacity) return false;
  list[*count] = interned;
  (*count)++;
  return true;
}

static LVKW_X11ClipboardMime *_clipboard_find_owned_mime_by_type(LVKW_Context_X11 *ctx,
                                                                  const char *mime_type) {
  for (uint32_t i = 0; i < ctx->clipboard_owned_mime_count; ++i) {
    if (strcmp(ctx->clipboard_owned_mimes[i].mime_type, mime_type) == 0) {
      return &ctx->clipboard_owned_mimes[i];
    }
  }
  return NULL;
}

static bool _clipboard_request_target(LVKW_Window_X11 *window, Atom target, Atom *out_type,
                                      int *out_format, uint8_t **out_data, size_t *out_size) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  lvkw_XDeleteProperty(ctx, ctx->display, window->window, ctx->clipboard_property);
  lvkw_XConvertSelection(ctx, ctx->display, ctx->clipboard, target, ctx->clipboard_property,
                         window->window, CurrentTime);
  lvkw_XSync(ctx, ctx->display, False);

  const uint32_t timeout_ms = 1000;
  uint64_t start = _lvkw_get_timestamp_ms();
  while ((_lvkw_get_timestamp_ms() - start) < timeout_ms) {
    if (lvkw_ctx_pumpEvents_X11((LVKW_Context *)ctx, 10) != LVKW_SUCCESS) return false;

    Atom actual_type = None;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *probe = NULL;
    if (lvkw_XGetWindowProperty(ctx, ctx->display, window->window, ctx->clipboard_property, 0, 0,
                                False, AnyPropertyType, &actual_type, &actual_format, &nitems,
                                &bytes_after, &probe) != Success) {
      if (probe) lvkw_XFree(ctx, probe);
      return false;
    }
    if (probe) lvkw_XFree(ctx, probe);
    if (actual_type == None) continue;

    unsigned char *payload = NULL;
    if (lvkw_XGetWindowProperty(ctx, ctx->display, window->window, ctx->clipboard_property, 0,
                                1024 * 1024, True, AnyPropertyType, &actual_type, &actual_format,
                                &nitems, &bytes_after, &payload) != Success) {
      if (payload) lvkw_XFree(ctx, payload);
      return false;
    }

    *out_type = actual_type;
    *out_format = actual_format;
    *out_data = payload;
    // XGetWindowProperty reports 32-bit properties as an array of longs.
    *out_size = (actual_format == 8) ? (size_t)nitems
               : (actual_format == 16) ? (size_t)nitems * 2u
                                       : (size_t)nitems * sizeof(long);
    return true;
  }

  return false;
}

LVKW_Status lvkw_wnd_setClipboardText_X11(LVKW_Window *window, const char *text) {
  LVKW_API_VALIDATE(wnd_setClipboardText, window, text);
  const size_t text_size = strlen(text);
  const LVKW_ClipboardData items[2] = {
      {.mime_type = "text/plain;charset=utf-8", .data = text, .size = text_size},
      {.mime_type = "text/plain", .data = text, .size = text_size},
  };
  return lvkw_wnd_setClipboardData_X11(window, items, 2);
}

LVKW_Status lvkw_wnd_getClipboardText_X11(LVKW_Window *window, const char **out_text) {
  LVKW_API_VALIDATE(wnd_getClipboardText, window, out_text);
  const void *bytes = NULL;
  size_t size = 0;
  LVKW_Status status =
      lvkw_wnd_getClipboardData_X11(window, "text/plain;charset=utf-8", &bytes, &size);
  if (status != LVKW_SUCCESS) {
    status = lvkw_wnd_getClipboardData_X11(window, "text/plain", &bytes, &size);
    if (status != LVKW_SUCCESS) return status;
  }

  LVKW_Window_X11 *x11_window = (LVKW_Window_X11 *)window;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)x11_window->base.prv.ctx_base;
  if (!_clipboard_copy_into_read_cache(ctx, bytes, size, true)) return LVKW_ERROR;
  *out_text = (const char *)ctx->clipboard_read_cache;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setClipboardData_X11(LVKW_Window *window, const LVKW_ClipboardData *data,
                                          uint32_t count) {
  LVKW_API_VALIDATE(wnd_setClipboardData, window, data, count);
  LVKW_Window_X11 *x11_window = (LVKW_Window_X11 *)window;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)x11_window->base.prv.ctx_base;

  _clipboard_clear_owned_data(ctx);
  _clipboard_invalidate_mime_query(ctx);
  ctx->clipboard_read_cache_size = 0;

  LVKW_X11ClipboardMime *owned =
      (LVKW_X11ClipboardMime *)lvkw_context_alloc(&ctx->linux_base.base, sizeof(*owned) * count);
  if (!owned) return LVKW_ERROR;
  memset(owned, 0, sizeof(*owned) * count);

  for (uint32_t i = 0; i < count; ++i) {
    owned[i].mime_type = _lvkw_string_cache_intern(&ctx->linux_base.base.prv.string_cache,
                                                   &ctx->linux_base.base, data[i].mime_type);
    owned[i].atom = lvkw_XInternAtom(ctx, ctx->display, owned[i].mime_type, False);
    owned[i].size = data[i].size;
    if (data[i].size > 0) {
      owned[i].bytes = lvkw_context_alloc(&ctx->linux_base.base, data[i].size);
      if (!owned[i].bytes) {
        for (uint32_t j = 0; j < i; ++j) lvkw_context_free(&ctx->linux_base.base, owned[j].bytes);
        lvkw_context_free(&ctx->linux_base.base, owned);
        return LVKW_ERROR;
      }
      memcpy(owned[i].bytes, data[i].data, data[i].size);
    }
  }

  ctx->clipboard_owned_mimes = owned;
  ctx->clipboard_owned_mime_count = count;
  ctx->clipboard_owner_window = x11_window->window;

  lvkw_XSetSelectionOwner(ctx, ctx->display, ctx->clipboard, x11_window->window, CurrentTime);
  if (lvkw_XGetSelectionOwner(ctx, ctx->display, ctx->clipboard) != x11_window->window) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&x11_window->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                "Failed to become clipboard owner");
    return LVKW_ERROR;
  }
  lvkw_XSync(ctx, ctx->display, False);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardData_X11(LVKW_Window *window, const char *mime_type,
                                          const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(wnd_getClipboardData, window, mime_type, out_data, out_size);
  LVKW_Window_X11 *x11_window = (LVKW_Window_X11 *)window;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)x11_window->base.prv.ctx_base;
  ctx->clipboard_read_cache_size = 0;

  if (ctx->clipboard_owner_window == x11_window->window) {
    LVKW_X11ClipboardMime *mime = _clipboard_find_owned_mime_by_type(ctx, mime_type);
    if (!mime && strcmp(mime_type, "text/plain") == 0) {
      mime = _clipboard_find_owned_mime_by_type(ctx, "text/plain;charset=utf-8");
    }
    if (!mime) {
      LVKW_REPORT_WIND_DIAGNOSTIC(&x11_window->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                  "Requested MIME type is not available");
      return LVKW_ERROR;
    }
    if (!_clipboard_copy_into_read_cache(ctx, mime->bytes, mime->size, false)) return LVKW_ERROR;
    *out_data = ctx->clipboard_read_cache;
    *out_size = ctx->clipboard_read_cache_size;
    return LVKW_SUCCESS;
  }

  Atom target = lvkw_XInternAtom(ctx, ctx->display, mime_type, True);

  Atom type = None;
  int format = 0;
  uint8_t *payload = NULL;
  size_t payload_size = 0;
  const bool is_text_request =
      strcmp(mime_type, "text/plain") == 0 || strcmp(mime_type, "text/plain;charset=utf-8") == 0;

  if (target == None && is_text_request) {
    target = ctx->utf8_string;
  }
  if (target == None) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&x11_window->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                "Requested MIME type is not available");
    return LVKW_ERROR;
  }

  bool requested = _clipboard_request_target(x11_window, target, &type, &format, &payload, &payload_size);
  if (!requested && is_text_request && target != XA_STRING) {
    requested = _clipboard_request_target(x11_window, XA_STRING, &type, &format, &payload, &payload_size);
  }
  if (!requested) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&x11_window->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                "Clipboard transfer failed");
    return LVKW_ERROR;
  }
  if (format != 8 || !payload) {
    if (payload) lvkw_XFree(ctx, payload);
    LVKW_REPORT_WIND_DIAGNOSTIC(&x11_window->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                "Clipboard payload format unsupported");
    return LVKW_ERROR;
  }

  bool ok = _clipboard_copy_into_read_cache(ctx, payload, payload_size, false);
  lvkw_XFree(ctx, payload);
  if (!ok) return LVKW_ERROR;
  *out_data = ctx->clipboard_read_cache;
  *out_size = ctx->clipboard_read_cache_size;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_X11(LVKW_Window *window, const char ***out_mime_types,
                                               uint32_t *count) {
  LVKW_API_VALIDATE(wnd_getClipboardMimeTypes, window, out_mime_types, count);
  LVKW_Window_X11 *x11_window = (LVKW_Window_X11 *)window;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)x11_window->base.prv.ctx_base;

  _clipboard_invalidate_mime_query(ctx);

  if (ctx->clipboard_owner_window == x11_window->window) {
    const uint32_t n = ctx->clipboard_owned_mime_count;
    if (!out_mime_types) {
      *count = n;
      return LVKW_SUCCESS;
    }

    const char **list = (const char **)lvkw_context_alloc(&ctx->linux_base.base, sizeof(char *) * n);
    if (!list && n > 0) return LVKW_ERROR;
    for (uint32_t i = 0; i < n; ++i) list[i] = ctx->clipboard_owned_mimes[i].mime_type;
    ctx->clipboard_mime_query_ptr = list;
    ctx->clipboard_mime_query_count = n;
    *out_mime_types = list;
    *count = n;
    return LVKW_SUCCESS;
  }

  Atom type = None;
  int format = 0;
  uint8_t *payload = NULL;
  size_t payload_size = 0;
  if (!_clipboard_request_target(x11_window, ctx->targets, &type, &format, &payload, &payload_size)) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&x11_window->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                "Clipboard TARGETS query failed");
    return LVKW_ERROR;
  }
  if (type != XA_ATOM || format != 32 || !payload || payload_size == 0) {
    if (payload) lvkw_XFree(ctx, payload);
    LVKW_REPORT_WIND_DIAGNOSTIC(&x11_window->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                                "Clipboard TARGETS payload invalid");
    return LVKW_ERROR;
  }

  const uint32_t atom_count = (uint32_t)(payload_size / sizeof(Atom));
  const Atom *atoms = (const Atom *)payload;
  const uint32_t capacity = atom_count + 2u; // text aliases can expand a single X atom into two API MIME names.
  const char **list = (const char **)lvkw_context_alloc(&ctx->linux_base.base, sizeof(char *) * capacity);
  if (!list && atom_count > 0) {
    lvkw_XFree(ctx, payload);
    return LVKW_ERROR;
  }

  uint32_t out = 0;
  for (uint32_t i = 0; i < atom_count; ++i) {
    if (atoms[i] == ctx->targets) continue;
    if (atoms[i] == ctx->utf8_string) {
      if (!_clipboard_append_mime_unique(ctx, list, capacity, &out, "text/plain;charset=utf-8")) {
        lvkw_XFree(ctx, payload);
        lvkw_context_free(&ctx->linux_base.base, (void *)list);
        return LVKW_ERROR;
      }
      continue;
    }
    if (atoms[i] == ctx->text_atom || atoms[i] == XA_STRING) {
      if (!_clipboard_append_mime_unique(ctx, list, capacity, &out, "text/plain")) {
        lvkw_XFree(ctx, payload);
        lvkw_context_free(&ctx->linux_base.base, (void *)list);
        return LVKW_ERROR;
      }
      continue;
    }

    char *name = lvkw_XGetAtomName(ctx, ctx->display, atoms[i]);
    if (!name) continue;
    if (!_clipboard_append_mime_unique(ctx, list, capacity, &out, name)) {
      lvkw_XFree(ctx, name);
      lvkw_XFree(ctx, payload);
      lvkw_context_free(&ctx->linux_base.base, (void *)list);
      return LVKW_ERROR;
    }
    lvkw_XFree(ctx, name);
  }
  lvkw_XFree(ctx, payload);

  if (!out_mime_types) {
    *count = out;
    lvkw_context_free(&ctx->linux_base.base, (void *)list);
    return LVKW_SUCCESS;
  }

  ctx->clipboard_mime_query_ptr = list;
  ctx->clipboard_mime_query_count = out;
  *out_mime_types = list;
  *count = out;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getStandardCursor_X11(LVKW_Context *ctx_handle, LVKW_CursorShape shape,
                                           LVKW_Cursor **out_cursor) {
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;
  *out_cursor = NULL;

  if (shape < 1 || shape > 12) return LVKW_ERROR_INVALID_USAGE;

  LVKW_Cursor_Base *cursor = &ctx->linux_base.base.prv.standard_cursors[shape];
  Cursor cursor_id = (Cursor)cursor->prv.backend_data[0];

  if (cursor_id == None) {
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
      cursor_id = lvkw_XcursorLibraryLoadCursor(ctx, ctx->display, name);
    }

    if (cursor_id == None && fallback_shape != 0) {
      cursor_id = lvkw_XCreateFontCursor(ctx, ctx->display, fallback_shape);
    }

    if (cursor_id == None) {
      return LVKW_ERROR;
    }

    cursor->prv.backend_data[0] = (uintptr_t)cursor_id;
  }

  *out_cursor = &cursor->pub;
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

  image->xhot = (XcursorDim)create_info->hot_spot.x;
  image->yhot = (XcursorDim)create_info->hot_spot.y;
  image->delay = 0;

  memcpy(image->pixels, create_info->pixels,
         (size_t)create_info->size.x * (size_t)create_info->size.y * sizeof(uint32_t));

  Cursor cursor_id = lvkw_XcursorImageLoadCursor(ctx, ctx->display, image);
  lvkw_XcursorImageDestroy(ctx, image);

  if (cursor_id == None) return LVKW_ERROR;

  LVKW_Cursor_Base *cursor = (LVKW_Cursor_Base *)_ctx_alloc(ctx, sizeof(LVKW_Cursor_Base));
  if (!cursor) {
    lvkw_XFreeCursor(ctx, ctx->display, cursor_id);
    return LVKW_ERROR;
  }

  memset(cursor, 0, sizeof(*cursor));
  cursor->prv.ctx_base = &ctx->linux_base.base;
#ifdef LVKW_INDIRECT_BACKEND
  cursor->prv.backend = &_lvkw_x11_backend;
#endif
  cursor->prv.backend_data[0] = (uintptr_t)cursor_id;

  *out_cursor = &cursor->pub;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_cursor_destroy_X11(LVKW_Cursor *cursor_handle) {
  if (!cursor_handle) return LVKW_SUCCESS;
  LVKW_Cursor_Base *cursor = (LVKW_Cursor_Base *)cursor_handle;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)cursor->prv.ctx_base;

  if (cursor->pub.flags & LVKW_CURSOR_FLAG_SYSTEM) return LVKW_SUCCESS;

  Cursor cursor_id = (Cursor)cursor->prv.backend_data[0];
  if (cursor_id != None) {
    lvkw_XFreeCursor(ctx, ctx->display, cursor_id);
  }
  _ctx_free(ctx, cursor);

  return LVKW_SUCCESS;
}
