#include <stdlib.h>
#include <string.h>

#include "lvkw/lvkw-core.h"
#include "lvkw/lvkw.h"
#include "lvkw_mock_internal.h"

LVKW_Status _lvkw_createContext_impl(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_BackendType backend = create_info->backend;

  // In mock-only builds, only AUTO backend selection is valid
  if (backend != LVKW_BACKEND_AUTO) {
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE,
                                     "Only AUTO backend selection available in mock build");
    return LVKW_ERROR;
  }

  return lvkw_ctx_create_Mock(create_info, out_ctx_handle);
}

LVKW_Status lvkw_ctx_destroy(LVKW_Context *ctx_handle) { return lvkw_ctx_destroy_Mock(ctx_handle); }

LVKW_Status lvkw_ctx_getVkExtensions(LVKW_Context *ctx_handle, uint32_t *out_count,
                                     const char *const **out_extensions) {
  return lvkw_ctx_getVkExtensions_Mock(ctx_handle, out_count, out_extensions);
}

LVKW_Status lvkw_ctx_pollEvents(LVKW_Context *ctx_handle, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                void *userdata) {
  return lvkw_ctx_pollEvents_Mock(ctx_handle, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms, LVKW_EventType event_mask,
                                LVKW_EventCallback callback, void *userdata) {
  return lvkw_ctx_waitEvents_Mock(ctx_handle, timeout_ms, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_update(LVKW_Context *ctx_handle, uint32_t field_mask, const LVKW_ContextAttributes *attributes) {
  return lvkw_ctx_update_Mock(ctx_handle, field_mask, attributes);
}

LVKW_Status lvkw_ctx_getMonitors(LVKW_Context *ctx_handle, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  return lvkw_ctx_getMonitors_Mock(ctx_handle, out_monitors, count);
}

LVKW_Status lvkw_ctx_getMonitorModes(LVKW_Context *ctx_handle, LVKW_MonitorId monitor, LVKW_VideoMode *out_modes,
                                     uint32_t *count) {
  return lvkw_ctx_getMonitorModes_Mock(ctx_handle, monitor, out_modes, count);
}

LVKW_Status lvkw_ctx_createWindow(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                  LVKW_Window **out_window_handle) {
  return lvkw_ctx_createWindow_Mock(ctx_handle, create_info, out_window_handle);
}

LVKW_Status lvkw_wnd_destroy(LVKW_Window *window_handle) { return lvkw_wnd_destroy_Mock(window_handle); }

LVKW_Status lvkw_wnd_createVkSurface(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  return lvkw_wnd_createVkSurface_Mock(window_handle, instance, out_surface);
}

LVKW_Status lvkw_wnd_getGeometry(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  return lvkw_wnd_getGeometry_Mock(window_handle, out_geometry);
}

LVKW_Status lvkw_wnd_update(LVKW_Window *window_handle, uint32_t field_mask, const LVKW_WindowAttributes *attributes) {

  return lvkw_wnd_update_Mock(window_handle, field_mask, attributes);

}



LVKW_Status lvkw_wnd_requestFocus(LVKW_Window *window_handle) { return lvkw_wnd_requestFocus_Mock(window_handle); }



LVKW_Cursor *lvkw_ctx_getStandardCursor(LVKW_Context *ctx, LVKW_CursorShape shape) {

  return lvkw_ctx_getStandardCursor_Mock(ctx, shape);

}



LVKW_Status lvkw_ctx_createCursor(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,

                                  LVKW_Cursor **out_cursor) {

  return lvkw_ctx_createCursor_Mock(ctx, create_info, out_cursor);

}



LVKW_Status lvkw_cursor_destroy(LVKW_Cursor *cursor) { return lvkw_cursor_destroy_Mock(cursor); }

#ifdef LVKW_CONTROLLER_ENABLED
LVKW_Status lvkw_ctrl_create(LVKW_Context *ctx, LVKW_CtrlId id, LVKW_Controller **out_controller) {
  return lvkw_ctrl_create_Mock(ctx, id, out_controller);
}

LVKW_Status lvkw_ctrl_destroy(LVKW_Controller *controller) { return lvkw_ctrl_destroy_Mock(controller); }

LVKW_Status lvkw_ctrl_getInfo(LVKW_Controller *controller, LVKW_CtrlInfo *out_info) {
  return lvkw_ctrl_getInfo_Mock(controller, out_info);
}

LVKW_Status lvkw_ctrl_setMotorLevels(LVKW_Controller *controller, uint32_t first_motor, uint32_t count,
                                     const LVKW_real_t *intensities) {
  return lvkw_ctrl_setMotorLevels_Mock(controller, first_motor, count, intensities);
}
#endif
