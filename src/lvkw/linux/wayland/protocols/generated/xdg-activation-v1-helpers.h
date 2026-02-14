/* Generated from xdg-activation-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_XDG_ACTIVATION_V1_H
#define LVKW_WAYLAND_HELPERS_XDG_ACTIVATION_V1_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct xdg_activation_v1;
struct xdg_activation_v1_listener;
struct xdg_activation_token_v1;
struct wl_surface;
/* interface xdg_activation_v1 */
static inline void
lvkw_xdg_activation_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_v1 *xdg_activation_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) xdg_activation_v1, user_data);
}

static inline void *
lvkw_xdg_activation_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_v1 *xdg_activation_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) xdg_activation_v1);
}

static inline uint32_t
lvkw_xdg_activation_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_v1 *xdg_activation_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) xdg_activation_v1);
}

static inline int
lvkw_xdg_activation_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_v1 *xdg_activation_v1, const struct xdg_activation_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) xdg_activation_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_xdg_activation_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_v1 *xdg_activation_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_v1, XDG_ACTIVATION_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_activation_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct xdg_activation_token_v1 *
lvkw_xdg_activation_v1_get_activation_token(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_v1 *xdg_activation_v1)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_v1, XDG_ACTIVATION_V1_GET_ACTIVATION_TOKEN, &xdg_activation_token_v1_interface, lib->proxy_get_version((struct wl_proxy *) xdg_activation_v1), 0, NULL);
	return (struct xdg_activation_token_v1 *) id;
}

static inline void
lvkw_xdg_activation_v1_activate(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_v1 *xdg_activation_v1, const char *token, struct wl_surface *surface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_v1, XDG_ACTIVATION_V1_ACTIVATE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_activation_v1), 0, token, surface);
}

struct xdg_activation_token_v1;
struct xdg_activation_token_v1_listener;
struct wl_seat;
struct wl_surface;
/* interface xdg_activation_token_v1 */
static inline void
lvkw_xdg_activation_token_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) xdg_activation_token_v1, user_data);
}

static inline void *
lvkw_xdg_activation_token_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) xdg_activation_token_v1);
}

static inline uint32_t
lvkw_xdg_activation_token_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) xdg_activation_token_v1);
}

static inline int
lvkw_xdg_activation_token_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1, const struct xdg_activation_token_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) xdg_activation_token_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_xdg_activation_token_v1_set_serial(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1, uint32_t serial, struct wl_seat *seat)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_token_v1, XDG_ACTIVATION_TOKEN_V1_SET_SERIAL, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_activation_token_v1), 0, serial, seat);
}

static inline void
lvkw_xdg_activation_token_v1_set_app_id(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1, const char *app_id)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_token_v1, XDG_ACTIVATION_TOKEN_V1_SET_APP_ID, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_activation_token_v1), 0, app_id);
}

static inline void
lvkw_xdg_activation_token_v1_set_surface(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1, struct wl_surface *surface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_token_v1, XDG_ACTIVATION_TOKEN_V1_SET_SURFACE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_activation_token_v1), 0, surface);
}

static inline void
lvkw_xdg_activation_token_v1_commit(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_token_v1, XDG_ACTIVATION_TOKEN_V1_COMMIT, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_activation_token_v1), 0);
}

static inline void
lvkw_xdg_activation_token_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct xdg_activation_token_v1 *xdg_activation_token_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_activation_token_v1, XDG_ACTIVATION_TOKEN_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_activation_token_v1), WL_MARSHAL_FLAG_DESTROY);
}

#endif
