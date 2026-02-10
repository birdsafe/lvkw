#ifndef LVKW_MOCK_INTERNAL_H_INCLUDED
#define LVKW_MOCK_INTERNAL_H_INCLUDED

#include "lvkw/lvkw.h"
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"

typedef struct LVKW_Window_Mock LVKW_Window_Mock;

typedef struct LVKW_Context_Mock {
  LVKW_Context_Base base;
  LVKW_Window_Mock *window_list_head;
  LVKW_EventQueue event_queue;
  uint32_t idle_timeout_ms;
} LVKW_Context_Mock;

typedef struct LVKW_Window_Mock {
  LVKW_Window_Base base;
  LVKW_Window_Mock *next;

  LVKW_Size size;
  LVKW_Size framebuffer_size;
  bool is_fullscreen;
  LVKW_CursorMode cursor_mode;
  LVKW_CursorShape cursor_shape;
  char *title;
  char *app_id;
} LVKW_Window_Mock;

LVKW_Status lvkw_context_create_Mock(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
void lvkw_context_destroy_Mock(LVKW_Context *handle);
void *lvkw_context_getUserData_Mock(const LVKW_Context *ctx);
void lvkw_context_getVulkanInstanceExtensions_Mock(const LVKW_Context *ctx, uint32_t *count,
                                                   const char **out_extensions);
LVKW_ContextResult lvkw_context_pollEvents_Mock(LVKW_Context *ctx, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata);
LVKW_ContextResult lvkw_context_waitEvents_Mock(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                                 LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_context_setIdleTimeout_Mock(LVKW_Context *ctx, uint32_t timeout_ms);

LVKW_ContextResult lvkw_window_create_Mock(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                           LVKW_Window **out_window);
void lvkw_window_destroy_Mock(LVKW_Window *handle);
LVKW_WindowResult lvkw_window_createVkSurface_Mock(const LVKW_Window *window, VkInstance instance,
                                                   VkSurfaceKHR *out_surface);
LVKW_WindowResult lvkw_window_getFramebufferSize_Mock(const LVKW_Window *window, LVKW_Size *out_size);
void *lvkw_window_getUserData_Mock(const LVKW_Window *window);
LVKW_WindowResult lvkw_window_setFullscreen_Mock(LVKW_Window *window, bool enabled);
LVKW_Status lvkw_window_setCursorMode_Mock(LVKW_Window *window, LVKW_CursorMode mode);
LVKW_Status lvkw_window_setCursorShape_Mock(LVKW_Window *window, LVKW_CursorShape shape);
LVKW_Status lvkw_window_requestFocus_Mock(LVKW_Window *window);

#endif
