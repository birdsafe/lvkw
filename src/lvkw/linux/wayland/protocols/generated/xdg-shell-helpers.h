/* Generated from xdg-shell.xml */

#ifndef LVKW_WAYLAND_HELPERS_XDG_SHELL_H
#define LVKW_WAYLAND_HELPERS_XDG_SHELL_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct xdg_wm_base;
struct xdg_wm_base_listener;
struct xdg_positioner;
struct xdg_surface;
struct wl_surface;
/* interface xdg_wm_base */
static inline void
lvkw_xdg_wm_base_set_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) xdg_wm_base, user_data);
}

static inline void *
lvkw_xdg_wm_base_get_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base)
{
	return lib->proxy_get_user_data((struct wl_proxy *) xdg_wm_base);
}

static inline uint32_t
lvkw_xdg_wm_base_get_version(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base)
{
	return lib->proxy_get_version((struct wl_proxy *) xdg_wm_base);
}

static inline int
lvkw_xdg_wm_base_add_listener(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base, const struct xdg_wm_base_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) xdg_wm_base, (void (**)(void)) listener, data);
}

static inline void
lvkw_xdg_wm_base_destroy(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_wm_base, XDG_WM_BASE_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_wm_base), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct xdg_positioner *
lvkw_xdg_wm_base_create_positioner(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) xdg_wm_base, XDG_WM_BASE_CREATE_POSITIONER, &xdg_positioner_interface, lib->proxy_get_version((struct wl_proxy *) xdg_wm_base), 0, NULL);
	return (struct xdg_positioner *) id;
}

static inline struct xdg_surface *
lvkw_xdg_wm_base_get_xdg_surface(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) xdg_wm_base, XDG_WM_BASE_GET_XDG_SURFACE, &xdg_surface_interface, lib->proxy_get_version((struct wl_proxy *) xdg_wm_base), 0, NULL, surface);
	return (struct xdg_surface *) id;
}

static inline void
lvkw_xdg_wm_base_pong(const LVKW_Lib_WaylandClient *lib, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_wm_base, XDG_WM_BASE_PONG, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_wm_base), 0, serial);
}

struct xdg_positioner;
struct xdg_positioner_listener;
/* interface xdg_positioner */
static inline void
lvkw_xdg_positioner_set_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) xdg_positioner, user_data);
}

static inline void *
lvkw_xdg_positioner_get_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner)
{
	return lib->proxy_get_user_data((struct wl_proxy *) xdg_positioner);
}

static inline uint32_t
lvkw_xdg_positioner_get_version(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner)
{
	return lib->proxy_get_version((struct wl_proxy *) xdg_positioner);
}

static inline int
lvkw_xdg_positioner_add_listener(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, const struct xdg_positioner_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) xdg_positioner, (void (**)(void)) listener, data);
}

static inline void
lvkw_xdg_positioner_destroy(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_xdg_positioner_set_size(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_SIZE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, width, height);
}

static inline void
lvkw_xdg_positioner_set_anchor_rect(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, int32_t x, int32_t y, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_ANCHOR_RECT, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, x, y, width, height);
}

static inline void
lvkw_xdg_positioner_set_anchor(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, uint32_t anchor)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_ANCHOR, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, anchor);
}

static inline void
lvkw_xdg_positioner_set_gravity(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, uint32_t gravity)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_GRAVITY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, gravity);
}

static inline void
lvkw_xdg_positioner_set_constraint_adjustment(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, uint32_t constraint_adjustment)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, constraint_adjustment);
}

static inline void
lvkw_xdg_positioner_set_offset(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, int32_t x, int32_t y)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_OFFSET, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, x, y);
}

static inline void
lvkw_xdg_positioner_set_reactive(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_REACTIVE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0);
}

static inline void
lvkw_xdg_positioner_set_parent_size(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, int32_t parent_width, int32_t parent_height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_PARENT_SIZE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, parent_width, parent_height);
}

static inline void
lvkw_xdg_positioner_set_parent_configure(const LVKW_Lib_WaylandClient *lib, struct xdg_positioner *xdg_positioner, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_positioner, XDG_POSITIONER_SET_PARENT_CONFIGURE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_positioner), 0, serial);
}

struct xdg_surface;
struct xdg_surface_listener;
struct xdg_toplevel;
struct xdg_popup;
struct xdg_surface;
struct xdg_positioner;
/* interface xdg_surface */
static inline void
lvkw_xdg_surface_set_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) xdg_surface, user_data);
}

static inline void *
lvkw_xdg_surface_get_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface)
{
	return lib->proxy_get_user_data((struct wl_proxy *) xdg_surface);
}

static inline uint32_t
lvkw_xdg_surface_get_version(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface)
{
	return lib->proxy_get_version((struct wl_proxy *) xdg_surface);
}

static inline int
lvkw_xdg_surface_add_listener(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface, const struct xdg_surface_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) xdg_surface, (void (**)(void)) listener, data);
}

static inline void
lvkw_xdg_surface_destroy(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_surface, XDG_SURFACE_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_surface), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct xdg_toplevel *
lvkw_xdg_surface_get_toplevel(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) xdg_surface, XDG_SURFACE_GET_TOPLEVEL, &xdg_toplevel_interface, lib->proxy_get_version((struct wl_proxy *) xdg_surface), 0, NULL);
	return (struct xdg_toplevel *) id;
}

