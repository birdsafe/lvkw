/* Generated from viewporter.xml */

#ifndef LVKW_WAYLAND_HELPERS_VIEWPORTER_H
#define LVKW_WAYLAND_HELPERS_VIEWPORTER_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct wp_viewporter;
struct wp_viewporter_listener;
struct wp_viewport;
struct wl_surface;
/* interface wp_viewporter */
static inline void
lvkw_wp_viewporter_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_viewporter *wp_viewporter, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wp_viewporter, user_data);
}

static inline void *
lvkw_wp_viewporter_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_viewporter *wp_viewporter)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wp_viewporter);
}

static inline uint32_t
lvkw_wp_viewporter_get_version(const LVKW_Lib_WaylandClient *lib, struct wp_viewporter *wp_viewporter)
{
	return lib->proxy_get_version((struct wl_proxy *) wp_viewporter);
}

static inline int
lvkw_wp_viewporter_add_listener(const LVKW_Lib_WaylandClient *lib, struct wp_viewporter *wp_viewporter, const struct wp_viewporter_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wp_viewporter, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_viewporter_destroy(const LVKW_Lib_WaylandClient *lib, struct wp_viewporter *wp_viewporter)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wp_viewporter, WP_VIEWPORTER_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wp_viewporter), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct wp_viewport *
lvkw_wp_viewporter_get_viewport(const LVKW_Lib_WaylandClient *lib, struct wp_viewporter *wp_viewporter, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wp_viewporter, WP_VIEWPORTER_GET_VIEWPORT, &wp_viewport_interface, lib->proxy_get_version((struct wl_proxy *) wp_viewporter), 0, NULL, surface);
	return (struct wp_viewport *) id;
}

struct wp_viewport;
struct wp_viewport_listener;
/* interface wp_viewport */
static inline void
lvkw_wp_viewport_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_viewport *wp_viewport, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wp_viewport, user_data);
}

static inline void *
lvkw_wp_viewport_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wp_viewport *wp_viewport)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wp_viewport);
}

static inline uint32_t
lvkw_wp_viewport_get_version(const LVKW_Lib_WaylandClient *lib, struct wp_viewport *wp_viewport)
{
	return lib->proxy_get_version((struct wl_proxy *) wp_viewport);
}

static inline int
lvkw_wp_viewport_add_listener(const LVKW_Lib_WaylandClient *lib, struct wp_viewport *wp_viewport, const struct wp_viewport_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wp_viewport, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_viewport_destroy(const LVKW_Lib_WaylandClient *lib, struct wp_viewport *wp_viewport)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wp_viewport, WP_VIEWPORT_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wp_viewport), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wp_viewport_set_source(const LVKW_Lib_WaylandClient *lib, struct wp_viewport *wp_viewport, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wp_viewport, WP_VIEWPORT_SET_SOURCE, NULL, lib->proxy_get_version((struct wl_proxy *) wp_viewport), 0, x, y, width, height);
}

static inline void
lvkw_wp_viewport_set_destination(const LVKW_Lib_WaylandClient *lib, struct wp_viewport *wp_viewport, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wp_viewport, WP_VIEWPORT_SET_DESTINATION, NULL, lib->proxy_get_version((struct wl_proxy *) wp_viewport), 0, width, height);
}

#endif
