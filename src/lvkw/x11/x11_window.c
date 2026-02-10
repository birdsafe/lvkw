#include <string.h>

#include "dlib/X11.h"
#include "dlib/Xcursor.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_x11_internal.h"

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_x11_backend;
#endif

static Visual *_lvkw_x11_find_alpha_visual(Display *dpy, int screen, int *out_depth) {
  XVisualInfo vinfo_template;
  vinfo_template.screen = screen;
  vinfo_template.depth = 32;
  vinfo_template.class = TrueColor;

  int nitems;
  XVisualInfo *vinfo_list =
      XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask | VisualClassMask, &vinfo_template, &nitems);

  Visual *visual = NULL;
  if (vinfo_list && nitems > 0) {
    visual = vinfo_list[0].visual;
    *out_depth = 32;
  }

  if (vinfo_list) XFree(vinfo_list);
  return visual;
}

LVKW_ContextResult lvkw_window_create_X11(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                          LVKW_Window **out_window_handle) {
  *out_window_handle = NULL;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)ctx_handle;

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  LVKW_Window_X11 *window = (LVKW_Window_X11 *)_ctx_alloc(ctx, sizeof(LVKW_Window_X11));
  if (!window) return LVKW_ERROR_NOOP;
  memset(window, 0, sizeof(*window));
#ifdef LVKW_INDIRECT_BACKEND
  window->base.prv.backend = &_lvkw_x11_backend;
#endif
  window->base.prv.ctx_base = &ctx->base;
  window->base.pub.userdata = create_info->userdata;
  window->size = create_info->size;

  uint32_t physical_width = (uint32_t)((double)create_info->size.width * ctx->scale);
  uint32_t physical_height = (uint32_t)((double)create_info->size.height * ctx->scale);

  int screen = DefaultScreen(ctx->display);
  Visual *visual = NULL;
  int depth = 0;

  if (create_info->flags & LVKW_WINDOW_TRANSPARENT) {
    visual = _lvkw_x11_find_alpha_visual(ctx->display, screen, &depth);
  }

  if (!visual) {
    visual = DefaultVisual(ctx->display, screen);
    depth = DefaultDepth(ctx->display, screen);
  }

  window->colormap = XCreateColormap(ctx->display, RootWindow(ctx->display, screen), visual, AllocNone);

  XSetWindowAttributes swa;
  swa.colormap = window->colormap;
  swa.background_pixel = 0;
  swa.border_pixel = 0;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask |
                   ButtonReleaseMask | StructureNotifyMask;

  window->window =
      XCreateWindow(ctx->display, RootWindow(ctx->display, screen), 0, 0, physical_width, physical_height, 0, depth,
                    InputOutput, visual, CWColormap | CWBackPixel | CWBorderPixel | CWEventMask, &swa);

  if (!window->window) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "XCreateWindow failed");
    XFreeColormap(ctx->display, window->colormap);
    _ctx_free(ctx, window);
    return LVKW_ERROR_NOOP;
  }

  XStoreName(ctx->display, window->window, create_info->title ? create_info->title : "Lvkw");

  if (create_info->app_id) {
    XClassHint *hint = XAllocClassHint();
    if (hint) {
      hint->res_name = (char *)create_info->app_id;
      hint->res_class = (char *)create_info->app_id;
      XSetClassHint(ctx->display, window->window, hint);
      XFree(hint);
    }
  }

  Atom protocols[] = {ctx->wm_delete_window, ctx->wm_take_focus, ctx->net_wm_ping};
  XSetWMProtocols(ctx->display, window->window, protocols, 3);

  XSaveContext(ctx->display, window->window, ctx->window_context, (XPointer)window);

  XMapWindow(ctx->display, window->window);

  // Add to context window list
  _lvkw_window_list_add(&ctx->base, &window->base);

  window->base.pub.is_ready = true;

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_OK;
}

void lvkw_window_destroy_X11(LVKW_Window *window_handle) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;
  if (!window) return;
  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  // Remove from window list
  _lvkw_window_list_remove(&ctx->base, &window->base);

  if (ctx->locked_window == window) {
    ctx->locked_window = NULL;
    XUngrabPointer(ctx->display, CurrentTime);
  }

  lvkw_event_queue_remove_window_events(&ctx->event_queue, window_handle);

  XDeleteContext(ctx->display, window->window, ctx->window_context);
  XDestroyWindow(ctx->display, window->window);
  XFreeColormap(ctx->display, window->colormap);
  _ctx_free(ctx, window);
}

