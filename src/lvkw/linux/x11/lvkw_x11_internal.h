// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_X11_INTERNAL_H_INCLUDED
#define LVKW_X11_INTERNAL_H_INCLUDED

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/scrnsaver.h>
#include <X11/keysym.h>

#include "dlib/X11.h"
#include "dlib/Xcursor.h"
#include "dlib/Xi.h"
#include "dlib/Xlib-xcb.h"
#include "dlib/Xrandr.h"
#include "dlib/Xss.h"
#include "dlib/xkbcommon.h"
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"
#include "lvkw_linux_internal.h"

#define _ctx_alloc(ctx, size) lvkw_context_alloc(&(ctx)->linux_base.base, size)
#define _ctx_free(ctx, ptr) lvkw_context_free(&(ctx)->linux_base.base, ptr)

typedef struct LVKW_Window_X11 LVKW_Window_X11;

typedef struct LVKW_Cursor_X11 {
  LVKW_Cursor_Base base;
  LVKW_CursorShape shape;
  Cursor cursor;
} LVKW_Cursor_X11;

typedef struct LVKW_Monitor_X11 {
  LVKW_Monitor_Base base;
  RROutput output;
  LVKW_VideoMode *modes;
  uint32_t mode_count;
} LVKW_Monitor_X11;

typedef struct LVKW_Context_X11 {
  LVKW_Context_Linux linux_base;

  struct {
    LVKW_Lib_X11 x11;
    LVKW_Lib_X11_XCB x11_xcb;
    LVKW_Lib_Xcursor xcursor;
    LVKW_Lib_Xrandr xrandr;
    LVKW_Lib_Xss xss;
    LVKW_Lib_Xi xi;
    LVKW_Lib_Xkb xkb;
  } dlib;

  LVKW_Scalar scale;
  Display *display;
  int randr_event_base;
  int randr_error_base;
  bool randr_available;
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
  LVKW_Window_X11 *locked_window;
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

/* X11 backend functions */

LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info,
                                LVKW_Context **out_context);
LVKW_Status lvkw_ctx_destroy_X11(LVKW_Context *handle);
LVKW_Status lvkw_ctx_getVkExtensions_X11(LVKW_Context *ctx, uint32_t *count,
                                         const char *const **out_extensions);
LVKW_Status lvkw_ctx_syncEvents_X11(LVKW_Context *ctx, uint32_t timeout_ms);
void _lvkw_x11_push_event_cb(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                             const LVKW_Event *evt);
LVKW_Status lvkw_ctx_postEvent_X11(LVKW_Context *ctx, LVKW_EventType type, LVKW_Window *window,
                                   const LVKW_Event *evt);
