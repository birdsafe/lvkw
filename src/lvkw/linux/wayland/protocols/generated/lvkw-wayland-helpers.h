/* Generated from wayland.xml */

#ifndef LVKW_WAYLAND_HELPERS_WAYLAND_H
#define LVKW_WAYLAND_HELPERS_WAYLAND_H

#include <stdint.h>
#include <stddef.h>

typedef struct LVKW_Context_WL LVKW_Context_WL;

struct wl_display;
struct wl_display_listener;
struct wl_callback;
struct wl_registry;
/* interface wl_display */
static inline void
lvkw_wl_display_set_user_data(LVKW_Context_WL *ctx, struct wl_display *wl_display, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_display, user_data);
}

static inline void *
lvkw_wl_display_get_user_data(LVKW_Context_WL *ctx, struct wl_display *wl_display)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_display);
}

static inline uint32_t
lvkw_wl_display_get_version(LVKW_Context_WL *ctx, struct wl_display *wl_display)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_display);
}

static inline int
lvkw_wl_display_add_listener(LVKW_Context_WL *ctx, struct wl_display *wl_display, const struct wl_display_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_display, (void (**)(void)) listener, data);
}

static inline struct wl_callback *
lvkw_wl_display_sync(LVKW_Context_WL *ctx, struct wl_display *wl_display)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_display, WL_DISPLAY_SYNC, &wl_callback_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_display), 0, NULL);
	return (struct wl_callback *) id;
}

static inline struct wl_registry *
lvkw_wl_display_get_registry(LVKW_Context_WL *ctx, struct wl_display *wl_display)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_display, WL_DISPLAY_GET_REGISTRY, &wl_registry_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_display), 0, NULL);
	return (struct wl_registry *) id;
}

static inline void
lvkw_wl_display_destroy(LVKW_Context_WL *ctx, struct wl_display *wl_display)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_display);
}

struct wl_registry;
struct wl_registry_listener;
/* interface wl_registry */
static inline void
lvkw_wl_registry_set_user_data(LVKW_Context_WL *ctx, struct wl_registry *wl_registry, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_registry, user_data);
}

static inline void *
lvkw_wl_registry_get_user_data(LVKW_Context_WL *ctx, struct wl_registry *wl_registry)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_registry);
}

static inline uint32_t
lvkw_wl_registry_get_version(LVKW_Context_WL *ctx, struct wl_registry *wl_registry)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_registry);
}

static inline int
lvkw_wl_registry_add_listener(LVKW_Context_WL *ctx, struct wl_registry *wl_registry, const struct wl_registry_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_registry, (void (**)(void)) listener, data);
}

static inline void *
lvkw_wl_registry_bind(LVKW_Context_WL *ctx, struct wl_registry *wl_registry, uint32_t name, const struct wl_interface *interface, uint32_t version)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_registry, WL_REGISTRY_BIND, interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_registry), 0, name, interface->name, version, NULL);
	return (void *) id;
}

static inline void
lvkw_wl_registry_destroy(LVKW_Context_WL *ctx, struct wl_registry *wl_registry)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_registry);
}

struct wl_callback;
struct wl_callback_listener;
/* interface wl_callback */
static inline void
lvkw_wl_callback_set_user_data(LVKW_Context_WL *ctx, struct wl_callback *wl_callback, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_callback, user_data);
}

static inline void *
lvkw_wl_callback_get_user_data(LVKW_Context_WL *ctx, struct wl_callback *wl_callback)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_callback);
}

static inline uint32_t
lvkw_wl_callback_get_version(LVKW_Context_WL *ctx, struct wl_callback *wl_callback)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_callback);
}

static inline int
lvkw_wl_callback_add_listener(LVKW_Context_WL *ctx, struct wl_callback *wl_callback, const struct wl_callback_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_callback, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_callback_destroy(LVKW_Context_WL *ctx, struct wl_callback *wl_callback)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_callback);
}

