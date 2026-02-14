/* Generated from xdg-output-unstable-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_XDG_OUTPUT_UNSTABLE_V1_H
#define LVKW_WAYLAND_HELPERS_XDG_OUTPUT_UNSTABLE_V1_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct zxdg_output_manager_v1;
struct zxdg_output_manager_v1_listener;
struct zxdg_output_v1;
struct wl_output;
/* interface zxdg_output_manager_v1 */
static inline void
lvkw_zxdg_output_manager_v1_set_user_data(LVKW_Context_WL *ctx, struct zxdg_output_manager_v1 *zxdg_output_manager_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zxdg_output_manager_v1, user_data);
}

static inline void *
lvkw_zxdg_output_manager_v1_get_user_data(LVKW_Context_WL *ctx, struct zxdg_output_manager_v1 *zxdg_output_manager_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zxdg_output_manager_v1);
}

static inline uint32_t
lvkw_zxdg_output_manager_v1_get_version(LVKW_Context_WL *ctx, struct zxdg_output_manager_v1 *zxdg_output_manager_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zxdg_output_manager_v1);
}

static inline int
lvkw_zxdg_output_manager_v1_add_listener(LVKW_Context_WL *ctx, struct zxdg_output_manager_v1 *zxdg_output_manager_v1, const struct zxdg_output_manager_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zxdg_output_manager_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zxdg_output_manager_v1_destroy(LVKW_Context_WL *ctx, struct zxdg_output_manager_v1 *zxdg_output_manager_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zxdg_output_manager_v1, ZXDG_OUTPUT_MANAGER_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zxdg_output_manager_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct zxdg_output_v1 *
lvkw_zxdg_output_manager_v1_get_xdg_output(LVKW_Context_WL *ctx, struct zxdg_output_manager_v1 *zxdg_output_manager_v1, struct wl_output *output)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zxdg_output_manager_v1, ZXDG_OUTPUT_MANAGER_V1_GET_XDG_OUTPUT, &zxdg_output_v1_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zxdg_output_manager_v1), 0, NULL, output);
	return (struct zxdg_output_v1 *) id;
}

struct zxdg_output_v1;
struct zxdg_output_v1_listener;
/* interface zxdg_output_v1 */
static inline void
lvkw_zxdg_output_v1_set_user_data(LVKW_Context_WL *ctx, struct zxdg_output_v1 *zxdg_output_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zxdg_output_v1, user_data);
}

static inline void *
lvkw_zxdg_output_v1_get_user_data(LVKW_Context_WL *ctx, struct zxdg_output_v1 *zxdg_output_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zxdg_output_v1);
}

static inline uint32_t
lvkw_zxdg_output_v1_get_version(LVKW_Context_WL *ctx, struct zxdg_output_v1 *zxdg_output_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zxdg_output_v1);
}

static inline int
lvkw_zxdg_output_v1_add_listener(LVKW_Context_WL *ctx, struct zxdg_output_v1 *zxdg_output_v1, const struct zxdg_output_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zxdg_output_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_zxdg_output_v1_destroy(LVKW_Context_WL *ctx, struct zxdg_output_v1 *zxdg_output_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zxdg_output_v1, ZXDG_OUTPUT_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zxdg_output_v1), WL_MARSHAL_FLAG_DESTROY);
}

#endif
