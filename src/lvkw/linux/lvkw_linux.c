// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_internal.h"

// Forward declarations for backend-specific creation functions
LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info,
                               LVKW_Context **out_context);
LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info,
                                LVKW_Context **out_context);

#include <time.h>

uint64_t _lvkw_get_timestamp_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info,
                                     LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;

  LVKW_BackendType backend = create_info->backend;

  if (backend == LVKW_BACKEND_WAYLAND || backend == LVKW_BACKEND_AUTO) {
    LVKW_Status res = lvkw_ctx_create_WL(create_info, out_ctx_handle);
    if (res == LVKW_SUCCESS) {
      return res;
    }
  }

  if (backend == LVKW_BACKEND_X11 || backend == LVKW_BACKEND_AUTO) {
    return lvkw_ctx_create_X11(create_info, out_ctx_handle);
  }

  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_destroy(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.destroy(ctx_handle);
}

LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                     const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_vulkan_instance_extensions(ctx_handle, count,
                                                                       out_extensions);
}

LVKW_Status lvkw_ctx_syncEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_syncEvents, ctx_handle, timeout_ms);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.sync_events(ctx_handle, timeout_ms);
}

LVKW_Status lvkw_ctx_postEvent(LVKW_Context *ctx_handle, LVKW_EventType type,
                               LVKW_Window *window, const LVKW_Event *evt) {
  LVKW_API_VALIDATE(ctx_postEvent, ctx_handle, type, window, evt);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.post_event(ctx_handle, type, window, evt);
}

LVKW_Status lvkw_ctx_scanEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_scanEvents, ctx_handle, event_mask, callback, userdata);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.scan_events(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask,
                            const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.update(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx_handle, LVKW_Monitor **out_monitors,
                                 uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_monitors, count);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_monitors(ctx_handle, out_monitors, count);
}

LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_monitor_modes(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_ctx_getMetrics(LVKW_Context *ctx_handle, LVKW_MetricsCategory category,
                                  void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx_handle, category, out_data, reset);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_metrics(ctx_handle, category, out_data, reset);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle,
                                  const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  *out_window_handle = NULL;
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->window.create(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_wnd_destroy(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.destroy(window_handle);
}

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                     VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  *out_surface = NULL;
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.create_vk_surface(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.get_geometry(window_handle, out_geometry);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask,
                            const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.update(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.request_focus(window_handle);
}

LVKW_Status lvkw_ctx_getStandardCursor(LVKW_Context *ctx_handle, LVKW_CursorShape shape,
                                       LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_getStandardCursor, ctx_handle, shape, out_cursor);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->cursor.get_standard(ctx_handle, shape, out_cursor);
}

LVKW_Status lvkw_ctx_createCursor(LVKW_Context *ctx_handle,
                                  const LVKW_CursorCreateInfo *create_info,
                                  LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx_handle, create_info, out_cursor);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->cursor.create(ctx_handle, create_info, out_cursor);
}

LVKW_Status lvkw_cursor_destroy(LVKW_Cursor *cursor) {
  LVKW_API_VALIDATE(cursor_destroy, cursor);
  LVKW_Cursor_Base *cursor_base = (LVKW_Cursor_Base *)cursor;
  return cursor_base->prv.backend->cursor.destroy(cursor);
}

LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_API_VALIDATE(wnd_setClipboardText, window, text);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window;
  return window_base->prv.backend->window.set_clipboard_text(window, text);
}

LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_API_VALIDATE(wnd_getClipboardText, window, out_text);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window;
  return window_base->prv.backend->window.get_clipboard_text(window, out_text);
}

LVKW_Status lvkw_wnd_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data,
                                      uint32_t count) {
  LVKW_API_VALIDATE(wnd_setClipboardData, window, data, count);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window;
  return window_base->prv.backend->window.set_clipboard_data(window, data, count);
}

LVKW_Status lvkw_wnd_getClipboardData(LVKW_Window *window, const char *mime_type,
                                      const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(wnd_getClipboardData, window, mime_type, out_data, out_size);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window;
  return window_base->prv.backend->window.get_clipboard_data(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types,
                                           uint32_t *count) {
  LVKW_API_VALIDATE(wnd_getClipboardMimeTypes, window, out_mime_types, count);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window;
  return window_base->prv.backend->window.get_clipboard_mime_types(window, out_mime_types, count);
}

#ifdef LVKW_ENABLE_CONTROLLER
#include "lvkw_linux_internal.h"

LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  LVKW_API_VALIDATE(ctrl_create, ctx, id, out_controller);
  return lvkw_ctrl_create_Linux(ctx, id, out_controller);
}

LVKW_Status lvkw_ctrl_destroy(LVKW_Controller *ctrl) {
  LVKW_API_VALIDATE(ctrl_destroy, ctrl);
  return lvkw_ctrl_destroy_Linux(ctrl);
}

LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_API_VALIDATE(ctrl_getInfo, controller, out_info);
  return lvkw_ctrl_getInfo_Linux(controller, out_info);
}

LVKW_Status lvkw_ctrl_list(LVKW_Context *ctx, LVKW_CtrlId *out_ids, uint32_t *out_count) {
  LVKW_API_VALIDATE(ctrl_list, ctx, out_ids, out_count);
  return lvkw_ctrl_list_Linux(ctx, out_ids, out_count);
}

LVKW_Status lvkw_ctrl_setHapticLevels(LVKW_Controller *controller, uint32_t first_haptic,
                                      uint32_t count, const LVKW_Scalar *intensities) {
  LVKW_API_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);
  return lvkw_ctrl_setHapticLevels_Linux(controller, first_haptic, count, intensities);
}
#endif