struct wl_compositor;
struct wl_compositor_listener;
struct wl_surface;
struct wl_region;
/* interface wl_compositor */
static inline void
lvkw_wl_compositor_set_user_data(LVKW_Context_WL *ctx, struct wl_compositor *wl_compositor, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_compositor, user_data);
}

static inline void *
lvkw_wl_compositor_get_user_data(LVKW_Context_WL *ctx, struct wl_compositor *wl_compositor)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_compositor);
}

static inline uint32_t
lvkw_wl_compositor_get_version(LVKW_Context_WL *ctx, struct wl_compositor *wl_compositor)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_compositor);
}

static inline int
lvkw_wl_compositor_add_listener(LVKW_Context_WL *ctx, struct wl_compositor *wl_compositor, const struct wl_compositor_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_compositor, (void (**)(void)) listener, data);
}

static inline struct wl_surface *
lvkw_wl_compositor_create_surface(LVKW_Context_WL *ctx, struct wl_compositor *wl_compositor)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_compositor, WL_COMPOSITOR_CREATE_SURFACE, &wl_surface_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_compositor), 0, NULL);
	return (struct wl_surface *) id;
}

static inline struct wl_region *
lvkw_wl_compositor_create_region(LVKW_Context_WL *ctx, struct wl_compositor *wl_compositor)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_compositor, WL_COMPOSITOR_CREATE_REGION, &wl_region_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_compositor), 0, NULL);
	return (struct wl_region *) id;
}

static inline void
lvkw_wl_compositor_destroy(LVKW_Context_WL *ctx, struct wl_compositor *wl_compositor)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_compositor);
}

struct wl_shm_pool;
struct wl_shm_pool_listener;
struct wl_buffer;
/* interface wl_shm_pool */
static inline void
lvkw_wl_shm_pool_set_user_data(LVKW_Context_WL *ctx, struct wl_shm_pool *wl_shm_pool, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_shm_pool, user_data);
}

static inline void *
lvkw_wl_shm_pool_get_user_data(LVKW_Context_WL *ctx, struct wl_shm_pool *wl_shm_pool)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_shm_pool);
}

static inline uint32_t
lvkw_wl_shm_pool_get_version(LVKW_Context_WL *ctx, struct wl_shm_pool *wl_shm_pool)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shm_pool);
}

static inline int
lvkw_wl_shm_pool_add_listener(LVKW_Context_WL *ctx, struct wl_shm_pool *wl_shm_pool, const struct wl_shm_pool_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_shm_pool, (void (**)(void)) listener, data);
}

static inline struct wl_buffer *
lvkw_wl_shm_pool_create_buffer(LVKW_Context_WL *ctx, struct wl_shm_pool *wl_shm_pool, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shm_pool, WL_SHM_POOL_CREATE_BUFFER, &wl_buffer_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shm_pool), 0, NULL, offset, width, height, stride, format);
	return (struct wl_buffer *) id;
}

static inline void
lvkw_wl_shm_pool_destroy(LVKW_Context_WL *ctx, struct wl_shm_pool *wl_shm_pool)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shm_pool, WL_SHM_POOL_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shm_pool), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_shm_pool_resize(LVKW_Context_WL *ctx, struct wl_shm_pool *wl_shm_pool, int32_t size)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shm_pool, WL_SHM_POOL_RESIZE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shm_pool), 0, size);
}

struct wl_shm;
struct wl_shm_listener;
struct wl_shm_pool;
/* interface wl_shm */
static inline void
lvkw_wl_shm_set_user_data(LVKW_Context_WL *ctx, struct wl_shm *wl_shm, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_shm, user_data);
}

static inline void *
lvkw_wl_shm_get_user_data(LVKW_Context_WL *ctx, struct wl_shm *wl_shm)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_shm);
}

static inline uint32_t
lvkw_wl_shm_get_version(LVKW_Context_WL *ctx, struct wl_shm *wl_shm)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shm);
}

static inline int
lvkw_wl_shm_add_listener(LVKW_Context_WL *ctx, struct wl_shm *wl_shm, const struct wl_shm_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_shm, (void (**)(void)) listener, data);
}