LVKW_WindowResult lvkw_window_createVkSurface_X11(const LVKW_Window *window_handle, VkInstance instance,

                                                  VkSurfaceKHR *out_surface) {
  *out_surface = VK_NULL_HANDLE;

  const LVKW_Window_X11 *window = (const LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.is_lost) return LVKW_ERROR_WINDOW_LOST;

  PFN_vkCreateXlibSurfaceKHR fpCreateXlibSurfaceKHR =

      (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");

  if (!fpCreateXlibSurfaceKHR) {
    LVKW_REPORT_WIND_DIAGNOSIS(&window->base, LVKW_DIAGNOSIS_VULKAN_FAILURE, "vkCreateXlibSurfaceKHR not found");

    return LVKW_ERROR_NOOP;
  }

  VkXlibSurfaceCreateInfoKHR createInfo = {

      .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,

      .dpy = ctx->display,

      .window = window->window,

  };

  if (fpCreateXlibSurfaceKHR(instance, &createInfo, NULL, out_surface) != VK_SUCCESS) {
    LVKW_REPORT_WIND_DIAGNOSIS(&window->base, LVKW_DIAGNOSIS_VULKAN_FAILURE, "vkCreateXlibSurfaceKHR failure");

    return LVKW_ERROR_NOOP;
  }

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.is_lost) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_OK;
}

LVKW_WindowResult lvkw_window_getFramebufferSize_X11(const LVKW_Window *window_handle, LVKW_Size *out_size) {
  const LVKW_Window_X11 *window = (const LVKW_Window_X11 *)window_handle;

  const LVKW_Context_X11 *ctx = (const LVKW_Context_X11 *)window->base.prv.ctx_base;

  out_size->width = (uint32_t)((double)window->size.width * ctx->scale);

  out_size->height = (uint32_t)((double)window->size.height * ctx->scale);

  return LVKW_OK;
}

LVKW_WindowResult lvkw_window_setFullscreen_X11(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;

  if (window->base.pub.is_lost) return LVKW_ERROR_WINDOW_LOST;

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

  XSendEvent(ctx->display, DefaultRootWindow(ctx->display), False, SubstructureNotifyMask | StructureNotifyMask, &ev);

  _lvkw_x11_check_error(ctx);
  if (ctx->base.pub.is_lost) return LVKW_ERROR_CONTEXT_LOST;
  if (window->base.pub.is_lost) return LVKW_ERROR_WINDOW_LOST;

  return LVKW_OK;
}

LVKW_Status lvkw_window_setCursorMode_X11(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  if (!window) return LVKW_OK;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.is_lost || window->base.pub.is_lost) return LVKW_ERROR_NOOP;

  if (window->cursor_mode == mode) return LVKW_OK;

  Display *dpy = ctx->display;

  if (mode == LVKW_CURSOR_LOCKED) {
    uint32_t phys_w = (uint32_t)((double)window->size.width * ctx->scale);

    uint32_t phys_h = (uint32_t)((double)window->size.height * ctx->scale);

    XGrabPointer(dpy, window->window, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,

                 GrabModeAsync, window->window, ctx->hidden_cursor, CurrentTime);

    XWarpPointer(dpy, None, window->window, 0, 0, 0, 0, (int)(phys_w / 2), (int)(phys_h / 2));

    window->last_x = (double)(phys_w / 2.0);

    window->last_y = (double)(phys_h / 2.0);

    ctx->locked_window = window;
  }

  else {
    XUngrabPointer(dpy, CurrentTime);

    XUndefineCursor(dpy, window->window);

    if (ctx->locked_window == window) ctx->locked_window = NULL;
  }

  window->cursor_mode = mode;

  return LVKW_OK;
}

