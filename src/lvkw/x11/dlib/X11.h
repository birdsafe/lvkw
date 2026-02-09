#ifndef LVKW_X11_DLIB_X11_H_INCLUDED
#define LVKW_X11_DLIB_X11_H_INCLUDED

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#include "lvkw_internal.h"

#define LVKW_X11_FUNCTIONS_TABLE                             \
  LVKW_LIB_FN(OpenDisplay, XOpenDisplay)                     \
  LVKW_LIB_FN(CloseDisplay, XCloseDisplay)                   \
  LVKW_LIB_FN(QueryExtension, XQueryExtension)               \
  LVKW_LIB_FN(InternAtom, XInternAtom)                       \
  LVKW_LIB_FN(rmUniqueQuark, XrmUniqueQuark)                 \
  LVKW_LIB_FN(CreateBitmapFromData, XCreateBitmapFromData)   \
  LVKW_LIB_FN(CreatePixmapCursor, XCreatePixmapCursor)       \
  LVKW_LIB_FN(FreePixmap, XFreePixmap)                       \
  LVKW_LIB_FN(FreeCursor, XFreeCursor)                       \
  LVKW_LIB_FN(Pending, XPending)                             \
  LVKW_LIB_FN(NextEvent, XNextEvent)                         \
  LVKW_LIB_FN(PeekEvent, XPeekEvent)                         \
  LVKW_LIB_FN(FindContext, XFindContext)                     \
  LVKW_LIB_FN(LookupKeysym, XLookupKeysym)                   \
  LVKW_LIB_FN(WarpPointer, XWarpPointer)                     \
  LVKW_LIB_FN(CreateSimpleWindow, XCreateSimpleWindow)       \
  LVKW_LIB_FN(StoreName, XStoreName)                         \
  LVKW_LIB_FN(SelectInput, XSelectInput)                     \
  LVKW_LIB_FN(SetWMProtocols, XSetWMProtocols)               \
  LVKW_LIB_FN(SetClassHint, XSetClassHint)                   \
  LVKW_LIB_FN(AllocClassHint, XAllocClassHint)               \
  LVKW_LIB_FN(Free, XFree)                                   \
  LVKW_LIB_FN(SaveContext, XSaveContext)                     \
  LVKW_LIB_FN(MapWindow, XMapWindow)                         \
  LVKW_LIB_FN(DeleteContext, XDeleteContext)                 \
  LVKW_LIB_FN(DestroyWindow, XDestroyWindow)                 \
  LVKW_LIB_FN(SendEvent, XSendEvent)                         \
  LVKW_LIB_FN(GetEventData, XGetEventData)                   \
  LVKW_LIB_FN(FreeEventData, XFreeEventData)                 \
  LVKW_LIB_FN(GrabPointer, XGrabPointer)                     \
  LVKW_LIB_FN(UngrabPointer, XUngrabPointer)                 \
  LVKW_LIB_FN(UndefineCursor, XUndefineCursor)               \
  LVKW_LIB_FN(DefineCursor, XDefineCursor)                   \
  LVKW_LIB_FN(SetInputFocus, XSetInputFocus)                 \
  LVKW_LIB_FN(GetVisualInfo, XGetVisualInfo)                 \
  LVKW_LIB_FN(CreateColormap, XCreateColormap)               \
  LVKW_LIB_FN(CreateWindow, XCreateWindow)                   \
  LVKW_LIB_FN(FreeColormap, XFreeColormap)                   \
  LVKW_LIB_FN(SetErrorHandler, XSetErrorHandler)             \
  LVKW_LIB_FN(GetErrorText, XGetErrorText)                   \
  LVKW_LIB_FN(ResourceManagerString, XResourceManagerString) \
  LVKW_LIB_FN(rmGetStringDatabase, XrmGetStringDatabase)     \
  LVKW_LIB_FN(rmGetResource, XrmGetResource)                 \
  LVKW_LIB_FN(rmDestroyDatabase, XrmDestroyDatabase)         \
  LVKW_LIB_FN(rmInitialize, XrmInitialize)                   \
  LVKW_LIB_FN(Sync, XSync)                                   \
  /* End of table */

#define XOpenDisplay(...) _lvkw_lib_x11.OpenDisplay(__VA_ARGS__)
#define XCloseDisplay(...) _lvkw_lib_x11.CloseDisplay(__VA_ARGS__)
#define XQueryExtension(...) _lvkw_lib_x11.QueryExtension(__VA_ARGS__)
#define XInternAtom(...) _lvkw_lib_x11.InternAtom(__VA_ARGS__)

typedef struct _LVKW_Lib_X11 {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) typeof(sym) *name;
  LVKW_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} _LVKW_Lib_X11;

extern _LVKW_Lib_X11 _lvkw_lib_x11;

#ifdef XUniqueContext
#undef XUniqueContext
#endif