static inline struct wl_shm_pool *
lvkw_wl_shm_create_pool(LVKW_Context_WL *ctx, struct wl_shm *wl_shm, int32_t fd, int32_t size)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shm, WL_SHM_CREATE_POOL, &wl_shm_pool_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shm), 0, NULL, fd, size);
	return (struct wl_shm_pool *) id;
}

static inline void
lvkw_wl_shm_destroy(LVKW_Context_WL *ctx, struct wl_shm *wl_shm)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_shm);
}

struct wl_buffer;
struct wl_buffer_listener;
/* interface wl_buffer */
static inline void
lvkw_wl_buffer_set_user_data(LVKW_Context_WL *ctx, struct wl_buffer *wl_buffer, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_buffer, user_data);
}

static inline void *
lvkw_wl_buffer_get_user_data(LVKW_Context_WL *ctx, struct wl_buffer *wl_buffer)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_buffer);
}

static inline uint32_t
lvkw_wl_buffer_get_version(LVKW_Context_WL *ctx, struct wl_buffer *wl_buffer)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_buffer);
}

static inline int
lvkw_wl_buffer_add_listener(LVKW_Context_WL *ctx, struct wl_buffer *wl_buffer, const struct wl_buffer_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_buffer, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_buffer_destroy(LVKW_Context_WL *ctx, struct wl_buffer *wl_buffer)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_buffer, WL_BUFFER_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_buffer), WL_MARSHAL_FLAG_DESTROY);
}

struct wl_data_offer;
struct wl_data_offer_listener;
/* interface wl_data_offer */
static inline void
lvkw_wl_data_offer_set_user_data(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_data_offer, user_data);
}

static inline void *
lvkw_wl_data_offer_get_user_data(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_data_offer);
}

static inline uint32_t
lvkw_wl_data_offer_get_version(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_offer);
}

static inline int
lvkw_wl_data_offer_add_listener(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer, const struct wl_data_offer_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_data_offer, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_data_offer_accept(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer, uint32_t serial, const char *mime_type)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_ACCEPT, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_offer), 0, serial, mime_type);
}

static inline void
lvkw_wl_data_offer_receive(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer, const char *mime_type, int32_t fd)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_RECEIVE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_offer), 0, mime_type, fd);
}

static inline void
lvkw_wl_data_offer_destroy(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_offer), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_data_offer_finish(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_FINISH, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_offer), 0);
}

static inline void
lvkw_wl_data_offer_set_actions(LVKW_Context_WL *ctx, struct wl_data_offer *wl_data_offer, uint32_t dnd_actions, uint32_t preferred_action)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_SET_ACTIONS, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_offer), 0, dnd_actions, preferred_action);
}

struct wl_data_source;
struct wl_data_source_listener;
/* interface wl_data_source */
static inline void
lvkw_wl_data_source_set_user_data(LVKW_Context_WL *ctx, struct wl_data_source *wl_data_source, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_data_source, user_data);
}

static inline void *
lvkw_wl_data_source_get_user_data(LVKW_Context_WL *ctx, struct wl_data_source *wl_data_source)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_data_source);
}

static inline uint32_t
lvkw_wl_data_source_get_version(LVKW_Context_WL *ctx, struct wl_data_source *wl_data_source)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_source);
}

static inline int
lvkw_wl_data_source_add_listener(LVKW_Context_WL *ctx, struct wl_data_source *wl_data_source, const struct wl_data_source_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_data_source, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_data_source_offer(LVKW_Context_WL *ctx, struct wl_data_source *wl_data_source, const char *mime_type)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_source, WL_DATA_SOURCE_OFFER, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_source), 0, mime_type);
}

static inline void
lvkw_wl_data_source_destroy(LVKW_Context_WL *ctx, struct wl_data_source *wl_data_source)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_source, WL_DATA_SOURCE_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_source), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_data_source_set_actions(LVKW_Context_WL *ctx, struct wl_data_source *wl_data_source, uint32_t dnd_actions)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_source, WL_DATA_SOURCE_SET_ACTIONS, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_source), 0, dnd_actions);
}

