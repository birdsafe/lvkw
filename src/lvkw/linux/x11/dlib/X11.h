// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

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
  LVKW_LIB_FN(ResizeWindow, XResizeWindow)                   \
  /* End of table */

typedef struct LVKW_Lib_X11 {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name, sym) __typeof__(sym)* name;
  LVKW_X11_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_X11;

#endif