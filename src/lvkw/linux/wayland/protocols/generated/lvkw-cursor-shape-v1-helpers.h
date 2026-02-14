/* Generated from cursor-shape-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_CURSOR_SHAPE_V1_H
#define LVKW_WAYLAND_HELPERS_CURSOR_SHAPE_V1_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct wp_cursor_shape_manager_v1;
struct wp_cursor_shape_manager_v1_listener;
struct wp_cursor_shape_device_v1;
struct wl_pointer;
struct wp_cursor_shape_device_v1;
struct zwp_tablet_tool_v2;
/* interface wp_cursor_shape_manager_v1 */
static inline void
lvkw_wp_cursor_shape_manager_v1_set_user_data(LVKW_Context_WL *ctx, struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wp_cursor_shape_manager_v1, user_data);
}

static inline void *
lvkw_wp_cursor_shape_manager_v1_get_user_data(LVKW_Context_WL *ctx, struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wp_cursor_shape_manager_v1);
}

static inline uint32_t
lvkw_wp_cursor_shape_manager_v1_get_version(LVKW_Context_WL *ctx, struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_cursor_shape_manager_v1);
}

static inline int
lvkw_wp_cursor_shape_manager_v1_add_listener(LVKW_Context_WL *ctx, struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1, const struct wp_cursor_shape_manager_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wp_cursor_shape_manager_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_cursor_shape_manager_v1_destroy(LVKW_Context_WL *ctx, struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_cursor_shape_manager_v1, WP_CURSOR_SHAPE_MANAGER_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_cursor_shape_manager_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct wp_cursor_shape_device_v1 *
lvkw_wp_cursor_shape_manager_v1_get_pointer(LVKW_Context_WL *ctx, struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1, struct wl_pointer *pointer)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_cursor_shape_manager_v1, WP_CURSOR_SHAPE_MANAGER_V1_GET_POINTER, &wp_cursor_shape_device_v1_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_cursor_shape_manager_v1), 0, NULL, pointer);
	return (struct wp_cursor_shape_device_v1 *) id;
}

static inline struct wp_cursor_shape_device_v1 *
lvkw_wp_cursor_shape_manager_v1_get_tablet_tool_v2(LVKW_Context_WL *ctx, struct wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1, struct zwp_tablet_tool_v2 *tablet_tool)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_cursor_shape_manager_v1, WP_CURSOR_SHAPE_MANAGER_V1_GET_TABLET_TOOL_V2, &wp_cursor_shape_device_v1_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_cursor_shape_manager_v1), 0, NULL, tablet_tool);
	return (struct wp_cursor_shape_device_v1 *) id;
}

struct wp_cursor_shape_device_v1;
struct wp_cursor_shape_device_v1_listener;
/* interface wp_cursor_shape_device_v1 */
static inline void
lvkw_wp_cursor_shape_device_v1_set_user_data(LVKW_Context_WL *ctx, struct wp_cursor_shape_device_v1 *wp_cursor_shape_device_v1, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wp_cursor_shape_device_v1, user_data);
}

static inline void *
lvkw_wp_cursor_shape_device_v1_get_user_data(LVKW_Context_WL *ctx, struct wp_cursor_shape_device_v1 *wp_cursor_shape_device_v1)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wp_cursor_shape_device_v1);
}

static inline uint32_t
lvkw_wp_cursor_shape_device_v1_get_version(LVKW_Context_WL *ctx, struct wp_cursor_shape_device_v1 *wp_cursor_shape_device_v1)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_cursor_shape_device_v1);
}

static inline int
lvkw_wp_cursor_shape_device_v1_add_listener(LVKW_Context_WL *ctx, struct wp_cursor_shape_device_v1 *wp_cursor_shape_device_v1, const struct wp_cursor_shape_device_v1_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wp_cursor_shape_device_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_wp_cursor_shape_device_v1_destroy(LVKW_Context_WL *ctx, struct wp_cursor_shape_device_v1 *wp_cursor_shape_device_v1)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_cursor_shape_device_v1, WP_CURSOR_SHAPE_DEVICE_V1_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_cursor_shape_device_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wp_cursor_shape_device_v1_set_shape(LVKW_Context_WL *ctx, struct wp_cursor_shape_device_v1 *wp_cursor_shape_device_v1, uint32_t serial, uint32_t shape)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wp_cursor_shape_device_v1, WP_CURSOR_SHAPE_DEVICE_V1_SET_SHAPE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wp_cursor_shape_device_v1), 0, serial, shape);
}

#endif