struct wl_data_device;
struct wl_data_device_listener;
struct wl_data_source;
struct wl_surface;
struct wl_surface;
struct wl_data_source;
/* interface wl_data_device */
static inline void
lvkw_wl_data_device_set_user_data(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_data_device, user_data);
}

static inline void *
lvkw_wl_data_device_get_user_data(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_data_device);
}

static inline uint32_t
lvkw_wl_data_device_get_version(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_device);
}

static inline int
lvkw_wl_data_device_add_listener(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device, const struct wl_data_device_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_data_device, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_data_device_start_drag(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device, struct wl_data_source *source, struct wl_surface *origin, struct wl_surface *icon, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_device, WL_DATA_DEVICE_START_DRAG, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_device), 0, source, origin, icon, serial);
}

static inline void
lvkw_wl_data_device_set_selection(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device, struct wl_data_source *source, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_device, WL_DATA_DEVICE_SET_SELECTION, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_device), 0, source, serial);
}

static inline void
lvkw_wl_data_device_release(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_device, WL_DATA_DEVICE_RELEASE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_device), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_data_device_destroy(LVKW_Context_WL *ctx, struct wl_data_device *wl_data_device)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_data_device);
}

struct wl_data_device_manager;
struct wl_data_device_manager_listener;
struct wl_data_source;
struct wl_data_device;
struct wl_seat;
/* interface wl_data_device_manager */
static inline void
lvkw_wl_data_device_manager_set_user_data(LVKW_Context_WL *ctx, struct wl_data_device_manager *wl_data_device_manager, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_data_device_manager, user_data);
}

static inline void *
lvkw_wl_data_device_manager_get_user_data(LVKW_Context_WL *ctx, struct wl_data_device_manager *wl_data_device_manager)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_data_device_manager);
}

static inline uint32_t
lvkw_wl_data_device_manager_get_version(LVKW_Context_WL *ctx, struct wl_data_device_manager *wl_data_device_manager)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_device_manager);
}

static inline int
lvkw_wl_data_device_manager_add_listener(LVKW_Context_WL *ctx, struct wl_data_device_manager *wl_data_device_manager, const struct wl_data_device_manager_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_data_device_manager, (void (**)(void)) listener, data);
}

static inline struct wl_data_source *
lvkw_wl_data_device_manager_create_data_source(LVKW_Context_WL *ctx, struct wl_data_device_manager *wl_data_device_manager)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_device_manager, WL_DATA_DEVICE_MANAGER_CREATE_DATA_SOURCE, &wl_data_source_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_device_manager), 0, NULL);
	return (struct wl_data_source *) id;
}

static inline struct wl_data_device *
lvkw_wl_data_device_manager_get_data_device(LVKW_Context_WL *ctx, struct wl_data_device_manager *wl_data_device_manager, struct wl_seat *seat)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_data_device_manager, WL_DATA_DEVICE_MANAGER_GET_DATA_DEVICE, &wl_data_device_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_data_device_manager), 0, NULL, seat);
	return (struct wl_data_device *) id;
}

static inline void
lvkw_wl_data_device_manager_destroy(LVKW_Context_WL *ctx, struct wl_data_device_manager *wl_data_device_manager)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_data_device_manager);
}

struct wl_shell;
struct wl_shell_listener;
struct wl_shell_surface;
struct wl_surface;
/* interface wl_shell */
static inline void
lvkw_wl_shell_set_user_data(LVKW_Context_WL *ctx, struct wl_shell *wl_shell, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_shell, user_data);
}

static inline void *
lvkw_wl_shell_get_user_data(LVKW_Context_WL *ctx, struct wl_shell *wl_shell)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_shell);
}

static inline uint32_t
lvkw_wl_shell_get_version(LVKW_Context_WL *ctx, struct wl_shell *wl_shell)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell);
}

static inline int
lvkw_wl_shell_add_listener(LVKW_Context_WL *ctx, struct wl_shell *wl_shell, const struct wl_shell_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_shell, (void (**)(void)) listener, data);
}

