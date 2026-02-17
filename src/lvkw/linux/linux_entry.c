// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "api_constraints.h"
#include "internal.h"

// Forward declarations for backend-specific creation functions
LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info,
                               LVKW_Context **out_context);
LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info,
                                LVKW_Context **out_context);

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info,
                                     LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;

  LVKW_BackendType backend = create_info->backend;

  if (backend == LVKW_BACKEND_WAYLAND || backend == LVKW_BACKEND_AUTO) {
    const LVKW_ContextCreateInfo *wl_create_info = create_info;

    // Prevent the wayland startup from spamming diagnostics when we are just probing it.
#ifdef LVKW_INDIRECT_BACKEND
    LVKW_ContextCreateInfo create_info_copy;
    if (backend == LVKW_BACKEND_AUTO && create_info->attributes.diagnostic_cb) {
      create_info_copy = *create_info;
      create_info_copy.attributes.diagnostic_cb = NULL;
      wl_create_info = &create_info_copy;
    }
#endif

    LVKW_Status res = lvkw_ctx_create_WL(wl_create_info, out_ctx_handle);
    if (res == LVKW_SUCCESS) {
      if (backend == LVKW_BACKEND_AUTO) {
        LVKW_REPORT_CTX_DIAGNOSTIC((LVKW_Context_Base *)*out_ctx_handle, LVKW_DIAGNOSTIC_INFO,
                                   "Selected backend: Wayland");
      }
      return res;
    }
  }

  if (backend == LVKW_BACKEND_X11 || backend == LVKW_BACKEND_AUTO) {
    LVKW_Status res = lvkw_ctx_create_X11(create_info, out_ctx_handle);
    if (res == LVKW_SUCCESS && backend == LVKW_BACKEND_AUTO) {
      LVKW_REPORT_CTX_DIAGNOSTIC((LVKW_Context_Base *)*out_ctx_handle, LVKW_DIAGNOSTIC_INFO,
                                 "Selected backend: X11");
    }
    return res;
  }

  return LVKW_ERROR;
}

LVKW_Status lvkw_context_destroy(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.destroy(ctx_handle);
}

LVKW_Status lvkw_display_listVkExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                     const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_vulkan_instance_extensions(ctx_handle, count,
                                                                       out_extensions);
}

LVKW_Status lvkw_events_pump(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_API_VALIDATE(ctx_pumpEvents, ctx_handle, timeout_ms);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.pump_events(ctx_handle, timeout_ms);
}

LVKW_Status lvkw_events_commit(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_commitEvents, ctx_handle);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.commit_events(ctx_handle);
}

LVKW_Status _lvkw_ctx_post_backend(LVKW_Context *ctx_handle, LVKW_EventType type,
                                   LVKW_Window *window, const LVKW_Event *evt) {
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.post_event(ctx_handle, type, window, evt);
}

