#include <stdlib.h>
#include <string.h>

#include "lvkw_api_constraints.h"
#include "lvkw_mem_internal.h"
#include "lvkw_mock_internal.h"

LVKW_Status lvkw_ctx_createWindow_Mock(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                       LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  *out_window_handle = NULL;

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  LVKW_Window_Mock *window = (LVKW_Window_Mock *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Window_Mock));

  if (!window) {
    return LVKW_ERROR;
  }

  memset(window, 0, sizeof(LVKW_Window_Mock));

  window->base.prv.ctx_base = &ctx->base;

  window->base.pub.userdata = create_info->userdata;

  window->size = create_info->attributes.logicalSize;

  window->framebuffer_size.x = (int32_t)create_info->attributes.logicalSize.x;
  window->framebuffer_size.y = (int32_t)create_info->attributes.logicalSize.y;

  window->is_fullscreen = create_info->attributes.fullscreen;
  window->is_maximized = create_info->attributes.maximized;

  window->cursor_mode = LVKW_CURSOR_NORMAL;

  window->cursor = create_info->attributes.cursor;

  window->transparent = create_info->transparent;
  window->accept_dnd = create_info->attributes.accept_dnd;

  window->monitor = create_info->attributes.monitor;

  window->text_input_type = create_info->attributes.text_input_type;
  window->text_input_rect = create_info->attributes.text_input_rect;

  if (create_info->attributes.title) {
    size_t len = strlen(create_info->attributes.title) + 1;

    window->title = (char *)lvkw_context_alloc(&ctx->base, len);

    if (window->title) {
      memcpy(window->title, create_info->attributes.title, len);
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

  *out_window_handle = (LVKW_Window *)window;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_destroy_Mock(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)window->base.prv.ctx_base;

  // Remove from list
  _lvkw_window_list_remove(&ctx->base, &window->base);

  lvkw_event_queue_remove_window_events(&ctx->event_queue, window_handle);

  if (window->title) lvkw_context_free(&ctx->base, window->title);

  if (window->app_id) lvkw_context_free(&ctx->base, window->app_id);

  lvkw_context_free(&ctx->base, window);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_createVkSurface_Mock(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  *out_surface = NULL;

  (void)window_handle;

  (void)instance;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getGeometry_Mock(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  out_geometry->logicalSize = window->size;
  out_geometry->pixelSize = window->framebuffer_size;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_Mock(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setMaximized_Mock(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setCursorMode_Mock(LVKW_Window *window_handle, LVKW_CursorMode mode);
static LVKW_Status _lvkw_wnd_setCursor_Mock(LVKW_Window *window_handle, LVKW_Cursor *cursor);

LVKW_Status lvkw_wnd_update_Mock(LVKW_Window *window_handle, uint32_t field_mask,
                                 const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)window->base.prv.ctx_base;

  if (field_mask & LVKW_WND_ATTR_TITLE) {
    if (window->title) lvkw_context_free(&ctx->base, window->title);
    size_t len = strlen(attributes->title) + 1;
    window->title = (char *)lvkw_context_alloc(&ctx->base, len);
    if (window->title) {
      memcpy(window->title, attributes->title, len);
    }
  }

  if (field_mask & LVKW_WND_ATTR_LOGICAL_SIZE) {
    window->size = attributes->logicalSize;
    window->framebuffer_size.x = (int32_t)attributes->logicalSize.x;
    window->framebuffer_size.y = (int32_t)attributes->logicalSize.y;
  }

  if (field_mask & LVKW_WND_ATTR_FULLSCREEN) {
    _lvkw_wnd_setFullscreen_Mock(window_handle, attributes->fullscreen);
  }

  if (field_mask & LVKW_WND_ATTR_MAXIMIZED) {
    _lvkw_wnd_setMaximized_Mock(window_handle, attributes->maximized);
  }

  if (field_mask & LVKW_WND_ATTR_CURSOR_MODE) {
    _lvkw_wnd_setCursorMode_Mock(window_handle, attributes->cursor_mode);
  }

  if (field_mask & LVKW_WND_ATTR_CURSOR) {
    _lvkw_wnd_setCursor_Mock(window_handle, attributes->cursor);
  }

  if (field_mask & LVKW_WND_ATTR_MONITOR) {
    window->monitor = attributes->monitor;
  }

  if (field_mask & LVKW_WND_ATTR_ACCEPT_DND) {
    window->accept_dnd = attributes->accept_dnd;
  }

  if (field_mask & LVKW_WND_ATTR_TEXT_INPUT_TYPE) {
    window->text_input_type = attributes->text_input_type;
  }

  if (field_mask & LVKW_WND_ATTR_TEXT_INPUT_RECT) {
    window->text_input_rect = attributes->text_input_rect;
  }

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_Mock(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  window->is_fullscreen = enabled;

  // Push a resize event if fullscreen changed?

  LVKW_Event ev = {0};

  ev.resized.geometry.logicalSize = window->size;
  ev.resized.geometry.pixelSize = window->framebuffer_size;

  lvkw_event_queue_push(window->base.prv.ctx_base, &((LVKW_Context_Mock *)window->base.prv.ctx_base)->event_queue, LVKW_EVENT_TYPE_WINDOW_RESIZED, window_handle, &ev);

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setMaximized_Mock(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  if (window->is_maximized == enabled) return LVKW_SUCCESS;

  window->is_maximized = enabled;

  if (enabled)
    window->base.pub.flags |= LVKW_WND_STATE_MAXIMIZED;
  else
    window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_MAXIMIZED;

  LVKW_Event ev = {0};
  ev.maximized.maximized = enabled;

  lvkw_event_queue_push(window->base.prv.ctx_base, &((LVKW_Context_Mock *)window->base.prv.ctx_base)->event_queue, LVKW_EVENT_TYPE_WINDOW_MAXIMIZED, window_handle, &ev);

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursorMode_Mock(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  window->cursor_mode = mode;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursor_Mock(LVKW_Window *window_handle, LVKW_Cursor *cursor) {
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  window->cursor = cursor;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_requestFocus_Mock(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_Mock *window = (LVKW_Window_Mock *)window_handle;

  (void)window;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setClipboardText_Mock(LVKW_Window *window, const char *text) {
  LVKW_API_VALIDATE(wnd_setClipboardText, window, text);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardText_Mock(LVKW_Window *window, const char **out_text) {
  LVKW_API_VALIDATE(wnd_getClipboardText, window, out_text);
  if (out_text) *out_text = "";
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_setClipboardData_Mock(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  LVKW_API_VALIDATE(wnd_setClipboardData, window, data, count);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardData_Mock(LVKW_Window *window, const char *mime_type, const void **out_data,
                                           size_t *out_size) {
  LVKW_API_VALIDATE(wnd_getClipboardData, window, mime_type, out_data, out_size);
  if (out_data) *out_data = NULL;
  if (out_size) *out_size = 0;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_Mock(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  LVKW_API_VALIDATE(wnd_getClipboardMimeTypes, window, out_mime_types, count);
  if (count) *count = 0;
  return LVKW_SUCCESS;
}

void lvkw_mock_markWindowReady(LVKW_Window *window) {
  LVKW_Window_Mock *wnd = (LVKW_Window_Mock *)window;

  LVKW_Event ev = {0};

  lvkw_event_queue_push(wnd->base.prv.ctx_base, &((LVKW_Context_Mock *)wnd->base.prv.ctx_base)->event_queue, LVKW_EVENT_TYPE_WINDOW_READY, window, &ev);
}

LVKW_Status lvkw_ctx_createCursor_Mock(LVKW_Context *ctx_handle, const LVKW_CursorCreateInfo *create_info,
                                       LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx_handle, create_info, out_cursor);
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)ctx_handle;

  LVKW_Cursor_Mock *cursor = (LVKW_Cursor_Mock *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Cursor_Mock));
  if (!cursor) return LVKW_ERROR;

  memset(cursor, 0, sizeof(*cursor));
  cursor->base.pub.flags = 0;
  cursor->base.prv.ctx_base = &ctx->base;
  cursor->shape = (LVKW_CursorShape)0;

  *out_cursor = (LVKW_Cursor *)cursor;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_cursor_destroy_Mock(LVKW_Cursor *cursor_handle) {
  LVKW_API_VALIDATE(cursor_destroy, cursor_handle);
  if (cursor_handle->flags & LVKW_CURSOR_FLAG_SYSTEM) return LVKW_SUCCESS;

  LVKW_Cursor_Mock *cursor = (LVKW_Cursor_Mock *)cursor_handle;
  LVKW_Context_Mock *ctx = (LVKW_Context_Mock *)cursor->base.prv.ctx_base;

  lvkw_context_free(&ctx->base, cursor);
  return LVKW_SUCCESS;
}
