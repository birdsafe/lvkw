#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "lvkw_internal.h"

#define LVKW_VALIDATE(func, ...)                                \
  do {                                                          \
    LVKW_Status _s = _lvkw_api_constraints_##func(__VA_ARGS__); \
    if (_s != LVKW_SUCCESS) return _s;                          \
  } while (0)

// Forward declarations for backend-specific creation functions
LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
LVKW_Status lvkw_ctx_create_X11(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
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
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.destroy(ctx_handle);
}

LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count, const char *const **out_extensions) {
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_vulkan_instance_extensions(ctx_handle, count, out_extensions);
}

LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.poll_events(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.wait_events(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.update(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx_handle, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_monitors(ctx_handle, out_monitors, count);
}

LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx_handle, LVKW_MonitorId monitor, LVKW_VideoMode *out_modes,
                                     uint32_t *count) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->context.get_monitor_modes(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  *out_window_handle = NULL;
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->window.create(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_wnd_destroy(LVKW_Window *window_handle) {
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.destroy(window_handle);
}

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  *out_surface = NULL;
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.create_vk_surface(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  const LVKW_Window_Base *window_base = (const LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.get_geometry(window_handle, out_geometry);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.update(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  LVKW_Window_Base *window_base = (LVKW_Window_Base *)window_handle;
  return window_base->prv.backend->window.request_focus(window_handle);
}

LVKW_Cursor *lvkw_ctx_getStandardCursor(LVKW_Context *ctx_handle, LVKW_CursorShape shape) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->cursor.get_standard(ctx_handle, shape);
}

LVKW_Status lvkw_ctx_createCursor(LVKW_Context *ctx_handle, const LVKW_CursorCreateInfo *create_info,
                                  LVKW_Cursor **out_cursor) {
  LVKW_Context_Base *ctx_base = (LVKW_Context_Base *)ctx_handle;
  return ctx_base->prv.backend->cursor.create(ctx_handle, create_info, out_cursor);
}

LVKW_Status lvkw_cursor_destroy(LVKW_Cursor *cursor) {
  if (!cursor) return LVKW_SUCCESS;
  LVKW_Cursor_Base *cursor_base = (LVKW_Cursor_Base *)cursor;
  return cursor_base->prv.backend->cursor.destroy(cursor);
}

LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Linux");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on Linux");
  return LVKW_ERROR;
}

#ifdef LVKW_CONTROLLER_ENABLED
#include "lvkw_linux_internal.h"

LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  return lvkw_ctrl_create_Linux(ctx, id, out_controller);
}

LVKW_Status lvkw_ctrl_destroy(LVKW_Controller *ctrl) { return lvkw_ctrl_destroy_Linux(ctrl); }

LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  return lvkw_ctrl_getInfo_Linux(controller, out_info);
}
#endif