LVKW_Status lvkw_ctx_scanEvents_X11(LVKW_Context *ctx, LVKW_EventType event_mask,
                                    LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_update_X11(LVKW_Context *ctx, uint32_t field_mask,
                                const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_X11(LVKW_Context *ctx, LVKW_Monitor **out_monitors,
                                     uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_X11(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                         LVKW_VideoMode *out_modes, uint32_t *count);
LVKW_Status lvkw_ctx_getMetrics_X11(LVKW_Context *ctx, LVKW_MetricsCategory category,
                                      void *out_data, bool reset);
void _lvkw_x11_update_monitors(LVKW_Context_X11 *ctx);
LVKW_Status lvkw_ctx_createWindow_X11(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                      LVKW_Window **out_window);
LVKW_Status lvkw_wnd_destroy_X11(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_X11(LVKW_Window *window, VkInstance instance,
                                         VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_X11(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_X11(LVKW_Window *window, uint32_t field_mask,
                                const LVKW_WindowAttributes *attributes);
LVKW_Status _lvkw_wnd_setCursor_X11(LVKW_Window *window_handle, LVKW_Cursor *cursor_handle);
LVKW_Status lvkw_wnd_requestFocus_X11(LVKW_Window *window_handle);
LVKW_Status lvkw_wnd_setClipboardText_X11(LVKW_Window *window, const char *text);
LVKW_Status lvkw_wnd_getClipboardText_X11(LVKW_Window *window, const char **out_text);
LVKW_Status lvkw_wnd_setClipboardData_X11(LVKW_Window *window, const LVKW_ClipboardData *data,
                                          uint32_t count);
LVKW_Status lvkw_wnd_getClipboardData_X11(LVKW_Window *window, const char *mime_type,
                                          const void **out_data, size_t *out_size);
LVKW_Status lvkw_wnd_getClipboardMimeTypes_X11(LVKW_Window *window, const char ***out_mime_types,
                                               uint32_t *count);

LVKW_Status lvkw_ctx_getStandardCursor_X11(LVKW_Context *ctx, LVKW_CursorShape shape,
                                           LVKW_Cursor **out_cursor);
LVKW_Status lvkw_ctx_createCursor_X11(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                      LVKW_Cursor **out_cursor);
LVKW_Status lvkw_cursor_destroy_X11(LVKW_Cursor *cursor);

static inline void _lvkw_x11_check_error(LVKW_Context_X11 *ctx) {}

LVKW_MouseButton _lvkw_x11_translate_button(unsigned int button);

/* X11 helpers */

static inline Display *lvkw_XOpenDisplay(struct LVKW_Context_X11 *ctx, const char *display_name) {
  return ctx->dlib.x11.OpenDisplay(display_name);
}
static inline int lvkw_XCloseDisplay(struct LVKW_Context_X11 *ctx, Display *display) {
  return ctx->dlib.x11.CloseDisplay(display);
}
static inline Bool lvkw_XQueryExtension(struct LVKW_Context_X11 *ctx, Display *display,
                                         const char *name, int *major_opcode_return,
                                         int *first_event_return, int *first_error_return) {
  return ctx->dlib.x11.QueryExtension(display, name, major_opcode_return, first_event_return,
                                      first_error_return);
}
static inline Atom lvkw_XInternAtom(struct LVKW_Context_X11 *ctx, Display *display,
                                     const char *atom_name, Bool only_if_exists) {
  return ctx->dlib.x11.InternAtom(display, atom_name, only_if_exists);
}
static inline XContext lvkw_XUniqueContext(struct LVKW_Context_X11 *ctx) {
  return (XContext)ctx->dlib.x11.rmUniqueQuark();
}
static inline Pixmap lvkw_XCreateBitmapFromData(struct LVKW_Context_X11 *ctx, Display *display,
                                                 Drawable d, const char *data, unsigned int width,
                                                 unsigned int height) {
  return ctx->dlib.x11.CreateBitmapFromData(display, d, data, width, height);
}
static inline Cursor lvkw_XCreatePixmapCursor(struct LVKW_Context_X11 *ctx, Display *display,
                                               Pixmap source, Pixmap mask, XColor *foreground_color,
                                               XColor *background_color, unsigned int x,
                                               unsigned int y) {
  return ctx->dlib.x11.CreatePixmapCursor(display, source, mask, foreground_color,
                                          background_color, x, y);
}
static inline int lvkw_XFreePixmap(struct LVKW_Context_X11 *ctx, Display *display, Pixmap pixmap) {
  return ctx->dlib.x11.FreePixmap(display, pixmap);
}
static inline int lvkw_XFreeCursor(struct LVKW_Context_X11 *ctx, Display *display, Cursor cursor) {
  return ctx->dlib.x11.FreeCursor(display, cursor);
}
static inline int lvkw_XPending(struct LVKW_Context_X11 *ctx, Display *display) {
  return ctx->dlib.x11.Pending(display);
}
static inline int lvkw_XNextEvent(struct LVKW_Context_X11 *ctx, Display *display,
                                  XEvent *event_return) {
  return ctx->dlib.x11.NextEvent(display, event_return);
}
static inline int lvkw_XPeekEvent(struct LVKW_Context_X11 *ctx, Display *display,
                                  XEvent *event_return) {
  return ctx->dlib.x11.PeekEvent(display, event_return);
}
static inline int lvkw_XFindContext(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                    XContext context, XPointer *data_return) {
  return ctx->dlib.x11.FindContext(display, w, context, data_return);
}
static inline KeySym lvkw_XLookupKeysym(struct LVKW_Context_X11 *ctx, XKeyEvent *key_event,
                                        int index) {
  return ctx->dlib.x11.LookupKeysym(key_event, index);
}
static inline int lvkw_XWarpPointer(struct LVKW_Context_X11 *ctx, Display *display,
                                    Window src_w, Window dest_w, int src_x, int src_y,
                                    unsigned int src_width, unsigned int src_height, int dest_x,
                                    int dest_y) {
  return ctx->dlib.x11.WarpPointer(display, src_w, dest_w, src_x, src_y, src_width, src_height,
                                   dest_x, dest_y);
}
static inline Window lvkw_XCreateSimpleWindow(struct LVKW_Context_X11 *ctx, Display *display,
                                               Window parent, int x, int y, unsigned int width,
                                               unsigned int height, unsigned int border_width,
                                               unsigned long border, unsigned long background) {
  return ctx->dlib.x11.CreateSimpleWindow(display, parent, x, y, width, height, border_width,
                                          border, background);
}
static inline int lvkw_XStoreName(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                  const char *window_name) {
  return ctx->dlib.x11.StoreName(display, w, window_name);
}
static inline int lvkw_XSelectInput(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                    long event_mask) {
  return ctx->dlib.x11.SelectInput(display, w, event_mask);
}
static inline Status lvkw_XSetWMProtocols(struct LVKW_Context_X11 *ctx, Display *display,
                                           Window w, Atom *protocols, int count) {
  return ctx->dlib.x11.SetWMProtocols(display, w, protocols, count);
}
static inline int lvkw_XSetClassHint(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                     XClassHint *class_hints) {
  return ctx->dlib.x11.SetClassHint(display, w, class_hints);
}
static inline XClassHint *lvkw_XAllocClassHint(struct LVKW_Context_X11 *ctx) {
  (void)ctx;
  return ctx->dlib.x11.AllocClassHint();
}
static inline int lvkw_XFree(struct LVKW_Context_X11 *ctx, void *data) {
  return ctx->dlib.x11.Free(data);
}
static inline int lvkw_XSaveContext(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                    XContext context, const char *data) {
  return ctx->dlib.x11.SaveContext(display, w, context, data);
}
static inline int lvkw_XMapWindow(struct LVKW_Context_X11 *ctx, Display *display, Window w) {
  return ctx->dlib.x11.MapWindow(display, w);
}
static inline int lvkw_XDeleteContext(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                      XContext context) {
  return ctx->dlib.x11.DeleteContext(display, w, context);
}
static inline int lvkw_XDestroyWindow(struct LVKW_Context_X11 *ctx, Display *display, Window w) {
  return ctx->dlib.x11.DestroyWindow(display, w);
}
static inline Status lvkw_XSendEvent(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                      Bool propagate, long event_mask, XEvent *event_send) {
  return ctx->dlib.x11.SendEvent(display, w, propagate, event_mask, event_send);
}
static inline Bool lvkw_XGetEventData(struct LVKW_Context_X11 *ctx, Display *display,
                                       XGenericEventCookie *cookie) {
  return ctx->dlib.x11.GetEventData(display, cookie);
}
static inline void lvkw_XFreeEventData(struct LVKW_Context_X11 *ctx, Display *display,
                                        XGenericEventCookie *cookie) {
  ctx->dlib.x11.FreeEventData(display, cookie);
}
static inline int lvkw_XGrabPointer(struct LVKW_Context_X11 *ctx, Display *display,
                                    Window grab_window, Bool owner_events, unsigned int event_mask,
                                    int pointer_mode, int keyboard_mode, Window confine_to,
                                    Cursor cursor, Time time) {
  return ctx->dlib.x11.GrabPointer(display, grab_window, owner_events, event_mask, pointer_mode,
                                   keyboard_mode, confine_to, cursor, time);
}
static inline int lvkw_XUngrabPointer(struct LVKW_Context_X11 *ctx, Display *display, Time time) {
  return ctx->dlib.x11.UngrabPointer(display, time);
}
static inline int lvkw_XUndefineCursor(struct LVKW_Context_X11 *ctx, Display *display, Window w) {
  return ctx->dlib.x11.UndefineCursor(display, w);
}
static inline int lvkw_XDefineCursor(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                     Cursor cursor) {
  return ctx->dlib.x11.DefineCursor(display, w, cursor);
}
static inline int lvkw_XSetInputFocus(struct LVKW_Context_X11 *ctx, Display *display,
                                      Window focus, int revert_to, Time time) {
  return ctx->dlib.x11.SetInputFocus(display, focus, revert_to, time);
}
static inline XVisualInfo *lvkw_XGetVisualInfo(struct LVKW_Context_X11 *ctx, Display *display,
                                               long vinfo_mask, XVisualInfo *vinfo_template,
                                               int *nitems_return) {
  return ctx->dlib.x11.GetVisualInfo(display, vinfo_mask, vinfo_template, nitems_return);
}
static inline Colormap lvkw_XCreateColormap(struct LVKW_Context_X11 *ctx, Display *display,
                                            Window w, Visual *visual, int alloc) {
  return ctx->dlib.x11.CreateColormap(display, w, visual, alloc);
}
static inline Window lvkw_XCreateWindow(struct LVKW_Context_X11 *ctx, Display *display,
                                         Window parent, int x, int y, unsigned int width,
                                         unsigned int height, unsigned int border_width, int depth,
                                         unsigned int class, Visual *visual,
                                         unsigned long valuemask, XSetWindowAttributes *attributes) {
  return ctx->dlib.x11.CreateWindow(display, parent, x, y, width, height, border_width, depth,
                                    class, visual, valuemask, attributes);
}
static inline int lvkw_XFreeColormap(struct LVKW_Context_X11 *ctx, Display *display,
                                     Colormap colormap) {
  return ctx->dlib.x11.FreeColormap(display, colormap);
}
static inline int (*lvkw_XSetErrorHandler(struct LVKW_Context_X11 *ctx,
                                           int (*handler)(Display *, XErrorEvent *)))(Display *,
                                                                                      XErrorEvent *) {
  return ctx->dlib.x11.SetErrorHandler(handler);
}
static inline int lvkw_XGetErrorText(struct LVKW_Context_X11 *ctx, Display *display, int code,
                                     char *buffer_return, int length) {
  return ctx->dlib.x11.GetErrorText(display, code, buffer_return, length);
}
static inline char *lvkw_XResourceManagerString(struct LVKW_Context_X11 *ctx, Display *display) {
  return ctx->dlib.x11.ResourceManagerString(display);
}
static inline XrmDatabase lvkw_XrmGetStringDatabase(struct LVKW_Context_X11 *ctx,
                                                     const char *data) {
  return ctx->dlib.x11.rmGetStringDatabase(data);
}
static inline Bool lvkw_XrmGetResource(struct LVKW_Context_X11 *ctx, XrmDatabase database,
                                        const char *str_name, const char *str_class,
                                        char **str_type_return, XrmValue *value_return) {
  return ctx->dlib.x11.rmGetResource(database, str_name, str_class, str_type_return, value_return);
}
static inline void lvkw_XrmDestroyDatabase(struct LVKW_Context_X11 *ctx, XrmDatabase database) {
  (void)ctx;
  ctx->dlib.x11.rmDestroyDatabase(database);
}
static inline void lvkw_XrmInitialize(struct LVKW_Context_X11 *ctx) {
  (void)ctx;
  ctx->dlib.x11.rmInitialize();
}
static inline int lvkw_XSync(struct LVKW_Context_X11 *ctx, Display *display, Bool discard) {
  return ctx->dlib.x11.Sync(display, discard);
}
static inline int lvkw_XResizeWindow(struct LVKW_Context_X11 *ctx, Display *display, Window w,
                                     unsigned int width, unsigned int height) {
  return ctx->dlib.x11.ResizeWindow(display, w, width, height);
}
static inline int lvkw_XConnectionNumber(struct LVKW_Context_X11 *ctx, Display *display) {
  return ctx->dlib.x11.GetConnectionNumber(display);
}
static inline Cursor lvkw_XCreateFontCursor(struct LVKW_Context_X11 *ctx, Display *display,
                                            unsigned int shape) {
  return ctx->dlib.x11.CreateFontCursor(display, shape);
}

/* Xcursor helpers */

static inline Cursor lvkw_XcursorLibraryLoadCursor(struct LVKW_Context_X11 *ctx, Display *dpy,
                                                   const char *name) {
  return ctx->dlib.xcursor.LibraryLoadCursor(dpy, name);
}

static inline XcursorImage *lvkw_XcursorImageCreate(struct LVKW_Context_X11 *ctx, int width,
                                                    int height) {
  return ctx->dlib.xcursor.ImageCreate(width, height);
}

static inline Cursor lvkw_XcursorImageLoadCursor(struct LVKW_Context_X11 *ctx, Display *dpy,
                                                 const XcursorImage *image) {
  return ctx->dlib.xcursor.ImageLoadCursor(dpy, image);
}

static inline void lvkw_XcursorImageDestroy(struct LVKW_Context_X11 *ctx, XcursorImage *image) {
  ctx->dlib.xcursor.ImageDestroy(image);
}

/* XInput2 helpers */

static inline Status lvkw_XIQueryVersion(struct LVKW_Context_X11 *ctx, Display *display,
                                         int *major_version_inout, int *minor_version_inout) {
  return ctx->dlib.xi.QueryVersion(display, major_version_inout, minor_version_inout);
}
static inline int lvkw_XISelectEvents(struct LVKW_Context_X11 *ctx, Display *display, Window win,
                                      XIEventMask *masks, int num_masks) {
  return ctx->dlib.xi.SelectEvents(display, win, masks, num_masks);
}
static inline XIDeviceInfo *lvkw_XIQueryDevice(struct LVKW_Context_X11 *ctx, Display *display,
                                               int deviceid, int *num_devices_return) {
  return ctx->dlib.xi.QueryDevice(display, deviceid, num_devices_return);
}
static inline void lvkw_XIFreeDeviceInfo(struct LVKW_Context_X11 *ctx, XIDeviceInfo *info) {
  (void)ctx;
  ctx->dlib.xi.FreeDeviceInfo(info);
}

/* Xlib-xcb helpers */

static inline xcb_connection_t *lvkw_XGetXCBConnection(struct LVKW_Context_X11 *ctx, Display *dpy) {
  return ctx->dlib.x11_xcb.GetXCBConnection(dpy);
}

/* XScreenSaver helpers */

static inline Bool lvkw_XScreenSaverQueryExtension(struct LVKW_Context_X11 *ctx, Display *display,
                                                   int *event_base_return, int *error_base_return) {
  return ctx->dlib.xss.QueryExtension(display, event_base_return, error_base_return);
}
static inline Status lvkw_XScreenSaverQueryInfo(struct LVKW_Context_X11 *ctx, Display *display,
                                                Drawable drawable, XScreenSaverInfo *saver_info) {
  return ctx->dlib.xss.QueryInfo(display, drawable, saver_info);
}
static inline XScreenSaverInfo *lvkw_XScreenSaverAllocInfo(struct LVKW_Context_X11 *ctx) {
  (void)ctx;
  return ctx->dlib.xss.AllocInfo();
}
static inline void lvkw_XScreenSaverSuspend(struct LVKW_Context_X11 *ctx, Display *display,
                                            Bool suspend) {
  ctx->dlib.xss.Suspend(display, suspend);
}

/* Xrandr helpers */

static inline Bool lvkw_XRRQueryExtension(struct LVKW_Context_X11 *ctx, Display *dpy,
                                          int *event_base_return, int *error_base_return) {
  return ctx->dlib.xrandr.QueryExtension(dpy, event_base_return, error_base_return);
}
static inline Status lvkw_XRRQueryVersion(struct LVKW_Context_X11 *ctx, Display *dpy,
                                          int *major_version_return, int *minor_version_return) {
  return ctx->dlib.xrandr.QueryVersion(dpy, major_version_return, minor_version_return);
}
static inline XRRScreenResources *lvkw_XRRGetScreenResourcesCurrent(struct LVKW_Context_X11 *ctx,
                                                                    Display *dpy, Window window) {
  return ctx->dlib.xrandr.GetScreenResourcesCurrent(dpy, window);
}
static inline XRROutputInfo *lvkw_XRRGetOutputInfo(struct LVKW_Context_X11 *ctx, Display *dpy,
                                                   XRRScreenResources *resources, RROutput output) {
  return ctx->dlib.xrandr.GetOutputInfo(dpy, resources, output);
}
static inline XRRCrtcInfo *lvkw_XRRGetCrtcInfo(struct LVKW_Context_X11 *ctx, Display *dpy,
                                               XRRScreenResources *resources, RRCrtc crtc) {
  return ctx->dlib.xrandr.GetCrtcInfo(dpy, resources, crtc);
}
static inline void lvkw_XRRFreeScreenResources(struct LVKW_Context_X11 *ctx,
                                               XRRScreenResources *resources) {
  (void)ctx;
  ctx->dlib.xrandr.FreeScreenResources(resources);
}
static inline void lvkw_XRRFreeOutputInfo(struct LVKW_Context_X11 *ctx, XRROutputInfo *output_info) {
  (void)ctx;
  ctx->dlib.xrandr.FreeOutputInfo(output_info);
}
static inline void lvkw_XRRFreeCrtcInfo(struct LVKW_Context_X11 *ctx, XRRCrtcInfo *crtc_info) {
  (void)ctx;
  ctx->dlib.xrandr.FreeCrtcInfo(crtc_info);
}
static inline void lvkw_XRRSelectInput(struct LVKW_Context_X11 *ctx, Display *dpy, Window window,
                                       int mask) {
  ctx->dlib.xrandr.SelectInput(dpy, window, mask);
}
static inline RROutput lvkw_XRRGetOutputPrimary(struct LVKW_Context_X11 *ctx, Display *dpy,
                                                Window window) {
  return ctx->dlib.xrandr.GetOutputPrimary(dpy, window);
}

/* xkbcommon helpers */

static inline struct xkb_context *lvkw_xkb_context_new(const LVKW_Context_X11 *ctx,
                                                       enum xkb_context_flags flags) {
  return ctx->dlib.xkb.context_new(flags);
}

static inline void lvkw_xkb_context_unref(const LVKW_Context_X11 *ctx, struct xkb_context *context) {
  ctx->dlib.xkb.context_unref(context);
}

static inline struct xkb_keymap *lvkw_xkb_keymap_new_from_string(
    const LVKW_Context_X11 *ctx, struct xkb_context *context, const char *string,
    enum xkb_keymap_format format, enum xkb_keymap_compile_flags flags) {
  return ctx->dlib.xkb.keymap_new_from_string(context, string, format, flags);
}

static inline void lvkw_xkb_keymap_unref(const LVKW_Context_X11 *ctx, struct xkb_keymap *keymap) {
  ctx->dlib.xkb.keymap_unref(keymap);
}

static inline struct xkb_state *lvkw_xkb_state_new(const LVKW_Context_X11 *ctx,
                                                   struct xkb_keymap *keymap) {
  return ctx->dlib.xkb.state_new(keymap);
}

static inline void lvkw_xkb_state_unref(const LVKW_Context_X11 *ctx, struct xkb_state *state) {
  ctx->dlib.xkb.state_unref(state);
}

static inline enum xkb_state_component lvkw_xkb_state_update_mask(
    const LVKW_Context_X11 *ctx, struct xkb_state *state, xkb_mod_mask_t depressed_mods,
    xkb_mod_mask_t latched_mods, xkb_mod_mask_t locked_mods, xkb_layout_index_t depressed_layout,
    xkb_layout_index_t latched_layout, xkb_layout_index_t locked_layout) {
  return ctx->dlib.xkb.state_update_mask(state, depressed_mods, latched_mods, locked_mods,
                                         depressed_layout, latched_layout, locked_layout);
}

static inline xkb_keysym_t lvkw_xkb_state_key_get_one_sym(const LVKW_Context_X11 *ctx,
                                                          struct xkb_state *state,
                                                          xkb_keycode_t key) {
  return ctx->dlib.xkb.state_key_get_one_sym(state, key);
}

static inline int lvkw_xkb_state_mod_name_is_active(const LVKW_Context_X11 *ctx,
                                                    struct xkb_state *state, const char *name,
                                                    enum xkb_state_component type) {
  return ctx->dlib.xkb.state_mod_name_is_active(state, name, type);
}

static inline xkb_mod_index_t lvkw_xkb_keymap_mod_get_index(const LVKW_Context_X11 *ctx,
                                                            struct xkb_keymap *keymap,
                                                            const char *name) {
  return ctx->dlib.xkb.keymap_mod_get_index(keymap, name);
}

static inline xkb_mod_mask_t lvkw_xkb_state_serialize_mods(const LVKW_Context_X11 *ctx,
                                                           struct xkb_state *state,
                                                           enum xkb_state_component type) {
  return ctx->dlib.xkb.state_serialize_mods(state, type);
}

static inline int lvkw_xkb_x11_setup_xkb_extension(
    const LVKW_Context_X11 *ctx, xcb_connection_t *connection, uint16_t major_xkb_version,
    uint16_t minor_xkb_version, enum xkb_x11_setup_xkb_extension_flags flags,
    uint16_t *major_xkb_version_out, uint16_t *minor_xkb_version_out, uint8_t *base_event_out,
    uint8_t *base_error_out) {
  return ctx->dlib.xkb.x11_setup_xkb_extension(connection, major_xkb_version, minor_xkb_version,
                                               flags, major_xkb_version_out, minor_xkb_version_out,
                                               base_event_out, base_error_out);
}

static inline int32_t lvkw_xkb_x11_get_core_keyboard_device_id(const LVKW_Context_X11 *ctx,
                                                               xcb_connection_t *connection) {
  return ctx->dlib.xkb.x11_get_core_keyboard_device_id(connection);
}

static inline struct xkb_keymap *lvkw_xkb_x11_keymap_new_from_device(
    const LVKW_Context_X11 *ctx, struct xkb_context *context, xcb_connection_t *connection,
    int32_t device_id, enum xkb_keymap_compile_flags flags) {
  return ctx->dlib.xkb.x11_keymap_new_from_device(context, connection, device_id, flags);
}

static inline struct xkb_state *lvkw_xkb_x11_state_new_from_device(const LVKW_Context_X11 *ctx,
                                                                   struct xkb_keymap *keymap,
                                                                   xcb_connection_t *connection,
                                                                   int32_t device_id) {
  return ctx->dlib.xkb.x11_state_new_from_device(keymap, connection, device_id);
}

#endif
