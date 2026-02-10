#include <stdlib.h>
#include <string.h>

#include "lvkw_api_checks.h"
#include "lvkw_mock_internal.h"

LVKW_Status lvkw_ctx_createWindow_Mock(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                           LVKW_Window **out_window_handle) {
  *out_window_handle = NULL;

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  LVKW_Window_Mock *window = (LVKW_Window_Mock *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Window_Mock));

  if (!window) {
    return LVKW_ERROR;
  }

  memset(window, 0, sizeof(LVKW_Window_Mock));

  window->base.prv.ctx_base = &ctx->base;

  window->base.pub.userdata = create_info->userdata;

  window->size = create_info->size;

  window->framebuffer_size = create_info->size;

  window->cursor_mode = LVKW_CURSOR_NORMAL;

  window->cursor_shape = LVKW_CURSOR_SHAPE_DEFAULT;

  if (create_info->title) {
    size_t len = strlen(create_info->title) + 1;

    window->title = (char *)lvkw_context_alloc(&ctx->base, len);

    if (window->title) {
      memcpy(window->title, create_info->title, len);
    }
  }

  if (create_info->app_id) {
    size_t len = strlen(create_info->app_id) + 1;

    window->app_id = (char *)lvkw_context_alloc(&ctx->base, len);

    if (window->app_id) {
      memcpy(window->app_id, create_info->app_id, len);
    }
  }

  // Add to list
  _lvkw_window_list_add(&ctx->base, &window->base);

  // Push a window ready event

  LVKW_Event ev = {0};

  ev.type = LVKW_EVENT_TYPE_WINDOW_READY;

  ev.window = (LVKW_Window *)window;

  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, &ev);

  *out_window_handle = (LVKW_Window *)window;

  return LVKW_SUCCESS;
}

void lvkw_wnd_destroy_Mock(LVKW_Window *window_handle) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)window->base.prv.ctx_base;

  // Remove from list
  _lvkw_window_list_remove(&ctx->base, &window->base);

  lvkw_event_queue_remove_window_events(&ctx->event_queue, window_handle);

  if (window->title) lvkw_context_free(&ctx->base, window->title);

  if (window->app_id) lvkw_context_free(&ctx->base, window->app_id);

  lvkw_context_free(&ctx->base, window);
}

LVKW_Status lvkw_wnd_createVkSurface_Mock(LVKW_Window *window_handle, VkInstance instance,
                                                   VkSurfaceKHR *out_surface) {
  *out_surface = NULL;

  (void)window_handle;

  (void)instance;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getFramebufferSize_Mock(LVKW_Window *window_handle, LVKW_Size *out_size) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  *out_size = window->framebuffer_size;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setFullscreen_Mock(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  window->is_fullscreen = enabled;

  // Push a resize event if fullscreen changed?

  LVKW_Event ev = {0};

  ev.type = LVKW_EVENT_TYPE_WINDOW_RESIZED;

  ev.window = (LVKW_Window *)window;

  ev.resized.size = window->size;

  ev.resized.framebufferSize = window->framebuffer_size;

  lvkw_event_queue_push(window->base.prv.ctx_base, &((LVKW_Context_Mock *)window->base.prv.ctx_base)->event_queue, &ev);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setCursorMode_Mock(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  window->cursor_mode = mode;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setCursorShape_Mock(LVKW_Window *window_handle, LVKW_CursorShape shape) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  window->cursor_shape = shape;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_requestFocus_Mock(LVKW_Window *window_handle) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  (void)window;

  return LVKW_SUCCESS;
}