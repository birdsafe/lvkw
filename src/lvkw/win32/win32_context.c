// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "api_constraints.h"
#include "win32_internal.h"

LVKW_Status lvkw_ctx_create_Win32(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;
  LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_BACKEND_UNAVAILABLE, "Win32 backend is currently stubbed");
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_destroy_Win32(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  (void)ctx_handle;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getVkExtensions_Win32(LVKW_Context *ctx_handle, uint32_t *count,
                                           const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  (void)ctx_handle;
  if (count) *count = 0;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_update_Win32(LVKW_Context *ctx_handle, uint32_t field_mask,
                                  const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  (void)ctx_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_getMonitors_Win32(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_refs, count);
  (void)ctx_handle;
  (void)out_refs;
  (void)count;
  return LVKW_ERROR;
}

LVKW_Status lvkw_ctx_getMonitorModes_Win32(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                           LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  (void)ctx_handle;
  (void)monitor;
  (void)out_modes;
  (void)count;
  return LVKW_ERROR;
}
