// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_MACOS_INTERNAL_H_INCLUDED
#define LVKW_MACOS_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"
#include "lvkw_event_queue.h"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

// Minimal stubs for internal structures
typedef struct LVKW_Context_Cocoa {
  LVKW_Context_Base base;
#ifdef __OBJC__
  NSApplication *app;
#else
  void *app;
#endif
#ifdef __OBJC__
  NSEventModifierFlags last_modifiers;
#else
  uint64_t last_modifiers;
#endif
} LVKW_Context_Cocoa;

typedef struct LVKW_Window_Cocoa {
  LVKW_Window_Base base;
#ifdef __OBJC__
  NSWindow *window;
  NSView *view;
  CAMetalLayer *layer;
#else
  void *window;
  void *view;
  void *layer;
#endif
} LVKW_Window_Cocoa;

// Backend function prototypes
LVKW_Status lvkw_ctx_create_Cocoa(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
LVKW_Status lvkw_ctx_destroy_Cocoa(LVKW_Context *handle);
LVKW_Status lvkw_ctx_getVkExtensions_Cocoa(LVKW_Context *ctx, uint32_t *count,
                                           const char *const **out_extensions);
LVKW_Status lvkw_ctx_syncEvents_Cocoa(LVKW_Context *ctx, uint32_t timeout_ms);
LVKW_Status lvkw_ctx_postEvent_Cocoa(LVKW_Context *ctx, LVKW_EventType type, LVKW_Window *window,
                                     const LVKW_Event *evt);
LVKW_Status lvkw_ctx_scanEvents_Cocoa(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                      void *userdata);
LVKW_Status lvkw_ctx_update_Cocoa(LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_Cocoa(LVKW_Context *ctx, LVKW_Monitor **out_monitors, uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_Cocoa(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                           LVKW_VideoMode *out_modes, uint32_t *count);
LVKW_Status lvkw_ctx_getMetrics_Cocoa(LVKW_Context *ctx, LVKW_MetricsCategory category, void *out_data,
                                         bool reset);

LVKW_Status lvkw_ctx_createWindow_Cocoa(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window);
LVKW_Status lvkw_wnd_destroy_Cocoa(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_Cocoa(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_Cocoa(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_Cocoa(LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_Cocoa(LVKW_Window *window);
LVKW_Status lvkw_wnd_setClipboardText_Cocoa(LVKW_Window *window, const char *text);
LVKW_Status lvkw_wnd_getClipboardText_Cocoa(LVKW_Window *window, const char **out_text);
LVKW_Status lvkw_wnd_setClipboardData_Cocoa(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count);
LVKW_Status lvkw_wnd_getClipboardData_Cocoa(LVKW_Window *window, const char *mime_type, const void **out_data,
                                            size_t *out_size);
LVKW_Status lvkw_wnd_getClipboardMimeTypes_Cocoa(LVKW_Window *window, const char ***out_mime_types, uint32_t *count);

#endif  // LVKW_MACOS_INTERNAL_H_INCLUDED
