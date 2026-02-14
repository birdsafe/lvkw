/* Generated from pointer-constraints-unstable-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_POINTER_CONSTRAINTS_UNSTABLE_V1_H
#define LVKW_WAYLAND_HELPERS_POINTER_CONSTRAINTS_UNSTABLE_V1_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct zwp_pointer_constraints_v1;
struct zwp_pointer_constraints_v1_listener;
struct zwp_locked_pointer_v1;
struct wl_surface;
struct wl_pointer;
struct wl_region;
struct zwp_confined_pointer_v1;
struct wl_surface;
struct wl_pointer;
struct wl_region;
/* interface zwp_pointer_constraints_v1 */
static inline void
lvkw_zwp_pointer_constraints_v1_set_user_data(LVKW_Context_WL *ctx, struct zwp_pointer_constraints_v1 *zwp_pointer_constraints_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_pointer_constraints_v1, user_data);
}

static inline void *
lvkw_zwp_pointer_constraints_v1_get_user_data(LVKW_Context_WL *ctx, struct zwp_pointer_constraints_v1 *zwp_pointer_constraints_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_pointer_constraints_v1);
}

static inline uint32_t
lvkw_zwp_pointer_constraints_v1_get_version(LVKW_Context_WL *ctx, struct zwp_pointer_constraints_v1 *zwp_pointer_constraints_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_pointer_constraints_v1);
}

static inline int
lvkw_zwp_pointer_constraints_v1_add_listener(LVKW_Context_WL *ctx, struct zwp_pointer_constraints_v1 *zwp_pointer_constraints_v1, const struct zwp_pointer_constraints_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_pointer_constraints_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_pointer_constraints_v1_destroy(LVKW_Context_WL *ctx, struct zwp_pointer_constraints_v1 *zwp_pointer_constraints_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_pointer_constraints_v1, ZWP_POINTER_CONSTRAINTS_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_pointer_constraints_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct zwp_locked_pointer_v1 *
lvkw_zwp_pointer_constraints_v1_lock_pointer(LVKW_Context_WL *ctx, struct zwp_pointer_constraints_v1 *zwp_pointer_constraints_v1, struct wl_surface *surface, struct wl_pointer *pointer, struct wl_region *region, uint32_t lifetime)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_pointer_constraints_v1, ZWP_POINTER_CONSTRAINTS_V1_LOCK_POINTER, &zwp_locked_pointer_v1_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_pointer_constraints_v1), 0, NULL, surface, pointer, region, lifetime);
	return (struct zwp_locked_pointer_v1 *) id;
}

static inline struct zwp_confined_pointer_v1 *
lvkw_zwp_pointer_constraints_v1_confine_pointer(LVKW_Context_WL *ctx, struct zwp_pointer_constraints_v1 *zwp_pointer_constraints_v1, struct wl_surface *surface, struct wl_pointer *pointer, struct wl_region *region, uint32_t lifetime)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_pointer_constraints_v1, ZWP_POINTER_CONSTRAINTS_V1_CONFINE_POINTER, &zwp_confined_pointer_v1_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_pointer_constraints_v1), 0, NULL, surface, pointer, region, lifetime);
	return (struct zwp_confined_pointer_v1 *) id;
}

struct zwp_locked_pointer_v1;
struct zwp_locked_pointer_v1_listener;
struct wl_region;
/* interface zwp_locked_pointer_v1 */
static inline void
lvkw_zwp_locked_pointer_v1_set_user_data(LVKW_Context_WL *ctx, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_locked_pointer_v1, user_data);
}

static inline void *
lvkw_zwp_locked_pointer_v1_get_user_data(LVKW_Context_WL *ctx, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_locked_pointer_v1);
}

static inline uint32_t
lvkw_zwp_locked_pointer_v1_get_version(LVKW_Context_WL *ctx, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_locked_pointer_v1);
}

static inline int
lvkw_zwp_locked_pointer_v1_add_listener(LVKW_Context_WL *ctx, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1, const struct zwp_locked_pointer_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_locked_pointer_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_locked_pointer_v1_destroy(LVKW_Context_WL *ctx, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_locked_pointer_v1, ZWP_LOCKED_POINTER_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_locked_pointer_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_zwp_locked_pointer_v1_set_cursor_position_hint(LVKW_Context_WL *ctx, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_locked_pointer_v1, ZWP_LOCKED_POINTER_V1_SET_CURSOR_POSITION_HINT, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_locked_pointer_v1), 0, surface_x, surface_y);
}

static inline void
lvkw_zwp_locked_pointer_v1_set_region(LVKW_Context_WL *ctx, struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1, struct wl_region *region)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_locked_pointer_v1, ZWP_LOCKED_POINTER_V1_SET_REGION, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_locked_pointer_v1), 0, region);
}

struct zwp_confined_pointer_v1;
struct zwp_confined_pointer_v1_listener;
struct wl_region;
/* interface zwp_confined_pointer_v1 */
static inline void
lvkw_zwp_confined_pointer_v1_set_user_data(LVKW_Context_WL *ctx, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_confined_pointer_v1, user_data);
}

static inline void *
lvkw_zwp_confined_pointer_v1_get_user_data(LVKW_Context_WL *ctx, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_confined_pointer_v1);
}

static inline uint32_t
lvkw_zwp_confined_pointer_v1_get_version(LVKW_Context_WL *ctx, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_confined_pointer_v1);
}

static inline int
lvkw_zwp_confined_pointer_v1_add_listener(LVKW_Context_WL *ctx, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1, const struct zwp_confined_pointer_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_confined_pointer_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_confined_pointer_v1_destroy(LVKW_Context_WL *ctx, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_confined_pointer_v1, ZWP_CONFINED_POINTER_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_confined_pointer_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_zwp_confined_pointer_v1_set_region(LVKW_Context_WL *ctx, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1, struct wl_region *region)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_confined_pointer_v1, ZWP_CONFINED_POINTER_V1_SET_REGION, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_confined_pointer_v1), 0, region);
}

#endif