#define XOpenDisplay(...) _lvkw_lib_x11.OpenDisplay(__VA_ARGS__)
#define XCloseDisplay(...) _lvkw_lib_x11.CloseDisplay(__VA_ARGS__)
#define XQueryExtension(...) _lvkw_lib_x11.QueryExtension(__VA_ARGS__)
#define XInternAtom(...) _lvkw_lib_x11.InternAtom(__VA_ARGS__)
#define XUniqueContext() (XContext) _lvkw_lib_x11.rmUniqueQuark()
#define XCreateBitmapFromData(...) _lvkw_lib_x11.CreateBitmapFromData(__VA_ARGS__)
#define XCreatePixmapCursor(...) _lvkw_lib_x11.CreatePixmapCursor(__VA_ARGS__)
#define XFreePixmap(...) _lvkw_lib_x11.FreePixmap(__VA_ARGS__)
#define XFreeCursor(...) _lvkw_lib_x11.FreeCursor(__VA_ARGS__)
#define XPending(...) _lvkw_lib_x11.Pending(__VA_ARGS__)
#define XNextEvent(...) _lvkw_lib_x11.NextEvent(__VA_ARGS__)
#define XPeekEvent(...) _lvkw_lib_x11.PeekEvent(__VA_ARGS__)
#define XFindContext(...) _lvkw_lib_x11.FindContext(__VA_ARGS__)
#define XLookupKeysym(...) _lvkw_lib_x11.LookupKeysym(__VA_ARGS__)
#define XWarpPointer(...) _lvkw_lib_x11.WarpPointer(__VA_ARGS__)
#define XCreateSimpleWindow(...) _lvkw_lib_x11.CreateSimpleWindow(__VA_ARGS__)
#define XStoreName(...) _lvkw_lib_x11.StoreName(__VA_ARGS__)
#define XSelectInput(...) _lvkw_lib_x11.SelectInput(__VA_ARGS__)
#define XSetWMProtocols(...) _lvkw_lib_x11.SetWMProtocols(__VA_ARGS__)
#define XSetClassHint(...) _lvkw_lib_x11.SetClassHint(__VA_ARGS__)
#define XAllocClassHint(...) _lvkw_lib_x11.AllocClassHint(__VA_ARGS__)
#define XFree(...) _lvkw_lib_x11.Free(__VA_ARGS__)
#define XSaveContext(...) _lvkw_lib_x11.SaveContext(__VA_ARGS__)
#define XMapWindow(...) _lvkw_lib_x11.MapWindow(__VA_ARGS__)
#define XDeleteContext(...) _lvkw_lib_x11.DeleteContext(__VA_ARGS__)
#define XDestroyWindow(...) _lvkw_lib_x11.DestroyWindow(__VA_ARGS__)
#define XSendEvent(...) _lvkw_lib_x11.SendEvent(__VA_ARGS__)
#define XGetEventData(...) _lvkw_lib_x11.GetEventData(__VA_ARGS__)
#define XFreeEventData(...) _lvkw_lib_x11.FreeEventData(__VA_ARGS__)
#define XGrabPointer(...) _lvkw_lib_x11.GrabPointer(__VA_ARGS__)
#define XUngrabPointer(...) _lvkw_lib_x11.UngrabPointer(__VA_ARGS__)
#define XUndefineCursor(...) _lvkw_lib_x11.UndefineCursor(__VA_ARGS__)
#define XDefineCursor(...) _lvkw_lib_x11.DefineCursor(__VA_ARGS__)
#define XSetInputFocus(...) _lvkw_lib_x11.SetInputFocus(__VA_ARGS__)
#define XGetVisualInfo(...) _lvkw_lib_x11.GetVisualInfo(__VA_ARGS__)
#define XCreateColormap(...) _lvkw_lib_x11.CreateColormap(__VA_ARGS__)
#define XCreateWindow(...) _lvkw_lib_x11.CreateWindow(__VA_ARGS__)
#define XFreeColormap(...) _lvkw_lib_x11.FreeColormap(__VA_ARGS__)
#define XSetErrorHandler(...) _lvkw_lib_x11.SetErrorHandler(__VA_ARGS__)
#define XGetErrorText(...) _lvkw_lib_x11.GetErrorText(__VA_ARGS__)
#define XResourceManagerString(...) _lvkw_lib_x11.ResourceManagerString(__VA_ARGS__)
#define XrmGetStringDatabase(...) _lvkw_lib_x11.rmGetStringDatabase(__VA_ARGS__)
#define XrmGetResource(...) _lvkw_lib_x11.rmGetResource(__VA_ARGS__)
#define XrmDestroyDatabase(...) _lvkw_lib_x11.rmDestroyDatabase(__VA_ARGS__)
#define XrmInitialize(...) _lvkw_lib_x11.rmInitialize(__VA_ARGS__)
#define XSync(...) _lvkw_lib_x11.Sync(__VA_ARGS__)

#endif