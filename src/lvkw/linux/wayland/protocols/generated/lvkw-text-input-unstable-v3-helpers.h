/* Generated from text-input-unstable-v3.xml */

#ifndef LVKW_WAYLAND_HELPERS_TEXT_INPUT_UNSTABLE_V3_H
#define LVKW_WAYLAND_HELPERS_TEXT_INPUT_UNSTABLE_V3_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct zwp_text_input_manager_v3;
struct zwp_text_input_manager_v3_listener;
struct zwp_text_input_v3;
struct wl_seat;
/* interface zwp_text_input_manager_v3 */
static inline void
lvkw_zwp_text_input_manager_v3_set_user_data(LVKW_Context_WL *ctx, struct zwp_text_input_manager_v3 *zwp_text_input_manager_v3, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_text_input_manager_v3, user_data);
}

static inline void *
lvkw_zwp_text_input_manager_v3_get_user_data(LVKW_Context_WL *ctx, struct zwp_text_input_manager_v3 *zwp_text_input_manager_v3)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_text_input_manager_v3);
}

static inline uint32_t
lvkw_zwp_text_input_manager_v3_get_version(LVKW_Context_WL *ctx, struct zwp_text_input_manager_v3 *zwp_text_input_manager_v3)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_manager_v3);
}

static inline int
lvkw_zwp_text_input_manager_v3_add_listener(LVKW_Context_WL *ctx, struct zwp_text_input_manager_v3 *zwp_text_input_manager_v3, const struct zwp_text_input_manager_v3_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_text_input_manager_v3, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_text_input_manager_v3_destroy(LVKW_Context_WL *ctx, struct zwp_text_input_manager_v3 *zwp_text_input_manager_v3)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_manager_v3, ZWP_TEXT_INPUT_MANAGER_V3_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_manager_v3), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct zwp_text_input_v3 *
lvkw_zwp_text_input_manager_v3_get_text_input(LVKW_Context_WL *ctx, struct zwp_text_input_manager_v3 *zwp_text_input_manager_v3, struct wl_seat *seat)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_manager_v3, ZWP_TEXT_INPUT_MANAGER_V3_GET_TEXT_INPUT, &zwp_text_input_v3_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_manager_v3), 0, NULL, seat);
	return (struct zwp_text_input_v3 *) id;
}

struct zwp_text_input_v3;
struct zwp_text_input_v3_listener;
/* interface zwp_text_input_v3 */
static inline void
lvkw_zwp_text_input_v3_set_user_data(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) zwp_text_input_v3, user_data);
}

static inline void *
lvkw_zwp_text_input_v3_get_user_data(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) zwp_text_input_v3);
}

static inline uint32_t
lvkw_zwp_text_input_v3_get_version(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3);
}

static inline int
lvkw_zwp_text_input_v3_add_listener(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3, const struct zwp_text_input_v3_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) zwp_text_input_v3, (void (**)(void)) listener, data);
}

static inline void
lvkw_zwp_text_input_v3_destroy(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_zwp_text_input_v3_enable(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_ENABLE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), 0);
}

static inline void
lvkw_zwp_text_input_v3_disable(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_DISABLE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), 0);
}

static inline void
lvkw_zwp_text_input_v3_set_surrounding_text(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3, const char *text, int32_t cursor, int32_t anchor)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_SET_SURROUNDING_TEXT, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), 0, text, cursor, anchor);
}

static inline void
lvkw_zwp_text_input_v3_set_text_change_cause(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3, uint32_t cause)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_SET_TEXT_CHANGE_CAUSE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), 0, cause);
}

static inline void
lvkw_zwp_text_input_v3_set_content_type(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3, uint32_t hint, uint32_t purpose)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_SET_CONTENT_TYPE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), 0, hint, purpose);
}

static inline void
lvkw_zwp_text_input_v3_set_cursor_rectangle(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3, int32_t x, int32_t y, int32_t width, int32_t height)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_SET_CURSOR_RECTANGLE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), 0, x, y, width, height);
}

static inline void
lvkw_zwp_text_input_v3_commit(LVKW_Context_WL *ctx, struct zwp_text_input_v3 *zwp_text_input_v3)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) zwp_text_input_v3, ZWP_TEXT_INPUT_V3_COMMIT, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) zwp_text_input_v3), 0);
}

#endif
