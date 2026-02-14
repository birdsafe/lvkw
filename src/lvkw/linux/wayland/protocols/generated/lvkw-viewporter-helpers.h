/* Generated from viewporter.xml */

#ifndef LVKW_WAYLAND_HELPERS_VIEWPORTER_H
#define LVKW_WAYLAND_HELPERS_VIEWPORTER_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct wp_viewporter;
struct wp_viewporter_listener;
struct wp_viewport;
struct wl_surface;
/* interface wp_viewporter */
static inline void
lvkw_wp_viewporter_set_user_data(LVKW_Context_WL *ctx, struct wp_viewporter *wp_viewporter, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wp_viewporter, user_data);
}

static inline void *
lvkw_wp_viewporter_get_user_data(LVKW_Context_WL *ctx, struct wp_viewporter *wp_viewporter)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wp_viewporter);
}

static inline uint32_t
lvkw_wp_viewporter_get_version(LVKW_Context_WL *ctx, struct wp_viewporter *wp_viewporter)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_viewporter);
}

static inline int
lvkw_wp_viewporter_add_listener(LVKW_Context_WL *ctx, struct wp_viewporter *wp_viewporter, const struct wp_viewporter_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wp_viewporter, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_viewporter_destroy(LVKW_Context_WL *ctx, struct wp_viewporter *wp_viewporter)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_viewporter, WP_VIEWPORTER_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_viewporter), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct wp_viewport *
lvkw_wp_viewporter_get_viewport(LVKW_Context_WL *ctx, struct wp_viewporter *wp_viewporter, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_viewporter, WP_VIEWPORTER_GET_VIEWPORT, &wp_viewport_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_viewporter), 0, NULL, surface);
	return (struct wp_viewport *) id;
}

struct wp_viewport;
struct wp_viewport_listener;
/* interface wp_viewport */
static inline void
lvkw_wp_viewport_set_user_data(LVKW_Context_WL *ctx, struct wp_viewport *wp_viewport, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wp_viewport, user_data);
}

static inline void *
lvkw_wp_viewport_get_user_data(LVKW_Context_WL *ctx, struct wp_viewport *wp_viewport)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wp_viewport);
}

static inline uint32_t
lvkw_wp_viewport_get_version(LVKW_Context_WL *ctx, struct wp_viewport *wp_viewport)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_viewport);
}

static inline int
lvkw_wp_viewport_add_listener(LVKW_Context_WL *ctx, struct wp_viewport *wp_viewport, const struct wp_viewport_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wp_viewport, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_viewport_destroy(LVKW_Context_WL *ctx, struct wp_viewport *wp_viewport)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_viewport, WP_VIEWPORT_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_viewport), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wp_viewport_set_source(LVKW_Context_WL *ctx, struct wp_viewport *wp_viewport, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_viewport, WP_VIEWPORT_SET_SOURCE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_viewport), 0, x, y, width, height);
}

static inline void
lvkw_wp_viewport_set_destination(LVKW_Context_WL *ctx, struct wp_viewport *wp_viewport, int32_t width, int32_t height)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_viewport, WP_VIEWPORT_SET_DESTINATION, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_viewport), 0, width, height);
}

#endif
