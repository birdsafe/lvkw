#ifndef LVKW_WAYLAND_CLIENT_DLIB_H_INCLUDED
#define LVKW_WAYLAND_CLIENT_DLIB_H_INCLUDED

#include "lvkw_internal.h"
#include "vendor/wayland-client-core.h"

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
#define LVKW_LIB_FN(name) typeof(wl_##name) *name;
  LVKW_WL_FUNCTIONS_TABLE
#undef LVKW_LIB_FN
} LVKW_Lib_WaylandClient;

extern LVKW_Lib_WaylandClient lvkw_lib_wl;

#define wl_display_connect(...) lvkw_lib_wl.display_connect(__VA_ARGS__)
#define wl_display_disconnect(...) lvkw_lib_wl.display_disconnect(__VA_ARGS__)
#define wl_display_roundtrip(...) lvkw_lib_wl.display_roundtrip(__VA_ARGS__)
#define wl_display_flush(...) lvkw_lib_wl.display_flush(__VA_ARGS__)
#define wl_display_prepare_read(...) lvkw_lib_wl.display_prepare_read(__VA_ARGS__)
#define wl_display_get_fd(...) lvkw_lib_wl.display_get_fd(__VA_ARGS__)
#define wl_display_read_events(...) lvkw_lib_wl.display_read_events(__VA_ARGS__)
#define wl_display_cancel_read(...) lvkw_lib_wl.display_cancel_read(__VA_ARGS__)
#define wl_display_dispatch_pending(...) lvkw_lib_wl.display_dispatch_pending(__VA_ARGS__)
#define wl_display_get_error(...) lvkw_lib_wl.display_get_error(__VA_ARGS__)
#define wl_display_get_protocol_error(...) lvkw_lib_wl.display_get_protocol_error(__VA_ARGS__)
#define wl_proxy_get_version(...) lvkw_lib_wl.proxy_get_version(__VA_ARGS__)
#define wl_proxy_marshal_flags(...) lvkw_lib_wl.proxy_marshal_flags(__VA_ARGS__)
#define wl_proxy_add_listener(...) lvkw_lib_wl.proxy_add_listener(__VA_ARGS__)
#define wl_proxy_destroy(...) lvkw_lib_wl.proxy_destroy(__VA_ARGS__)
#define wl_proxy_set_user_data(...) lvkw_lib_wl.proxy_set_user_data(__VA_ARGS__)
#define wl_proxy_get_user_data(...) lvkw_lib_wl.proxy_get_user_data(__VA_ARGS__)

#include "vendor/wayland-client.h"

#endif
