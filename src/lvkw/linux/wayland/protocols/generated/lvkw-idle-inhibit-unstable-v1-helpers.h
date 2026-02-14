/* Generated from idle-inhibit-unstable-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_IDLE_INHIBIT_UNSTABLE_V1_H
#define LVKW_WAYLAND_HELPERS_IDLE_INHIBIT_UNSTABLE_V1_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct zwp_idle_inhibit_manager_v1;
struct zwp_idle_inhibit_manager_v1_listener;
struct zwp_idle_inhibitor_v1;
struct wl_surface;
/* interface zwp_idle_inhibit_manager_v1 */
static inline void
lvkw_zwp_idle_inhibit_manager_v1_set_user_data(LVKW_Context_WL *ctx, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_idle_inhibit_manager_v1, user_data);
}

static inline void *
lvkw_zwp_idle_inhibit_manager_v1_get_user_data(LVKW_Context_WL *ctx, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_idle_inhibit_manager_v1);
}

static inline uint32_t
lvkw_zwp_idle_inhibit_manager_v1_get_version(LVKW_Context_WL *ctx, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_idle_inhibit_manager_v1);
}

static inline int
lvkw_zwp_idle_inhibit_manager_v1_add_listener(LVKW_Context_WL *ctx, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1, const struct zwp_idle_inhibit_manager_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_idle_inhibit_manager_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_idle_inhibit_manager_v1_destroy(LVKW_Context_WL *ctx, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_idle_inhibit_manager_v1, ZWP_IDLE_INHIBIT_MANAGER_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_idle_inhibit_manager_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct zwp_idle_inhibitor_v1 *
lvkw_zwp_idle_inhibit_manager_v1_create_inhibitor(LVKW_Context_WL *ctx, struct zwp_idle_inhibit_manager_v1 *zwp_idle_inhibit_manager_v1, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_idle_inhibit_manager_v1, ZWP_IDLE_INHIBIT_MANAGER_V1_CREATE_INHIBITOR, &zwp_idle_inhibitor_v1_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_idle_inhibit_manager_v1), 0, NULL, surface);
	return (struct zwp_idle_inhibitor_v1 *) id;
}

struct zwp_idle_inhibitor_v1;
struct zwp_idle_inhibitor_v1_listener;
/* interface zwp_idle_inhibitor_v1 */
static inline void
lvkw_zwp_idle_inhibitor_v1_set_user_data(LVKW_Context_WL *ctx, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_idle_inhibitor_v1, user_data);
}

static inline void *
lvkw_zwp_idle_inhibitor_v1_get_user_data(LVKW_Context_WL *ctx, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_idle_inhibitor_v1);
}

static inline uint32_t
lvkw_zwp_idle_inhibitor_v1_get_version(LVKW_Context_WL *ctx, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_idle_inhibitor_v1);
}

static inline int
lvkw_zwp_idle_inhibitor_v1_add_listener(LVKW_Context_WL *ctx, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1, const struct zwp_idle_inhibitor_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_idle_inhibitor_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_idle_inhibitor_v1_destroy(LVKW_Context_WL *ctx, struct zwp_idle_inhibitor_v1 *zwp_idle_inhibitor_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_idle_inhibitor_v1, ZWP_IDLE_INHIBITOR_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_idle_inhibitor_v1), WL_MARSHAL_FLAG_DESTROY);
}

#endif