static inline struct wl_shell_surface *
lvkw_wl_shell_get_shell_surface(LVKW_Context_WL *ctx, struct wl_shell *wl_shell, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell, WL_SHELL_GET_SHELL_SURFACE, &wl_shell_surface_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell), 0, NULL, surface);
	return (struct wl_shell_surface *) id;
}

static inline void
lvkw_wl_shell_destroy(LVKW_Context_WL *ctx, struct wl_shell *wl_shell)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_shell);
}

struct wl_shell_surface;
struct wl_shell_surface_listener;
struct wl_seat;
struct wl_seat;
struct wl_surface;
struct wl_output;
struct wl_seat;
struct wl_surface;
struct wl_output;
/* interface wl_shell_surface */
static inline void
lvkw_wl_shell_surface_set_user_data(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_shell_surface, user_data);
}

static inline void *
lvkw_wl_shell_surface_get_user_data(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_shell_surface);
}

static inline uint32_t
lvkw_wl_shell_surface_get_version(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface);
}

static inline int
lvkw_wl_shell_surface_add_listener(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, const struct wl_shell_surface_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_shell_surface, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_shell_surface_pong(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_PONG, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, serial);
}

static inline void
lvkw_wl_shell_surface_move(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, struct wl_seat *seat, uint32_t serial)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_MOVE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, seat, serial);
}

static inline void
lvkw_wl_shell_surface_resize(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, struct wl_seat *seat, uint32_t serial, uint32_t edges)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_RESIZE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, seat, serial, edges);
}

static inline void
lvkw_wl_shell_surface_set_toplevel(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_TOPLEVEL, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0);
}

static inline void
lvkw_wl_shell_surface_set_transient(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, struct wl_surface *parent, int32_t x, int32_t y, uint32_t flags)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_TRANSIENT, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, parent, x, y, flags);
}

static inline void
lvkw_wl_shell_surface_set_fullscreen(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, uint32_t method, uint32_t framerate, struct wl_output *output)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_FULLSCREEN, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, method, framerate, output);
}

static inline void
lvkw_wl_shell_surface_set_popup(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, struct wl_seat *seat, uint32_t serial, struct wl_surface *parent, int32_t x, int32_t y, uint32_t flags)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_POPUP, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, seat, serial, parent, x, y, flags);
}

static inline void
lvkw_wl_shell_surface_set_maximized(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, struct wl_output *output)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_MAXIMIZED, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, output);
}

static inline void
lvkw_wl_shell_surface_set_title(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, const char *title)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_TITLE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, title);
}

static inline void
lvkw_wl_shell_surface_set_class(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface, const char *class_)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_CLASS, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, class_);
}

static inline void
lvkw_wl_shell_surface_destroy(LVKW_Context_WL *ctx, struct wl_shell_surface *wl_shell_surface)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_shell_surface);
}

struct wl_surface;
struct wl_surface_listener;
struct wl_buffer;
struct wl_callback;
struct wl_region;
struct wl_region;
/* interface wl_surface */
static inline void
lvkw_wl_surface_set_user_data(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_surface, user_data);
}

static inline void *
lvkw_wl_surface_get_user_data(LVKW_Context_WL *ctx, struct wl_surface *wl_surface)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_surface);
}

static inline uint32_t
lvkw_wl_surface_get_version(LVKW_Context_WL *ctx, struct wl_surface *wl_surface)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface);
}

static inline int
lvkw_wl_surface_add_listener(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, const struct wl_surface_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_surface, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_surface_destroy(LVKW_Context_WL *ctx, struct wl_surface *wl_surface)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_surface_attach(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, struct wl_buffer *buffer, int32_t x, int32_t y)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_ATTACH, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, buffer, x, y);
}

static inline void
lvkw_wl_surface_damage(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_DAMAGE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, x, y, width, height);
}

static inline struct wl_callback *
lvkw_wl_surface_frame(LVKW_Context_WL *ctx, struct wl_surface *wl_surface)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_FRAME, &wl_callback_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, NULL);
	return (struct wl_callback *) id;
}

