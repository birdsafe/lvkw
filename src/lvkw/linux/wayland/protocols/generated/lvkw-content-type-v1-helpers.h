/* Generated from content-type-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_CONTENT_TYPE_V1_H
#define LVKW_WAYLAND_HELPERS_CONTENT_TYPE_V1_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct wp_content_type_manager_v1;
struct wp_content_type_manager_v1_listener;
struct wp_content_type_v1;
struct wl_surface;
/* interface wp_content_type_manager_v1 */
static inline void
lvkw_wp_content_type_manager_v1_set_user_data(LVKW_Context_WL *ctx, struct wp_content_type_manager_v1 *wp_content_type_manager_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wp_content_type_manager_v1, user_data);
}

static inline void *
lvkw_wp_content_type_manager_v1_get_user_data(LVKW_Context_WL *ctx, struct wp_content_type_manager_v1 *wp_content_type_manager_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wp_content_type_manager_v1);
}

static inline uint32_t
lvkw_wp_content_type_manager_v1_get_version(LVKW_Context_WL *ctx, struct wp_content_type_manager_v1 *wp_content_type_manager_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_content_type_manager_v1);
}

static inline int
lvkw_wp_content_type_manager_v1_add_listener(LVKW_Context_WL *ctx, struct wp_content_type_manager_v1 *wp_content_type_manager_v1, const struct wp_content_type_manager_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wp_content_type_manager_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_content_type_manager_v1_destroy(LVKW_Context_WL *ctx, struct wp_content_type_manager_v1 *wp_content_type_manager_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_content_type_manager_v1, WP_CONTENT_TYPE_MANAGER_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_content_type_manager_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct wp_content_type_v1 *
lvkw_wp_content_type_manager_v1_get_surface_content_type(LVKW_Context_WL *ctx, struct wp_content_type_manager_v1 *wp_content_type_manager_v1, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_content_type_manager_v1, WP_CONTENT_TYPE_MANAGER_V1_GET_SURFACE_CONTENT_TYPE, &wp_content_type_v1_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_content_type_manager_v1), 0, NULL, surface);
	return (struct wp_content_type_v1 *) id;
}

struct wp_content_type_v1;
struct wp_content_type_v1_listener;
/* interface wp_content_type_v1 */
static inline void
lvkw_wp_content_type_v1_set_user_data(LVKW_Context_WL *ctx, struct wp_content_type_v1 *wp_content_type_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wp_content_type_v1, user_data);
}

static inline void *
lvkw_wp_content_type_v1_get_user_data(LVKW_Context_WL *ctx, struct wp_content_type_v1 *wp_content_type_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wp_content_type_v1);
}

static inline uint32_t
lvkw_wp_content_type_v1_get_version(LVKW_Context_WL *ctx, struct wp_content_type_v1 *wp_content_type_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_content_type_v1);
}

static inline int
lvkw_wp_content_type_v1_add_listener(LVKW_Context_WL *ctx, struct wp_content_type_v1 *wp_content_type_v1, const struct wp_content_type_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wp_content_type_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_content_type_v1_destroy(LVKW_Context_WL *ctx, struct wp_content_type_v1 *wp_content_type_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_content_type_v1, WP_CONTENT_TYPE_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_content_type_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wp_content_type_v1_set_content_type(LVKW_Context_WL *ctx, struct wp_content_type_v1 *wp_content_type_v1, uint32_t content_type)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_content_type_v1, WP_CONTENT_TYPE_V1_SET_CONTENT_TYPE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_content_type_v1), 0, content_type);
}

#endif
