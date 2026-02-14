// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_DETAILS_ABI_CHECKS_H_INCLUDED
#define LVKW_DETAILS_ABI_CHECKS_H_INCLUDED

/*
 * This file performs compile-time checks to ensure that the LVKW library ABI
 * matches the expectations of the configuration (Float vs Double).
 * These checks require C11 (_Static_assert) or C++11 (static_assert).
 */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define LVKW_STATIC_ASSERT _Static_assert
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define LVKW_STATIC_ASSERT static_assert
#else
#define LVKW_STATIC_ASSERT(cond, msg)
#endif

#if defined(LVKW_STATIC_ASSERT)

/* Ensure LVKW_Event fits within the guaranteed cache-line friendly size limits.
 * 32 bytes for Float config.
 * 48 bytes for Double config.
 */
#define LVKW_EXPECTED_EVENT_SIZE ((sizeof(LVKW_real_t) == 4) ? 32 : 48)

LVKW_STATIC_ASSERT(sizeof(LVKW_Event) <= LVKW_EXPECTED_EVENT_SIZE,
                   "LVKW_Event ABI Violation: Size exceeds guaranteed threshold. "
                   "Check structure packing or LVKW_USE_FLOAT configuration.");

/* Verify individual critical event sizes */
LVKW_STATIC_ASSERT(sizeof(LVKW_DndHoverEvent) <= LVKW_EXPECTED_EVENT_SIZE,
                   "LVKW_DndHoverEvent exceeds size limit.");

LVKW_STATIC_ASSERT(sizeof(LVKW_MouseMotionEvent) <= LVKW_EXPECTED_EVENT_SIZE,
                   "LVKW_MouseMotionEvent exceeds size limit.");

#endif

#endif /* LVKW_DETAILS_ABI_CHECKS_H_INCLUDED */
