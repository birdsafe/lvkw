#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw.h"
#include "lvkw_win32_internal.h"
#include "lvkw/details/lvkw_api_constraints.h"

#define LVKW_VALIDATE(func, ...) do { \
    LVKW_Status _s = _lvkw_api_constraints_##func(__VA_ARGS__); \
    if (_s != LVKW_SUCCESS) return _s; \
} while(0)

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  if (create_info->backend != LVKW_BACKEND_AUTO && create_info->backend != LVKW_BACKEND_WIN32) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE, "Requested backend is not supported on Windows");
    return LVKW_ERROR;
  }

  return lvkw_ctx_create_Win32(create_info, out_ctx_handle);
}

LVKW_Status lvkw_ctx_destroy(LVKW_Context *ctx_handle) {
  LVKW_VALIDATE(ctx_destroy, ctx_handle);
  lvkw_ctx_destroy_Win32(ctx_handle);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count, const char *const **out_extensions) {
  LVKW_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  *out_extensions = lvkw_ctx_getVkExtensions_Win32(ctx_handle, count);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  LVKW_VALIDATE(ctx_pollEvents, ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_pollEvents_Win32(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  LVKW_VALIDATE(ctx_waitEvents, ctx_handle, timeout_ms, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_Win32(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  LVKW_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  return lvkw_ctx_update_Win32(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx_handle, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  LVKW_VALIDATE(ctx_getMonitors, ctx_handle, out_monitors, count);
  return lvkw_ctx_getMonitors_Win32(ctx_handle, out_monitors, count);
}

LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx_handle, LVKW_MonitorId monitor,
                                     LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_Win32(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  LVKW_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_Win32(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_wnd_destroy(LVKW_Window *window_handle) {
  LVKW_VALIDATE(wnd_destroy, window_handle);
  lvkw_wnd_destroy_Win32(window_handle);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_Win32(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  return lvkw_wnd_getGeometry_Win32(window_handle, out_geometry);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask,
                            const LVKW_WindowAttributes *attributes) {
  LVKW_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  return lvkw_wnd_update_Win32(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  LVKW_VALIDATE(wnd_requestFocus, window_handle);
  return lvkw_wnd_requestFocus_Win32(window_handle);
}

LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_VALIDATE(wnd_setClipboardText, window, text);
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base*)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED, "Clipboard not implemented yet on Win32");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_VALIDATE(wnd_getClipboardText, window, out_text);
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base*)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED, "Clipboard not implemented yet on Win32");
  return LVKW_ERROR;
}

#ifdef LVKW_CONTROLLER_ENABLED
LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  LVKW_VALIDATE(ctrl_create, ctx, id, out_controller);
  return LVKW_ERROR; // Not implemented
}

LVKW_Status lvkw_ctrl_destroy(LVKW_Controller *ctrl) {
  LVKW_VALIDATE(ctrl_destroy, ctrl);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_VALIDATE(ctrl_getInfo, controller, out_info);
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctrl_setMotorLevels(LVKW_Controller *controller, uint32_t first_motor, uint32_t count,
                                     const LVKW_real_t *intensities) {
  LVKW_VALIDATE(ctrl_setMotorLevels, controller, first_motor, count, intensities);
  return LVKW_ERROR;
}
#endif
