#include <stdlib.h>
#include <string.h>

#include "lvkw/details/lvkw_api_constraints.h"
#include "lvkw/lvkw.h"
#include "lvkw_macos_internal.h"

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  if (create_info->backend != LVKW_BACKEND_AUTO && create_info->backend != LVKW_BACKEND_COCOA) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE,
                                     "Requested backend is not supported on MacOS");
    return LVKW_ERROR;
  }

  return lvkw_ctx_create_Cocoa(create_info, out_ctx_handle);
}

LVKW_Status lvkw_ctx_destroy(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  return lvkw_ctx_destroy_Cocoa(ctx_handle);
}

LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *count,
                                     const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  return lvkw_ctx_getVkExtensions_Cocoa(ctx_handle, count, out_extensions);
}

LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  LVKW_API_VALIDATE(ctx_pollEvents, ctx_handle, event_mask, callback, userdata);
  return lvkw_ctx_pollEvents_Cocoa(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_waitEvents, ctx_handle, timeout_ms, event_mask, callback, userdata);
  return lvkw_ctx_waitEvents_Cocoa(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  return lvkw_ctx_update_Cocoa(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx_handle, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_monitors, count);
  return lvkw_ctx_getMonitors_Cocoa(ctx_handle, out_monitors, count);
}

LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx_handle, LVKW_MonitorId monitor, LVKW_VideoMode *out_modes,
                                     uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  return lvkw_ctx_getMonitorModes_Cocoa(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  return lvkw_ctx_createWindow_Cocoa(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_wnd_destroy(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  return lvkw_wnd_destroy_Cocoa(window_handle);
}

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  return lvkw_wnd_createVkSurface_Cocoa(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  return lvkw_wnd_getGeometry_Cocoa(window_handle, out_geometry);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  return lvkw_wnd_update_Cocoa(window_handle, field_mask, attributes);
}

LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  return lvkw_wnd_requestFocus_Cocoa(window_handle);
}

LVKW_Status lvkw_wnd_setClipboardText(LVKW_Window *window, const char *text) {
  LVKW_API_VALIDATE(wnd_setClipboardText, window, text);
  return lvkw_wnd_setClipboardText_Cocoa(window, text);
}

LVKW_Status lvkw_wnd_getClipboardText(LVKW_Window *window, const char **out_text) {
  LVKW_API_VALIDATE(wnd_getClipboardText, window, out_text);
  return lvkw_wnd_getClipboardText_Cocoa(window, out_text);
}

LVKW_Status lvkw_wnd_setClipboardData(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  LVKW_API_VALIDATE(wnd_setClipboardData, window, data, count);
  return lvkw_wnd_setClipboardData_Cocoa(window, data, count);
}

LVKW_Status lvkw_wnd_getClipboardData(LVKW_Window *window, const char *mime_type, const void **out_data,
                                       size_t *out_size) {
  LVKW_API_VALIDATE(wnd_getClipboardData, window, mime_type, out_data, out_size);
  return lvkw_wnd_getClipboardData_Cocoa(window, mime_type, out_data, out_size);
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  LVKW_API_VALIDATE(wnd_getClipboardMimeTypes, window, out_mime_types, count);
  return lvkw_wnd_getClipboardMimeTypes_Cocoa(window, out_mime_types, count);
}

LVKW_Status lvkw_wnd_setClipboardText_Cocoa(LVKW_Window *window, const char *text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardText_Cocoa(LVKW_Window *window, const char **out_text) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_setClipboardData_Cocoa(LVKW_Window *window, const LVKW_ClipboardData *data, uint32_t count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardData_Cocoa(LVKW_Window *window, const char *mime_type, const void **out_data,
                                            size_t *out_size) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

LVKW_Status lvkw_wnd_getClipboardMimeTypes_Cocoa(LVKW_Window *window, const char ***out_mime_types, uint32_t *count) {
  LVKW_REPORT_WIND_DIAGNOSTIC((LVKW_Window_Base *)window, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                              "Clipboard not implemented yet on MacOS");
  return LVKW_ERROR;
}

#ifdef LVKW_CONTROLLER_ENABLED
LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  LVKW_API_VALIDATE(ctrl_create, ctx, id, out_controller);
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctrl_destroy(LVKW_Controller *ctrl) {
  LVKW_API_VALIDATE(ctrl_destroy, ctrl);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  LVKW_API_VALIDATE(ctrl_getInfo, controller, out_info);
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctrl_setHapticLevels(LVKW_Controller *controller, uint32_t first_haptic, uint32_t count,
                                      const LVKW_real_t *intensities) {
  LVKW_API_VALIDATE(ctrl_setHapticLevels, controller, first_haptic, count, intensities);
  return LVKW_ERROR;
}
#endif
