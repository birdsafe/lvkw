#include "lvkw_api_checks.h"
#include "lvkw_win32_internal.h"

LVKW_Status lvkw_ctx_create_Win32(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;
  LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE, "Win32 backend is currently stubbed");
  return LVKW_ERROR;
}

void lvkw_ctx_destroy_Win32(LVKW_Context *ctx_handle) {
  (void)ctx_handle;
}

const char *const *lvkw_ctx_getVkExtensions_Win32(LVKW_Context *ctx_handle, uint32_t *count) {
  (void)ctx_handle;
  if (count) *count = 0;
  return NULL;
}

LVKW_Status lvkw_ctx_update_Win32(LVKW_Context *ctx_handle, uint32_t field_mask,
                                  const LVKW_ContextAttributes *attributes) {
  (void)ctx_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_getMonitors_Win32(LVKW_Context *ctx_handle, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  (void)ctx_handle;
  (void)out_monitors;
  (void)count;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_getMonitorModes_Win32(LVKW_Context *ctx_handle, LVKW_MonitorId monitor,
                                           LVKW_VideoMode *out_modes, uint32_t *count) {
  (void)ctx_handle;
  (void)monitor;
  (void)out_modes;
  (void)count;
  return LVKW_ERROR;
}