static inline struct xdg_popup *
lvkw_xdg_surface_get_popup(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface, struct xdg_surface *parent, struct xdg_positioner *positioner)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) xdg_surface, XDG_SURFACE_GET_POPUP, &xdg_popup_interface, lib->proxy_get_version((struct wl_proxy *) xdg_surface), 0, NULL, parent, positioner);
	return (struct xdg_popup *) id;
}

static inline void
lvkw_xdg_surface_set_window_geometry(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_surface, XDG_SURFACE_SET_WINDOW_GEOMETRY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_surface), 0, x, y, width, height);
}

static inline void
lvkw_xdg_surface_ack_configure(const LVKW_Lib_WaylandClient *lib, struct xdg_surface *xdg_surface, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_surface, XDG_SURFACE_ACK_CONFIGURE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_surface), 0, serial);
}

struct xdg_toplevel;
struct xdg_toplevel_listener;
struct xdg_toplevel;
struct wl_seat;
struct wl_seat;
struct wl_seat;
struct wl_output;
/* interface xdg_toplevel */
static inline void
lvkw_xdg_toplevel_set_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) xdg_toplevel, user_data);
}

static inline void *
lvkw_xdg_toplevel_get_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel)
{
	return lib->proxy_get_user_data((struct wl_proxy *) xdg_toplevel);
}

static inline uint32_t
lvkw_xdg_toplevel_get_version(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel)
{
	return lib->proxy_get_version((struct wl_proxy *) xdg_toplevel);
}

static inline int
lvkw_xdg_toplevel_add_listener(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, const struct xdg_toplevel_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) xdg_toplevel, (void (**)(void)) listener, data);
}

static inline void
lvkw_xdg_toplevel_destroy(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_xdg_toplevel_set_parent(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, struct xdg_toplevel *parent)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_PARENT, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, parent);
}

static inline void
lvkw_xdg_toplevel_set_title(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, const char *title)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_TITLE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, title);
}

static inline void
lvkw_xdg_toplevel_set_app_id(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, const char *app_id)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_APP_ID, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, app_id);
}

static inline void
lvkw_xdg_toplevel_show_window_menu(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SHOW_WINDOW_MENU, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, seat, serial, x, y);
}

static inline void
lvkw_xdg_toplevel_move(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, struct wl_seat *seat, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_MOVE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, seat, serial);
}

static inline void
lvkw_xdg_toplevel_resize(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, struct wl_seat *seat, uint32_t serial, uint32_t edges)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_RESIZE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, seat, serial, edges);
}

static inline void
lvkw_xdg_toplevel_set_max_size(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_MAX_SIZE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, width, height);
}

static inline void
lvkw_xdg_toplevel_set_min_size(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_MIN_SIZE, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, width, height);
}

static inline void
lvkw_xdg_toplevel_set_maximized(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_MAXIMIZED, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0);
}

static inline void
lvkw_xdg_toplevel_unset_maximized(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_UNSET_MAXIMIZED, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0);
}

static inline void
lvkw_xdg_toplevel_set_fullscreen(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel, struct wl_output *output)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_FULLSCREEN, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0, output);
}

static inline void
lvkw_xdg_toplevel_unset_fullscreen(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_UNSET_FULLSCREEN, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0);
}

static inline void
lvkw_xdg_toplevel_set_minimized(const LVKW_Lib_WaylandClient *lib, struct xdg_toplevel *xdg_toplevel)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_toplevel, XDG_TOPLEVEL_SET_MINIMIZED, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_toplevel), 0);
}

struct xdg_popup;
struct xdg_popup_listener;
struct wl_seat;
struct xdg_positioner;
/* interface xdg_popup */
static inline void
lvkw_xdg_popup_set_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_popup *xdg_popup, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) xdg_popup, user_data);
}

static inline void *
lvkw_xdg_popup_get_user_data(const LVKW_Lib_WaylandClient *lib, struct xdg_popup *xdg_popup)
{
	return lib->proxy_get_user_data((struct wl_proxy *) xdg_popup);
}

static inline uint32_t
lvkw_xdg_popup_get_version(const LVKW_Lib_WaylandClient *lib, struct xdg_popup *xdg_popup)
{
	return lib->proxy_get_version((struct wl_proxy *) xdg_popup);
}

static inline int
lvkw_xdg_popup_add_listener(const LVKW_Lib_WaylandClient *lib, struct xdg_popup *xdg_popup, const struct xdg_popup_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) xdg_popup, (void (**)(void)) listener, data);
}

static inline void
lvkw_xdg_popup_destroy(const LVKW_Lib_WaylandClient *lib, struct xdg_popup *xdg_popup)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_popup, XDG_POPUP_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_popup), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_xdg_popup_grab(const LVKW_Lib_WaylandClient *lib, struct xdg_popup *xdg_popup, struct wl_seat *seat, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_popup, XDG_POPUP_GRAB, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_popup), 0, seat, serial);
}

static inline void
lvkw_xdg_popup_reposition(const LVKW_Lib_WaylandClient *lib, struct xdg_popup *xdg_popup, struct xdg_positioner *positioner, uint32_t token)
{
	lib->proxy_marshal_flags((struct wl_proxy *) xdg_popup, XDG_POPUP_REPOSITION, NULL, lib->proxy_get_version((struct wl_proxy *) xdg_popup), 0, positioner, token);
}

#endif
