/* Generated from tablet-v2.xml */

#ifndef LVKW_WAYLAND_HELPERS_TABLET_V2_H
#define LVKW_WAYLAND_HELPERS_TABLET_V2_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct zwp_tablet_manager_v2;
struct zwp_tablet_manager_v2_listener;
struct zwp_tablet_seat_v2;
struct wl_seat;
/* interface zwp_tablet_manager_v2 */
static inline void
lvkw_zwp_tablet_manager_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_manager_v2 *zwp_tablet_manager_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_manager_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_manager_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_manager_v2 *zwp_tablet_manager_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_manager_v2);
}

static inline uint32_t
lvkw_zwp_tablet_manager_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_manager_v2 *zwp_tablet_manager_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_manager_v2);
}

static inline int
lvkw_zwp_tablet_manager_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_manager_v2 *zwp_tablet_manager_v2, const struct zwp_tablet_manager_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_manager_v2, (void (**)(void)) listener, data);
}

static inline struct zwp_tablet_seat_v2 *
lvkw_zwp_tablet_manager_v2_get_tablet_seat(LVKW_Context_WL *ctx, struct zwp_tablet_manager_v2 *zwp_tablet_manager_v2, struct wl_seat *seat)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_manager_v2, ZWP_TABLET_MANAGER_V2_GET_TABLET_SEAT, &zwp_tablet_seat_v2_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_manager_v2), 0, NULL, seat);
	return (struct zwp_tablet_seat_v2 *) id;
}

static inline void
lvkw_zwp_tablet_manager_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_manager_v2 *zwp_tablet_manager_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_manager_v2, ZWP_TABLET_MANAGER_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_manager_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_seat_v2;
struct zwp_tablet_seat_v2_listener;
/* interface zwp_tablet_seat_v2 */
static inline void
lvkw_zwp_tablet_seat_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_seat_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_seat_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_seat_v2);
}

static inline uint32_t
lvkw_zwp_tablet_seat_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_seat_v2);
}

static inline int
lvkw_zwp_tablet_seat_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, const struct zwp_tablet_seat_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_seat_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_seat_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_seat_v2, ZWP_TABLET_SEAT_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_seat_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_tool_v2;
struct zwp_tablet_tool_v2_listener;
struct wl_surface;
/* interface zwp_tablet_tool_v2 */
static inline void
lvkw_zwp_tablet_tool_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_tool_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_tool_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_tool_v2);
}

static inline uint32_t
lvkw_zwp_tablet_tool_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_tool_v2);
}

static inline int
lvkw_zwp_tablet_tool_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, const struct zwp_tablet_tool_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_tool_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_tool_v2_set_cursor(LVKW_Context_WL *ctx, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2, uint32_t serial, struct wl_surface *surface, int32_t hotspot_x, int32_t hotspot_y)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_tool_v2, ZWP_TABLET_TOOL_V2_SET_CURSOR, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_tool_v2), 0, serial, surface, hotspot_x, hotspot_y);
}

static inline void
lvkw_zwp_tablet_tool_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_tool_v2 *zwp_tablet_tool_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_tool_v2, ZWP_TABLET_TOOL_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_tool_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_v2;
struct zwp_tablet_v2_listener;
/* interface zwp_tablet_v2 */
static inline void
lvkw_zwp_tablet_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_v2 *zwp_tablet_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_v2 *zwp_tablet_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_v2);
}

static inline uint32_t
lvkw_zwp_tablet_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_v2 *zwp_tablet_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_v2);
}

static inline int
lvkw_zwp_tablet_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_v2 *zwp_tablet_v2, const struct zwp_tablet_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_v2 *zwp_tablet_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_v2, ZWP_TABLET_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_pad_ring_v2;
struct zwp_tablet_pad_ring_v2_listener;
/* interface zwp_tablet_pad_ring_v2 */
static inline void
lvkw_zwp_tablet_pad_ring_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_ring_v2 *zwp_tablet_pad_ring_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_pad_ring_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_pad_ring_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_ring_v2 *zwp_tablet_pad_ring_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_pad_ring_v2);
}

static inline uint32_t
lvkw_zwp_tablet_pad_ring_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_pad_ring_v2 *zwp_tablet_pad_ring_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_ring_v2);
}

static inline int
lvkw_zwp_tablet_pad_ring_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_pad_ring_v2 *zwp_tablet_pad_ring_v2, const struct zwp_tablet_pad_ring_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_pad_ring_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_pad_ring_v2_set_feedback(LVKW_Context_WL *ctx, struct zwp_tablet_pad_ring_v2 *zwp_tablet_pad_ring_v2, const char *description, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_ring_v2, ZWP_TABLET_PAD_RING_V2_SET_FEEDBACK, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_ring_v2), 0, description, serial);
}

static inline void
lvkw_zwp_tablet_pad_ring_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_pad_ring_v2 *zwp_tablet_pad_ring_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_ring_v2, ZWP_TABLET_PAD_RING_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_ring_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_pad_strip_v2;
struct zwp_tablet_pad_strip_v2_listener;
/* interface zwp_tablet_pad_strip_v2 */
static inline void
lvkw_zwp_tablet_pad_strip_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_strip_v2 *zwp_tablet_pad_strip_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_pad_strip_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_pad_strip_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_strip_v2 *zwp_tablet_pad_strip_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_pad_strip_v2);
}

