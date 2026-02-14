/* Generated from fractional-scale-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_FRACTIONAL_SCALE_V1_H
#define LVKW_WAYLAND_HELPERS_FRACTIONAL_SCALE_V1_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct wp_fractional_scale_manager_v1;
struct wp_fractional_scale_manager_v1_listener;
struct wp_fractional_scale_v1;
struct wl_surface;
/* interface wp_fractional_scale_manager_v1 */
static inline void
lvkw_wp_fractional_scale_manager_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wp_fractional_scale_manager_v1, user_data);
}

static inline void *
lvkw_wp_fractional_scale_manager_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wp_fractional_scale_manager_v1);
}

static inline uint32_t
lvkw_wp_fractional_scale_manager_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) wp_fractional_scale_manager_v1);
}

static inline int
lvkw_wp_fractional_scale_manager_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1, const struct wp_fractional_scale_manager_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wp_fractional_scale_manager_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_fractional_scale_manager_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wp_fractional_scale_manager_v1, WP_FRACTIONAL_SCALE_MANAGER_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wp_fractional_scale_manager_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct wp_fractional_scale_v1 *
lvkw_wp_fractional_scale_manager_v1_get_fractional_scale(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wp_fractional_scale_manager_v1, WP_FRACTIONAL_SCALE_MANAGER_V1_GET_FRACTIONAL_SCALE, &wp_fractional_scale_v1_interface, lib->proxy_get_version((struct wl_proxy *) wp_fractional_scale_manager_v1), 0, NULL, surface);
	return (struct wp_fractional_scale_v1 *) id;
}

struct wp_fractional_scale_v1;
struct wp_fractional_scale_v1_listener;
/* interface wp_fractional_scale_v1 */
static inline void
lvkw_wp_fractional_scale_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_v1 *wp_fractional_scale_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wp_fractional_scale_v1, user_data);
}

static inline void *
lvkw_wp_fractional_scale_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_v1 *wp_fractional_scale_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wp_fractional_scale_v1);
}

static inline uint32_t
lvkw_wp_fractional_scale_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_v1 *wp_fractional_scale_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) wp_fractional_scale_v1);
}

static inline int
lvkw_wp_fractional_scale_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_v1 *wp_fractional_scale_v1, const struct wp_fractional_scale_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wp_fractional_scale_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_fractional_scale_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct wp_fractional_scale_v1 *wp_fractional_scale_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wp_fractional_scale_v1, WP_FRACTIONAL_SCALE_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wp_fractional_scale_v1), WL_MARSHAL_FLAG_DESTROY);
}

#endif