static inline void
lvkw_wl_surface_set_opaque_region(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, struct wl_region *region)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_OPAQUE_REGION, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, region);
}

static inline void
lvkw_wl_surface_set_input_region(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, struct wl_region *region)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_INPUT_REGION, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, region);
}

static inline void
lvkw_wl_surface_commit(LVKW_Context_WL *ctx, struct wl_surface *wl_surface)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_COMMIT, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0);
}

static inline void
lvkw_wl_surface_set_buffer_transform(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, int32_t transform)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_BUFFER_TRANSFORM, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, transform);
}

static inline void
lvkw_wl_surface_set_buffer_scale(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, int32_t scale)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_BUFFER_SCALE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, scale);
}

static inline void
lvkw_wl_surface_damage_buffer(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_DAMAGE_BUFFER, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, x, y, width, height);
}

static inline void
lvkw_wl_surface_offset(LVKW_Context_WL *ctx, struct wl_surface *wl_surface, int32_t x, int32_t y)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_OFFSET, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_surface), 0, x, y);
}

struct wl_seat;
struct wl_seat_listener;
struct wl_pointer;
struct wl_keyboard;
struct wl_touch;
/* interface wl_seat */
static inline void
lvkw_wl_seat_set_user_data(LVKW_Context_WL *ctx, struct wl_seat *wl_seat, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_seat, user_data);
}

static inline void *
lvkw_wl_seat_get_user_data(LVKW_Context_WL *ctx, struct wl_seat *wl_seat)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_seat);
}

static inline uint32_t
lvkw_wl_seat_get_version(LVKW_Context_WL *ctx, struct wl_seat *wl_seat)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_seat);
}

static inline int
lvkw_wl_seat_add_listener(LVKW_Context_WL *ctx, struct wl_seat *wl_seat, const struct wl_seat_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_seat, (void (**)(void)) listener, data);
}

static inline struct wl_pointer *
lvkw_wl_seat_get_pointer(LVKW_Context_WL *ctx, struct wl_seat *wl_seat)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_GET_POINTER, &wl_pointer_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_seat), 0, NULL);
	return (struct wl_pointer *) id;
}

static inline struct wl_keyboard *
lvkw_wl_seat_get_keyboard(LVKW_Context_WL *ctx, struct wl_seat *wl_seat)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_GET_KEYBOARD, &wl_keyboard_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_seat), 0, NULL);
	return (struct wl_keyboard *) id;
}

static inline struct wl_touch *
lvkw_wl_seat_get_touch(LVKW_Context_WL *ctx, struct wl_seat *wl_seat)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_GET_TOUCH, &wl_touch_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_seat), 0, NULL);
	return (struct wl_touch *) id;
}

static inline void
lvkw_wl_seat_release(LVKW_Context_WL *ctx, struct wl_seat *wl_seat)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_RELEASE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_seat), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_seat_destroy(LVKW_Context_WL *ctx, struct wl_seat *wl_seat)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_seat);
}

struct wl_pointer;
struct wl_pointer_listener;
struct wl_surface;
/* interface wl_pointer */
static inline void
lvkw_wl_pointer_set_user_data(LVKW_Context_WL *ctx, struct wl_pointer *wl_pointer, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_pointer, user_data);
}

static inline void *
lvkw_wl_pointer_get_user_data(LVKW_Context_WL *ctx, struct wl_pointer *wl_pointer)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_pointer);
}

static inline uint32_t
lvkw_wl_pointer_get_version(LVKW_Context_WL *ctx, struct wl_pointer *wl_pointer)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_pointer);
}

static inline int
lvkw_wl_pointer_add_listener(LVKW_Context_WL *ctx, struct wl_pointer *wl_pointer, const struct wl_pointer_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_pointer, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_pointer_set_cursor(LVKW_Context_WL *ctx, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, int32_t hotspot_x, int32_t hotspot_y)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_pointer, WL_POINTER_SET_CURSOR, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_pointer), 0, serial, surface, hotspot_x, hotspot_y);
}

