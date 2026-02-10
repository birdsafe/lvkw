#ifndef LVKW_MOCK_INTERNAL_H_INCLUDED
#define LVKW_MOCK_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"

typedef struct LVKW_Window_Mock LVKW_Window_Mock;

typedef struct LVKW_Context_Mock {
  LVKW_Context_Base base;
  LVKW_EventQueue event_queue;
  uint32_t idle_timeout_ms;
  bool inhibit_idle;
} LVKW_Context_Mock;

typedef struct LVKW_Window_Mock {
  LVKW_Window_Base base;

  LVKW_Size size;
  LVKW_Size framebuffer_size;
  bool is_fullscreen;
  LVKW_CursorMode cursor_mode;
  LVKW_CursorShape cursor_shape;
  char *title;
  char *app_id;
  bool transparent;
} LVKW_Window_Mock;

LVKW_Status lvkw_ctx_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
void lvkw_ctx_destroy_Mock(LVKW_Context *handle);
void lvkw_ctx_getVkExtensions_Mock(LVKW_Context *ctx, uint32_t *count,
                                                   const char **out_extensions);
LVKW_Status lvkw_ctx_pollEvents_Mock(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_waitEvents_Mock(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_updateAttributes_Mock(LVKW_Context *ctx, uint32_t field_mask,
                                               const LVKW_ContextAttributes *attributes);

LVKW_Status lvkw_ctx_createWindow_Mock(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                           LVKW_Window **out_window);
void lvkw_wnd_destroy_Mock(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_Mock(LVKW_Window *window, VkInstance instance,
                                                   VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getFramebufferSize_Mock(LVKW_Window *window, LVKW_Size *out_size);
LVKW_Status lvkw_wnd_setFullscreen_Mock(LVKW_Window *window, bool enabled);
LVKW_Status lvkw_wnd_setCursorMode_Mock(LVKW_Window *window, LVKW_CursorMode mode);
LVKW_Status lvkw_wnd_setCursorShape_Mock(LVKW_Window *window, LVKW_CursorShape shape);
LVKW_Status lvkw_wnd_updateAttributes_Mock(LVKW_Window *window, uint32_t field_mask,
                                               const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_Mock(LVKW_Window *window);

#define LVKW_BACKEND_MOCK 4

#endif