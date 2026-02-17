// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <windows.h>
#include "win32_internal.h"
#include "internal.h"

uint64_t _lvkw_get_timestamp_ms(void) {
  return GetTickCount64();
}

void lvkw_ctx_assertThread_Win32(LVKW_Context *ctx_handle) {
  (void)ctx_handle;
}