static inline void
lvkw_wl_pointer_release(LVKW_Context_WL *ctx, struct wl_pointer *wl_pointer)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_pointer, WL_POINTER_RELEASE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_pointer), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_pointer_destroy(LVKW_Context_WL *ctx, struct wl_pointer *wl_pointer)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_pointer);
}

struct wl_keyboard;
struct wl_keyboard_listener;
/* interface wl_keyboard */
static inline void
lvkw_wl_keyboard_set_user_data(LVKW_Context_WL *ctx, struct wl_keyboard *wl_keyboard, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_keyboard, user_data);
}

static inline void *
lvkw_wl_keyboard_get_user_data(LVKW_Context_WL *ctx, struct wl_keyboard *wl_keyboard)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_keyboard);
}

static inline uint32_t
lvkw_wl_keyboard_get_version(LVKW_Context_WL *ctx, struct wl_keyboard *wl_keyboard)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_keyboard);
}

static inline int
lvkw_wl_keyboard_add_listener(LVKW_Context_WL *ctx, struct wl_keyboard *wl_keyboard, const struct wl_keyboard_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_keyboard, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_keyboard_release(LVKW_Context_WL *ctx, struct wl_keyboard *wl_keyboard)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_keyboard, WL_KEYBOARD_RELEASE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_keyboard), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_keyboard_destroy(LVKW_Context_WL *ctx, struct wl_keyboard *wl_keyboard)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_keyboard);
}

struct wl_touch;
struct wl_touch_listener;
/* interface wl_touch */
static inline void
lvkw_wl_touch_set_user_data(LVKW_Context_WL *ctx, struct wl_touch *wl_touch, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_touch, user_data);
}

static inline void *
lvkw_wl_touch_get_user_data(LVKW_Context_WL *ctx, struct wl_touch *wl_touch)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_touch);
}

static inline uint32_t
lvkw_wl_touch_get_version(LVKW_Context_WL *ctx, struct wl_touch *wl_touch)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_touch);
}

static inline int
lvkw_wl_touch_add_listener(LVKW_Context_WL *ctx, struct wl_touch *wl_touch, const struct wl_touch_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_touch, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_touch_release(LVKW_Context_WL *ctx, struct wl_touch *wl_touch)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_touch, WL_TOUCH_RELEASE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_touch), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_touch_destroy(LVKW_Context_WL *ctx, struct wl_touch *wl_touch)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_touch);
}

struct wl_output;
struct wl_output_listener;
/* interface wl_output */
static inline void
lvkw_wl_output_set_user_data(LVKW_Context_WL *ctx, struct wl_output *wl_output, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_output, user_data);
}

static inline void *
lvkw_wl_output_get_user_data(LVKW_Context_WL *ctx, struct wl_output *wl_output)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_output);
}

static inline uint32_t
lvkw_wl_output_get_version(LVKW_Context_WL *ctx, struct wl_output *wl_output)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_output);
}

static inline int
lvkw_wl_output_add_listener(LVKW_Context_WL *ctx, struct wl_output *wl_output, const struct wl_output_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_output, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_output_release(LVKW_Context_WL *ctx, struct wl_output *wl_output)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_output, WL_OUTPUT_RELEASE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_output), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_output_destroy(LVKW_Context_WL *ctx, struct wl_output *wl_output)
{
	ctx->dlib.wl.proxy_destroy((struct wl_proxy *) wl_output);
}

struct wl_region;
struct wl_region_listener;
/* interface wl_region */
static inline void
lvkw_wl_region_set_user_data(LVKW_Context_WL *ctx, struct wl_region *wl_region, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_region, user_data);
}

static inline void *
lvkw_wl_region_get_user_data(LVKW_Context_WL *ctx, struct wl_region *wl_region)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_region);
}

static inline uint32_t
lvkw_wl_region_get_version(LVKW_Context_WL *ctx, struct wl_region *wl_region)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_region);
}