static inline uint32_t
lvkw_zwp_tablet_pad_strip_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_pad_strip_v2 *zwp_tablet_pad_strip_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_strip_v2);
}

static inline int
lvkw_zwp_tablet_pad_strip_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_pad_strip_v2 *zwp_tablet_pad_strip_v2, const struct zwp_tablet_pad_strip_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_pad_strip_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_pad_strip_v2_set_feedback(LVKW_Context_WL *ctx, struct zwp_tablet_pad_strip_v2 *zwp_tablet_pad_strip_v2, const char *description, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_strip_v2, ZWP_TABLET_PAD_STRIP_V2_SET_FEEDBACK, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_strip_v2), 0, description, serial);
}

static inline void
lvkw_zwp_tablet_pad_strip_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_pad_strip_v2 *zwp_tablet_pad_strip_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_strip_v2, ZWP_TABLET_PAD_STRIP_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_strip_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_pad_group_v2;
struct zwp_tablet_pad_group_v2_listener;
/* interface zwp_tablet_pad_group_v2 */
static inline void
lvkw_zwp_tablet_pad_group_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_group_v2 *zwp_tablet_pad_group_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_pad_group_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_pad_group_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_group_v2 *zwp_tablet_pad_group_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_pad_group_v2);
}

static inline uint32_t
lvkw_zwp_tablet_pad_group_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_pad_group_v2 *zwp_tablet_pad_group_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_group_v2);
}

static inline int
lvkw_zwp_tablet_pad_group_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_pad_group_v2 *zwp_tablet_pad_group_v2, const struct zwp_tablet_pad_group_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_pad_group_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_pad_group_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_pad_group_v2 *zwp_tablet_pad_group_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_group_v2, ZWP_TABLET_PAD_GROUP_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_group_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_pad_v2;
struct zwp_tablet_pad_v2_listener;
/* interface zwp_tablet_pad_v2 */
static inline void
lvkw_zwp_tablet_pad_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_v2 *zwp_tablet_pad_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_pad_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_pad_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_v2 *zwp_tablet_pad_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_pad_v2);
}

static inline uint32_t
lvkw_zwp_tablet_pad_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_pad_v2 *zwp_tablet_pad_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_v2);
}

static inline int
lvkw_zwp_tablet_pad_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_pad_v2 *zwp_tablet_pad_v2, const struct zwp_tablet_pad_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_pad_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_pad_v2_set_feedback(LVKW_Context_WL *ctx, struct zwp_tablet_pad_v2 *zwp_tablet_pad_v2, uint32_t button, const char *description, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_v2, ZWP_TABLET_PAD_V2_SET_FEEDBACK, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_v2), 0, button, description, serial);
}

static inline void
lvkw_zwp_tablet_pad_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_pad_v2 *zwp_tablet_pad_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_v2, ZWP_TABLET_PAD_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_v2), WL_MARSHAL_FLAG_DESTROY);
}

struct zwp_tablet_pad_dial_v2;
struct zwp_tablet_pad_dial_v2_listener;
/* interface zwp_tablet_pad_dial_v2 */
static inline void
lvkw_zwp_tablet_pad_dial_v2_set_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_dial_v2 *zwp_tablet_pad_dial_v2, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_tablet_pad_dial_v2, user_data);
}

static inline void *
lvkw_zwp_tablet_pad_dial_v2_get_user_data(LVKW_Context_WL *ctx, struct zwp_tablet_pad_dial_v2 *zwp_tablet_pad_dial_v2)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_tablet_pad_dial_v2);
}

static inline uint32_t
lvkw_zwp_tablet_pad_dial_v2_get_version(LVKW_Context_WL *ctx, struct zwp_tablet_pad_dial_v2 *zwp_tablet_pad_dial_v2)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_dial_v2);
}

static inline int
lvkw_zwp_tablet_pad_dial_v2_add_listener(LVKW_Context_WL *ctx, struct zwp_tablet_pad_dial_v2 *zwp_tablet_pad_dial_v2, const struct zwp_tablet_pad_dial_v2_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_tablet_pad_dial_v2, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_tablet_pad_dial_v2_set_feedback(LVKW_Context_WL *ctx, struct zwp_tablet_pad_dial_v2 *zwp_tablet_pad_dial_v2, const char *description, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_dial_v2, ZWP_TABLET_PAD_DIAL_V2_SET_FEEDBACK, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_dial_v2), 0, description, serial);
}

static inline void
lvkw_zwp_tablet_pad_dial_v2_destroy(LVKW_Context_WL *ctx, struct zwp_tablet_pad_dial_v2 *zwp_tablet_pad_dial_v2)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_tablet_pad_dial_v2, ZWP_TABLET_PAD_DIAL_V2_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_tablet_pad_dial_v2), WL_MARSHAL_FLAG_DESTROY);
}

#endif
