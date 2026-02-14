/* Generated from xdg-decoration-unstable-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_XDG_DECORATION_UNSTABLE_V1_H
#define LVKW_WAYLAND_HELPERS_XDG_DECORATION_UNSTABLE_V1_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct zxdg_decoration_manager_v1;
struct zxdg_decoration_manager_v1_listener;
struct zxdg_toplevel_decoration_v1;
struct xdg_toplevel;
/* interface zxdg_decoration_manager_v1 */
static inline void
lvkw_zxdg_decoration_manager_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) zxdg_decoration_manager_v1, user_data);
}

static inline void *
lvkw_zxdg_decoration_manager_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) zxdg_decoration_manager_v1);
}

static inline uint32_t
lvkw_zxdg_decoration_manager_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) zxdg_decoration_manager_v1);
}

static inline int
lvkw_zxdg_decoration_manager_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1, const struct zxdg_decoration_manager_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) zxdg_decoration_manager_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zxdg_decoration_manager_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) zxdg_decoration_manager_v1, ZXDG_DECORATION_MANAGER_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) zxdg_decoration_manager_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct zxdg_toplevel_decoration_v1 *
lvkw_zxdg_decoration_manager_v1_get_toplevel_decoration(const LVKW_Lib_WaylandClient *lib, struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1, struct xdg_toplevel *toplevel)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) zxdg_decoration_manager_v1, ZXDG_DECORATION_MANAGER_V1_GET_TOPLEVEL_DECORATION, &zxdg_toplevel_decoration_v1_interface, lib->proxy_get_version((struct wl_proxy *) zxdg_decoration_manager_v1), 0, NULL, toplevel);
	return (struct zxdg_toplevel_decoration_v1 *) id;
}

struct zxdg_toplevel_decoration_v1;
struct zxdg_toplevel_decoration_v1_listener;
/* interface zxdg_toplevel_decoration_v1 */
static inline void
lvkw_zxdg_toplevel_decoration_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) zxdg_toplevel_decoration_v1, user_data);
}

static inline void *
lvkw_zxdg_toplevel_decoration_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) zxdg_toplevel_decoration_v1);
}

static inline uint32_t
lvkw_zxdg_toplevel_decoration_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) zxdg_toplevel_decoration_v1);
}

static inline int
lvkw_zxdg_toplevel_decoration_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, const struct zxdg_toplevel_decoration_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) zxdg_toplevel_decoration_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zxdg_toplevel_decoration_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) zxdg_toplevel_decoration_v1, ZXDG_TOPLEVEL_DECORATION_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) zxdg_toplevel_decoration_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_zxdg_toplevel_decoration_v1_set_mode(const LVKW_Lib_WaylandClient *lib, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t mode)
{
	lib->proxy_marshal_flags((struct wl_proxy *) zxdg_toplevel_decoration_v1, ZXDG_TOPLEVEL_DECORATION_V1_SET_MODE, NULL, lib->proxy_get_version((struct wl_proxy *) zxdg_toplevel_decoration_v1), 0, mode);
}

static inline void
lvkw_zxdg_toplevel_decoration_v1_unset_mode(const LVKW_Lib_WaylandClient *lib, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) zxdg_toplevel_decoration_v1, ZXDG_TOPLEVEL_DECORATION_V1_UNSET_MODE, NULL, lib->proxy_get_version((struct wl_proxy *) zxdg_toplevel_decoration_v1), 0);
}

#endif
