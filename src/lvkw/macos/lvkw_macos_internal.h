#ifndef LVKW_MACOS_INTERNAL_H_INCLUDED
#define LVKW_MACOS_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

// Minimal stubs for internal structures
typedef struct LVKW_Context_Cocoa {
  LVKW_Context_Base base;
} LVKW_Context_Cocoa;

typedef struct LVKW_Window_Cocoa {
  LVKW_Window_Base base;
} LVKW_Window_Cocoa;

// Backend function prototypes
LVKW_Status lvkw_ctx_create_Cocoa(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
LVKW_Status lvkw_ctx_destroy_Cocoa(LVKW_Context *handle);
LVKW_Status lvkw_ctx_getVkExtensions_Cocoa(LVKW_Context *ctx, uint32_t *count,
                                           const char *const **out_extensions);
LVKW_Status lvkw_ctx_pollEvents_Cocoa(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                      void *userdata);
LVKW_Status lvkw_ctx_waitEvents_Cocoa(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_update_Cocoa(LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_Cocoa(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors, uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_Cocoa(LVKW_Context *ctx, LVKW_MonitorId monitor,
                                           LVKW_VideoMode *out_modes, uint32_t *count);

LVKW_Status lvkw_ctx_createWindow_Cocoa(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window);
LVKW_Status lvkw_wnd_destroy_Cocoa(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_Cocoa(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_Cocoa(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_Cocoa(LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_Cocoa(LVKW_Window *window);

#endif  // LVKW_MACOS_INTERNAL_H_INCLUDED
