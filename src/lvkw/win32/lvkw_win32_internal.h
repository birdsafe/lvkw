#ifndef LVKW_WIN32_INTERNAL_H_INCLUDED
#define LVKW_WIN32_INTERNAL_H_INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

// Minimal stubs for internal structures
typedef struct LVKW_Context_Win32 {
  LVKW_Context_Base base;
} LVKW_Context_Win32;

typedef struct LVKW_Window_Win32 {
  LVKW_Window_Base base;
} LVKW_Window_Win32;

// Backend function prototypes
LVKW_Status lvkw_ctx_create_Win32(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
void lvkw_ctx_destroy_Win32(LVKW_Context *handle);
const char *const *lvkw_ctx_getVkExtensions_Win32(LVKW_Context *ctx, uint32_t *count);
LVKW_Status lvkw_ctx_pollEvents_Win32(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                      void *userdata);
LVKW_Status lvkw_ctx_waitEvents_Win32(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_update_Win32(LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes);

LVKW_Status lvkw_ctx_createWindow_Win32(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window);
void lvkw_wnd_destroy_Win32(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_Win32(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_Win32(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_Win32(LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_Win32(LVKW_Window *window);
void lvkw_ctx_assertThread_Win32(LVKW_Context *ctx_handle);

#endif  // LVKW_WIN32_INTERNAL_H_INCLUDED