LVKW_Status lvkw_events_scan(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_scanEvents, ctx_handle, event_mask, callback, userdata);
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.scan_events(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_context_update(LVKW_Context *ctx_handle, uint32_t field_mask,
                            const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.update(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_display_listMonitors(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs,
                                 uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_refs, count);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_monitors(ctx_handle, out_refs, count);
}

LVKW_Status lvkw_display_createMonitor(LVKW_MonitorRef *monitor_ref, LVKW_Monitor **out_monitor) {
  LVKW_API_VALIDATE(monitor_createRef, monitor_ref, out_monitor);
  LVKW_Monitor_Base *monitor_base = (LVKW_Monitor_Base *)monitor_ref;
  monitor_base->prv.user_refcount++;
  *out_monitor = &monitor_base->pub;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_destroyMonitor(LVKW_Monitor *monitor) {
  LVKW_API_VALIDATE(monitor_destroy, monitor);
  LVKW_Monitor_Base *monitor_base = (LVKW_Monitor_Base *)monitor;
  monitor_base->prv.user_refcount--;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_display_listMonitorModes(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_monitor_modes(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_instrumentation_getMetrics(LVKW_Context *ctx_handle, LVKW_MetricsCategory category,
                                  void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx_handle, category, out_data, reset);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_metrics(ctx_handle, category, out_data, reset);
}

LVKW_Status lvkw_display_createWindow(LVKW_Context *ctx_handle,
                                  const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  *out_window_handle = NULL;
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->window.create(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_display_destroyWindow(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.destroy(window_handle);
}

LVKW_Status lvkw_display_createVkSurface(LVKW_Window *window_handle, VkInstance instance,
                                     VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  *out_surface = NULL;
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.create_vk_surface(window_handle, instance, out_surface);
}

LVKW_Status lvkw_display_getWindowGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.get_geometry(window_handle, out_geometry);
}

LVKW_Status lvkw_display_updateWindow(LVKW_Window *window_handle, uint32_t field_mask,
                            const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.update(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_display_requestWindowFocus(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.request_focus(window_handle);
}

LVKW_Status lvkw_display_getStandardCursor(LVKW_Context *ctx_handle, LVKW_CursorShape shape,
                                       LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_getStandardCursor, ctx_handle, shape, out_cursor);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->cursor.get_standard(ctx_handle, shape, out_cursor);
}

LVKW_Status lvkw_display_createCursor(LVKW_Context *ctx_handle,
                                  const LVKW_CursorCreateInfo *create_info,
                                  LVKW_Cursor **out_cursor) {
  LVKW_API_VALIDATE(ctx_createCursor, ctx_handle, create_info, out_cursor);
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->cursor.create(ctx_handle, create_info, out_cursor);
}

LVKW_Status lvkw_display_destroyCursor(LVKW_Cursor *cursor) {
  LVKW_API_VALIDATE(cursor_destroy, cursor);
  LVKW_Cursor_Base *cursor_base = (LVKW_Cursor_Base *)cursor;
  return cursor_base->prv.backend->cursor.destroy(cursor);
}

LVKW_Status lvkw_data_pushText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *text) {
  LVKW_API_VALIDATE(data_pushText, window, target, text);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window;
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    LVKW_REPORT_WIND_DIAGNOSTIC(window_base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "Only CLIPBOARD target is currently implemented");
    return LVKW_ERROR;
  }
  return window_base->prv.backend->window.set_clipboard_text(window, text);
}

LVKW_Status lvkw_data_pullText(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char **out_text) {
  LVKW_API_VALIDATE(data_pullText, window, target, out_text);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window;
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window_base,
                                LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "Only CLIPBOARD target is currently implemented");
    return LVKW_ERROR;
  }
  return window_base->prv.backend->window.get_clipboard_text(window, out_text);
}

LVKW_Status lvkw_data_pushData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const LVKW_DataBuffer *data, uint32_t count) {
  LVKW_API_VALIDATE(data_pushData, window, target, data, count);
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window;
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    LVKW_REPORT_WIND_DIAGNOSTIC(window_base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "Only CLIPBOARD target is currently implemented");
    return LVKW_ERROR;
  }
  return window_base->prv.backend->window.set_clipboard_data(window,
                                                              (const LVKW_ClipboardData *)data, count);
}

LVKW_Status lvkw_data_pullData(LVKW_Window *window, LVKW_DataExchangeTarget target,
                               const char *mime_type, const void **out_data, size_t *out_size) {
  LVKW_API_VALIDATE(data_pullData, window, target, mime_type, out_data, out_size);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window;
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window_base,
                                LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "Only CLIPBOARD target is currently implemented");
    return LVKW_ERROR;
  }
  return window_base->prv.backend->window.get_clipboard_data(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_data_listBufferMimeTypes(LVKW_Window *window, LVKW_DataExchangeTarget target,
                                          const char ***out_mime_types, uint32_t *count) {
  LVKW_API_VALIDATE(data_listBufferMimeTypes, window, target, out_mime_types, count);
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window;
  if (target != LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD) {
    LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window_base,
                                LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                "Only CLIPBOARD target is currently implemented");
    return LVKW_ERROR;
  }
  return window_base->prv.backend->window.get_clipboard_mime_types(window, out_mime_types, count);
}

LVKW_Status lvkw_data_setClipboardText(LVKW_Window *window, const char *text) {
  return lvkw_data_pushText(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, text);
}

LVKW_Status lvkw_data_getClipboardText(LVKW_Window *window, const char **out_text) {
  return lvkw_data_pullText(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, out_text);
}

LVKW_Status lvkw_data_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data,
                                      uint32_t count) {
  return lvkw_data_pushData(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD,
                            (const LVKW_DataBuffer *)data, count);
}

LVKW_Status lvkw_data_getClipboardData(LVKW_Window *window, const char *mime_type,
                                      const void **out_data, size_t *out_size) {
  return lvkw_data_pullData(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD, mime_type, out_data,
                            out_size);
}

LVKW_Status lvkw_data_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types,
                                           uint32_t *count) {
  return lvkw_data_listBufferMimeTypes(window, LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD,
                                       out_mime_types, count);
}

#ifdef LVKW_ENABLE_CONTROLLER
#include "linux_internal.h"

LVKW_Status lvkw_input_createController(LVKW_ControllerRef *controller_ref,
                                        LVKW_Controller **out_controller) {
  LVKW_API_VALIDATE(ctrl_create, controller_ref, out_controller);
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller_ref;
  ctrl->prv.user_refcount++;
  *out_controller = &ctrl->pub;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_destroyController(LVKW_Controller *controller) {
  LVKW_API_VALIDATE(ctrl_destroy, controller);
  LVKW_Controller_Base *ctrl = (LVKW_Controller_Base *)controller;
  ctrl->prv.user_refcount--;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_input_getControllerInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_API_VALIDATE(ctrl_getInfo, controller, out_info);
  return lvkw_ctrl_getInfo_Linux(controller, out_info);
}

LVKW_Status lvkw_input_listControllers(LVKW_Context *ctx, LVKW_ControllerRef **out_refs,
                                       uint32_t *out_count) {
  LVKW_API_VALIDATE(ctrl_list, ctx, out_refs, out_count);
  return lvkw_ctrl_list_Linux(ctx, out_refs, out_count);
}

LVKW_Status lvkw_input_setControllerHapticLevels(LVKW_Controller *controller, uint32_t first_haptic,
                                      uint32_t count, const LVKW_Scalar *intensities) {
  LVKW_API_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);
  return lvkw_ctrl_setHapticLevels_Linux(controller, first_haptic, count, intensities);
}
#endif