static inline int
lvkw_wl_region_add_listener(LVKW_Context_WL *ctx, struct wl_region *wl_region, const struct wl_region_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_region, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_region_destroy(LVKW_Context_WL *ctx, struct wl_region *wl_region)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_region, WL_REGION_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_region), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_region_add(LVKW_Context_WL *ctx, struct wl_region *wl_region, int32_t x, int32_t y, int32_t width, int32_t height)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_region, WL_REGION_ADD, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_region), 0, x, y, width, height);
}

static inline void
lvkw_wl_region_subtract(LVKW_Context_WL *ctx, struct wl_region *wl_region, int32_t x, int32_t y, int32_t width, int32_t height)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_region, WL_REGION_SUBTRACT, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_region), 0, x, y, width, height);
}

struct wl_subcompositor;
struct wl_subcompositor_listener;
struct wl_subsurface;
struct wl_surface;
struct wl_surface;
/* interface wl_subcompositor */
static inline void
lvkw_wl_subcompositor_set_user_data(LVKW_Context_WL *ctx, struct wl_subcompositor *wl_subcompositor, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_subcompositor, user_data);
}

static inline void *
lvkw_wl_subcompositor_get_user_data(LVKW_Context_WL *ctx, struct wl_subcompositor *wl_subcompositor)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_subcompositor);
}

static inline uint32_t
lvkw_wl_subcompositor_get_version(LVKW_Context_WL *ctx, struct wl_subcompositor *wl_subcompositor)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subcompositor);
}

static inline int
lvkw_wl_subcompositor_add_listener(LVKW_Context_WL *ctx, struct wl_subcompositor *wl_subcompositor, const struct wl_subcompositor_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_subcompositor, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_subcompositor_destroy(LVKW_Context_WL *ctx, struct wl_subcompositor *wl_subcompositor)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subcompositor, WL_SUBCOMPOSITOR_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subcompositor), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct wl_subsurface *
lvkw_wl_subcompositor_get_subsurface(LVKW_Context_WL *ctx, struct wl_subcompositor *wl_subcompositor, struct wl_surface *surface, struct wl_surface *parent)
{
	struct wl_proxy *id;
	id = ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subcompositor, WL_SUBCOMPOSITOR_GET_SUBSURFACE, &wl_subsurface_interface, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subcompositor), 0, NULL, surface, parent);
	return (struct wl_subsurface *) id;
}

struct wl_subsurface;
struct wl_subsurface_listener;
struct wl_surface;
struct wl_surface;
/* interface wl_subsurface */
static inline void
lvkw_wl_subsurface_set_user_data(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface, void *user_data)
{
	ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) wl_subsurface, user_data);
}

static inline void *
lvkw_wl_subsurface_get_user_data(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface)
{
	return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) wl_subsurface);
}

static inline uint32_t
lvkw_wl_subsurface_get_version(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface)
{
	return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subsurface);
}

static inline int
lvkw_wl_subsurface_add_listener(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface, const struct wl_subsurface_listener *listener, void *data)
{
	return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) wl_subsurface, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_subsurface_destroy(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_DESTROY, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subsurface), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_subsurface_set_position(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface, int32_t x, int32_t y)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_SET_POSITION, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subsurface), 0, x, y);
}

static inline void
lvkw_wl_subsurface_place_above(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface, struct wl_surface *sibling)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_PLACE_ABOVE, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subsurface), 0, sibling);
}

static inline void
lvkw_wl_subsurface_place_below(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface, struct wl_surface *sibling)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_PLACE_BELOW, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subsurface), 0, sibling);
}

static inline void
lvkw_wl_subsurface_set_sync(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_SET_SYNC, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subsurface), 0);
}

static inline void
lvkw_wl_subsurface_set_desync(LVKW_Context_WL *ctx, struct wl_subsurface *wl_subsurface)
{
	ctx->dlib.wl.proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_SET_DESYNC, NULL, ctx->dlib.wl.proxy_get_version((struct wl_proxy *) wl_subsurface), 0);
}

#endif