static const char *_lvkw_x11_cursor_shape_to_name(LVKW_CursorShape shape) {
  switch (shape) {
    case LVKW_CURSOR_SHAPE_DEFAULT:

      return "left_ptr";

    case LVKW_CURSOR_SHAPE_CONTEXT_MENU:

      return "context-menu";

    case LVKW_CURSOR_SHAPE_HELP:

      return "help";

    case LVKW_CURSOR_SHAPE_POINTER:

      return "hand2";

    case LVKW_CURSOR_SHAPE_PROGRESS:

      return "progress";

    case LVKW_CURSOR_SHAPE_WAIT:

      return "watch";

    case LVKW_CURSOR_SHAPE_CELL:

      return "cell";

    case LVKW_CURSOR_SHAPE_CROSSHAIR:

      return "crosshair";

    case LVKW_CURSOR_SHAPE_TEXT:

      return "xterm";

    case LVKW_CURSOR_SHAPE_VERTICAL_TEXT:

      return "vertical-text";

    case LVKW_CURSOR_SHAPE_ALIAS:

      return "alias";

    case LVKW_CURSOR_SHAPE_COPY:

      return "copy";

    case LVKW_CURSOR_SHAPE_MOVE:

      return "move";

    case LVKW_CURSOR_SHAPE_NO_DROP:

      return "no-drop";

    case LVKW_CURSOR_SHAPE_NOT_ALLOWED:

      return "not-allowed";

    case LVKW_CURSOR_SHAPE_GRAB:

      return "grab";

    case LVKW_CURSOR_SHAPE_GRABBING:

      return "grabbing";

    case LVKW_CURSOR_SHAPE_E_RESIZE:

      return "e-resize";

    case LVKW_CURSOR_SHAPE_N_RESIZE:

      return "n-resize";

    case LVKW_CURSOR_SHAPE_NE_RESIZE:

      return "ne-resize";

    case LVKW_CURSOR_SHAPE_NW_RESIZE:

      return "nw-resize";

    case LVKW_CURSOR_SHAPE_S_RESIZE:

      return "s-resize";

    case LVKW_CURSOR_SHAPE_SE_RESIZE:

      return "se-resize";

    case LVKW_CURSOR_SHAPE_SW_RESIZE:

      return "sw-resize";

    case LVKW_CURSOR_SHAPE_W_RESIZE:

      return "w-resize";

    case LVKW_CURSOR_SHAPE_EW_RESIZE:

      return "ew-resize";

    case LVKW_CURSOR_SHAPE_NS_RESIZE:

      return "ns-resize";

    case LVKW_CURSOR_SHAPE_NESW_RESIZE:

      return "nesw-resize";

    case LVKW_CURSOR_SHAPE_NWSE_RESIZE:

      return "nwse-resize";

    case LVKW_CURSOR_SHAPE_COL_RESIZE:

      return "col-resize";

    case LVKW_CURSOR_SHAPE_ROW_RESIZE:

      return "row-resize";

    case LVKW_CURSOR_SHAPE_ALL_SCROLL:

      return "all-scroll";

    case LVKW_CURSOR_SHAPE_ZOOM_IN:

      return "zoom-in";

    case LVKW_CURSOR_SHAPE_ZOOM_OUT:

      return "zoom-out";

    default:

      return "left_ptr";
  }
}

LVKW_Status lvkw_window_setCursorShape_X11(LVKW_Window *window_handle, LVKW_CursorShape shape) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  if (!window) return LVKW_OK;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.is_lost || window->base.pub.is_lost) return LVKW_ERROR_NOOP;

  if (!_lvkw_lib_xcursor.base.available) {
    LVKW_REPORT_WIND_DIAGNOSIS(&window->base, LVKW_DIAGNOSIS_FEATURE_UNSUPPORTED, "Xcursor extension not available");

    return LVKW_ERROR_NOOP;
  }

  const char *name = _lvkw_x11_cursor_shape_to_name(shape);

  Cursor cursor = XcursorLibraryLoadCursor(ctx->display, name);

  if (cursor != None) {
    XDefineCursor(ctx->display, window->window, cursor);

    XFreeCursor(ctx->display, cursor);
  }

  return LVKW_OK;
}

LVKW_Status lvkw_window_requestFocus_X11(LVKW_Window *window_handle) {
  LVKW_Window_X11 *window = (LVKW_Window_X11 *)window_handle;

  if (!window) return LVKW_OK;

  LVKW_Context_X11 *ctx = (LVKW_Context_X11 *)window->base.prv.ctx_base;

  _lvkw_x11_check_error(ctx);

  if (ctx->base.pub.is_lost || window->base.pub.is_lost) return LVKW_ERROR_NOOP;

  XEvent ev;

  memset(&ev, 0, sizeof(ev));

  ev.type = ClientMessage;

  ev.xclient.window = window->window;

  ev.xclient.message_type = ctx->net_active_window;

  ev.xclient.format = 32;

  ev.xclient.data.l[0] = 1;  // source indication: application

  ev.xclient.data.l[1] = CurrentTime;

  ev.xclient.data.l[2] = 0;

  XSendEvent(ctx->display, DefaultRootWindow(ctx->display), False, SubstructureNotifyMask | SubstructureRedirectMask,

             &ev);

  return LVKW_OK;
}
