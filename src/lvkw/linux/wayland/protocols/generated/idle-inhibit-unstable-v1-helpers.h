/* Generated from idle-inhibit-unstable-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_IDLE_INHIBIT_UNSTABLE_V1_H
#define LVKW_WAYLAND_HELPERS_IDLE_INHIBIT_UNSTABLE_V1_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct zwp_idle_inhibit_manager_v1;
struct zwp_idle_inhibit_manager_v1_listener;
struct zwp_idle_inhibitor_v1;
struct wl_surface;
/* interface zwp_idle_inhibit_manager_v1 */
static inline void
lvkw_zwp_idle_inhibit_manager_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) zwp_idle_inhibit_manager_v1, user_data);
}

static inline void *
lvkw_zwp_idle_inhibit_manager_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) zwp_idle_inhibit_manager_v1);
}

static inline uint32_t
lvkw_zwp_idle_inhibit_manager_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) zwp_idle_inhibit_manager_v1);
}

static inline int
lvkw_zwp_idle_inhibit_manager_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1, const struct zwp_idle_inhibit_manager_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) zwp_idle_inhibit_manager_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_idle_inhibit_manager_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) zwp_idle_inhibit_manager_v1, ZWP_IDLE_INHIBIT_MANAGER_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) zwp_idle_inhibit_manager_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct zwp_idle_inhibitor_v1 *
lvkw_zwp_idle_inhibit_manager_v1_create_inhibitor(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) zwp_idle_inhibit_manager_v1, ZWP_IDLE_INHIBIT_MANAGER_V1_CREATE_INHIBITOR, &zwp_idle_inhibitor_v1_interface, lib->proxy_get_version((struct wl_proxy *) zwp_idle_inhibit_manager_v1), 0, NULL, surface);
	return (struct zwp_idle_inhibitor_v1 *) id;
}

struct zwp_idle_inhibitor_v1;
struct zwp_idle_inhibitor_v1_listener;
/* interface zwp_idle_inhibitor_v1 */
static inline void
lvkw_zwp_idle_inhibitor_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) zwp_idle_inhibitor_v1, user_data);
}

static inline void *
lvkw_zwp_idle_inhibitor_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) zwp_idle_inhibitor_v1);
}

static inline uint32_t
lvkw_zwp_idle_inhibitor_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) zwp_idle_inhibitor_v1);
}

static inline int
lvkw_zwp_idle_inhibitor_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1, const struct zwp_idle_inhibitor_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) zwp_idle_inhibitor_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_idle_inhibitor_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) zwp_idle_inhibitor_v1, ZWP_IDLE_INHIBITOR_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) zwp_idle_inhibitor_v1), WL_MARSHAL_FLAG_DESTROY);
}

#endif
