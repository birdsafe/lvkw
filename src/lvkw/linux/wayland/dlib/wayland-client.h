// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_WAYLAND_CLIENT_DLIB_H_INCLUDED
#define LVKW_WAYLAND_CLIENT_DLIB_H_INCLUDED

#include "internal.h"
#include "vendor/wayland-client-core.h"
#include "lvkw/details/lvkw_config.h"

#define LVKW_WL_FUNCTIONS_TABLE           \
  LVKW_LIB_FN(display_connect)            \
  LVKW_LIB_FN(display_disconnect)         \
  LVKW_LIB_FN(display_roundtrip)          \
  LVKW_LIB_FN(display_flush)              \
  LVKW_LIB_FN(display_prepare_read)       \
  LVKW_LIB_FN(display_get_fd)             \
  LVKW_LIB_FN(display_read_events)        \
  LVKW_LIB_FN(display_cancel_read)        \
  LVKW_LIB_FN(display_dispatch_pending)   \
  LVKW_LIB_FN(display_get_error)          \
  LVKW_LIB_FN(display_get_protocol_error) \
  LVKW_LIB_FN(proxy_get_version)          \
  LVKW_LIB_FN(proxy_marshal_flags)        \
  LVKW_LIB_FN(proxy_add_listener)         \
  LVKW_LIB_FN(proxy_destroy)              \
  LVKW_LIB_FN(proxy_set_user_data)        \
  LVKW_LIB_FN(proxy_get_user_data)        \
  // end of table

typedef struct LVKW_Lib_WaylandClient {
  LVKW_External_Lib_Base base;
#define LVKW_LIB_FN(name) __typeof__(wl_##name) *name;
  LVKW_WL_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_WaylandClient;

/* 
 * Poison landings: 
 * We use object-like macros to redirect wl_ symbols to these landings.
 * This ensures that vendor headers still parse correctly (replacing function 
 * names in declarations) while preventing the linker from adding a hard 
 * dependency on libwayland-client.
 */


#ifdef LVKW_ENABLE_INTERNAL_CHECKS
void _lvkw_wayland_symbol_poison_trap(void);
#define LVKW_WL_TRAP() _lvkw_wayland_symbol_poison_trap()

static inline void _lvkw_wl_trap_v(void) {
  LVKW_WL_TRAP();
}
static inline int _lvkw_wl_trap_i(void) {
  LVKW_WL_TRAP();
  return 0;
}
static inline uint32_t _lvkw_wl_trap_u(void) {
  LVKW_WL_TRAP();
  return 0;
}
static inline void *_lvkw_wl_trap_p(void) {
  LVKW_WL_TRAP();
  return NULL;
}

#define lvkw_wl_poison_v(...) _lvkw_wl_trap_v()
#define lvkw_wl_poison_i(...) _lvkw_wl_trap_i()
#define lvkw_wl_poison_u(...) _lvkw_wl_trap_u()
#define lvkw_wl_poison_p(...) _lvkw_wl_trap_p()

#else
#define LVKW_WL_TRAP() ((void)0)

#define lvkw_wl_poison_v(...)
#define lvkw_wl_poison_i(...) 0
#define lvkw_wl_poison_u(...) 0
#define lvkw_wl_poison_p(...) NULL
#endif

#define wl_display_connect            lvkw_wl_poison_p
#define wl_display_disconnect         lvkw_wl_poison_v
#define wl_display_roundtrip          lvkw_wl_poison_i
#define wl_display_flush              lvkw_wl_poison_i
#define wl_display_prepare_read       lvkw_wl_poison_i
#define wl_display_get_fd             lvkw_wl_poison_i
#define wl_display_read_events        lvkw_wl_poison_i
#define wl_display_cancel_read        lvkw_wl_poison_v
#define wl_display_dispatch_pending   lvkw_wl_poison_i
#define wl_display_get_error          lvkw_wl_poison_i
#define wl_display_get_protocol_error lvkw_wl_poison_u
#define wl_proxy_get_version          lvkw_wl_poison_u
#define wl_proxy_marshal_flags        lvkw_wl_poison_p
#define wl_proxy_add_listener         lvkw_wl_poison_i
#define wl_proxy_destroy              lvkw_wl_poison_v
#define wl_proxy_set_user_data        lvkw_wl_poison_v
#define wl_proxy_get_user_data        lvkw_wl_poison_p

#include "vendor/wayland-client.h"

#endif
