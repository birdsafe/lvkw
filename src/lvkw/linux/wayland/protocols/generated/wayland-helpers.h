/* Generated from wayland.xml */

#ifndef LVKW_WAYLAND_HELPERS_WAYLAND_H
#define LVKW_WAYLAND_HELPERS_WAYLAND_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct wl_display;
struct wl_display_listener;
struct wl_callback;
struct wl_registry;
/* interface wl_display */
static inline void
lvkw_wl_display_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_display *wl_display, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_display, user_data);
}

static inline void *
lvkw_wl_display_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_display *wl_display)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_display);
}

static inline uint32_t
lvkw_wl_display_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_display *wl_display)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_display);
}

static inline int
lvkw_wl_display_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_display *wl_display, const struct wl_display_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_display, (void (**)(void)) listener, data);
}

static inline struct wl_callback *
lvkw_wl_display_sync(const LVKW_Lib_WaylandClient *lib, struct wl_display *wl_display)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_display, WL_DISPLAY_SYNC, &wl_callback_interface, lib->proxy_get_version((struct wl_proxy *) wl_display), 0, NULL);
	return (struct wl_callback *) id;
}

static inline struct wl_registry *
lvkw_wl_display_get_registry(const LVKW_Lib_WaylandClient *lib, struct wl_display *wl_display)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_display, WL_DISPLAY_GET_REGISTRY, &wl_registry_interface, lib->proxy_get_version((struct wl_proxy *) wl_display), 0, NULL);
	return (struct wl_registry *) id;
}

static inline void
lvkw_wl_display_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_display *wl_display)
{
	lib->proxy_destroy((struct wl_proxy *) wl_display);
}

struct wl_registry;
struct wl_registry_listener;
/* interface wl_registry */
static inline void
lvkw_wl_registry_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_registry *wl_registry, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_registry, user_data);
}

static inline void *
lvkw_wl_registry_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_registry *wl_registry)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_registry);
}

static inline uint32_t
lvkw_wl_registry_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_registry *wl_registry)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_registry);
}

static inline int
lvkw_wl_registry_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_registry *wl_registry, const struct wl_registry_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_registry, (void (**)(void)) listener, data);
}

static inline void *
lvkw_wl_registry_bind(const LVKW_Lib_WaylandClient *lib, struct wl_registry *wl_registry, uint32_t name, const struct wl_interface *interface, uint32_t version)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_registry, WL_REGISTRY_BIND, interface, lib->proxy_get_version((struct wl_proxy *) wl_registry), 0, name, interface->name, version, NULL);
	return (void *) id;
}

static inline void
lvkw_wl_registry_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_registry *wl_registry)
{
	lib->proxy_destroy((struct wl_proxy *) wl_registry);
}

struct wl_callback;
struct wl_callback_listener;
/* interface wl_callback */
static inline void
lvkw_wl_callback_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_callback *wl_callback, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_callback, user_data);
}

static inline void *
lvkw_wl_callback_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_callback *wl_callback)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_callback);
}

static inline uint32_t
lvkw_wl_callback_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_callback *wl_callback)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_callback);
}

static inline int
lvkw_wl_callback_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_callback *wl_callback, const struct wl_callback_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_callback, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_callback_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_callback *wl_callback)
{
	lib->proxy_destroy((struct wl_proxy *) wl_callback);
}

struct wl_compositor;
struct wl_compositor_listener;
struct wl_surface;
struct wl_region;
/* interface wl_compositor */
static inline void
lvkw_wl_compositor_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_compositor *wl_compositor, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_compositor, user_data);
}

static inline void *
lvkw_wl_compositor_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_compositor *wl_compositor)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_compositor);
}

static inline uint32_t
lvkw_wl_compositor_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_compositor *wl_compositor)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_compositor);
}

static inline int
lvkw_wl_compositor_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_compositor *wl_compositor, const struct wl_compositor_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_compositor, (void (**)(void)) listener, data);
}

static inline struct wl_surface *
lvkw_wl_compositor_create_surface(const LVKW_Lib_WaylandClient *lib, struct wl_compositor *wl_compositor)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_compositor, WL_COMPOSITOR_CREATE_SURFACE, &wl_surface_interface, lib->proxy_get_version((struct wl_proxy *) wl_compositor), 0, NULL);
	return (struct wl_surface *) id;
}

static inline struct wl_region *
lvkw_wl_compositor_create_region(const LVKW_Lib_WaylandClient *lib, struct wl_compositor *wl_compositor)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_compositor, WL_COMPOSITOR_CREATE_REGION, &wl_region_interface, lib->proxy_get_version((struct wl_proxy *) wl_compositor), 0, NULL);
	return (struct wl_region *) id;
}

static inline void
lvkw_wl_compositor_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_compositor *wl_compositor)
{
	lib->proxy_destroy((struct wl_proxy *) wl_compositor);
}

struct wl_shm_pool;
struct wl_shm_pool_listener;
struct wl_buffer;
/* interface wl_shm_pool */
static inline void
lvkw_wl_shm_pool_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shm_pool *wl_shm_pool, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_shm_pool, user_data);
}

static inline void *
lvkw_wl_shm_pool_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shm_pool *wl_shm_pool)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_shm_pool);
}

static inline uint32_t
lvkw_wl_shm_pool_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_shm_pool *wl_shm_pool)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_shm_pool);
}

static inline int
lvkw_wl_shm_pool_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_shm_pool *wl_shm_pool, const struct wl_shm_pool_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_shm_pool, (void (**)(void)) listener, data);
}

static inline struct wl_buffer *
lvkw_wl_shm_pool_create_buffer(const LVKW_Lib_WaylandClient *lib, struct wl_shm_pool *wl_shm_pool, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_shm_pool, WL_SHM_POOL_CREATE_BUFFER, &wl_buffer_interface, lib->proxy_get_version((struct wl_proxy *) wl_shm_pool), 0, NULL, offset, width, height, stride, format);
	return (struct wl_buffer *) id;
}

static inline void
lvkw_wl_shm_pool_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_shm_pool *wl_shm_pool)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shm_pool, WL_SHM_POOL_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shm_pool), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_shm_pool_resize(const LVKW_Lib_WaylandClient *lib, struct wl_shm_pool *wl_shm_pool, int32_t size)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shm_pool, WL_SHM_POOL_RESIZE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shm_pool), 0, size);
}

struct wl_shm;
struct wl_shm_listener;
struct wl_shm_pool;
/* interface wl_shm */
static inline void
lvkw_wl_shm_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shm *wl_shm, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_shm, user_data);
}

static inline void *
lvkw_wl_shm_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shm *wl_shm)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_shm);
}

static inline uint32_t
lvkw_wl_shm_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_shm *wl_shm)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_shm);
}

static inline int
lvkw_wl_shm_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_shm *wl_shm, const struct wl_shm_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_shm, (void (**)(void)) listener, data);
}

static inline struct wl_shm_pool *
lvkw_wl_shm_create_pool(const LVKW_Lib_WaylandClient *lib, struct wl_shm *wl_shm, int32_t fd, int32_t size)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_shm, WL_SHM_CREATE_POOL, &wl_shm_pool_interface, lib->proxy_get_version((struct wl_proxy *) wl_shm), 0, NULL, fd, size);
	return (struct wl_shm_pool *) id;
}

static inline void
lvkw_wl_shm_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_shm *wl_shm)
{
	lib->proxy_destroy((struct wl_proxy *) wl_shm);
}

struct wl_buffer;
struct wl_buffer_listener;
/* interface wl_buffer */
static inline void
lvkw_wl_buffer_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_buffer *wl_buffer, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_buffer, user_data);
}

static inline void *
lvkw_wl_buffer_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_buffer *wl_buffer)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_buffer);
}

static inline uint32_t
lvkw_wl_buffer_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_buffer *wl_buffer)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_buffer);
}

static inline int
lvkw_wl_buffer_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_buffer *wl_buffer, const struct wl_buffer_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_buffer, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_buffer_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_buffer *wl_buffer)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_buffer, WL_BUFFER_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_buffer), WL_MARSHAL_FLAG_DESTROY);
}

struct wl_data_offer;
struct wl_data_offer_listener;
/* interface wl_data_offer */
static inline void
lvkw_wl_data_offer_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_data_offer, user_data);
}

static inline void *
lvkw_wl_data_offer_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_data_offer);
}

static inline uint32_t
lvkw_wl_data_offer_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_data_offer);
}

static inline int
lvkw_wl_data_offer_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer, const struct wl_data_offer_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_data_offer, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_data_offer_accept(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer, uint32_t serial, const char *mime_type)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_ACCEPT, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_offer), 0, serial, mime_type);
}

static inline void
lvkw_wl_data_offer_receive(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer, const char *mime_type, int32_t fd)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_RECEIVE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_offer), 0, mime_type, fd);
}

static inline void
lvkw_wl_data_offer_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_offer), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_data_offer_finish(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_FINISH, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_offer), 0);
}

static inline void
lvkw_wl_data_offer_set_actions(const LVKW_Lib_WaylandClient *lib, struct wl_data_offer *wl_data_offer, uint32_t dnd_actions, uint32_t preferred_action)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_offer, WL_DATA_OFFER_SET_ACTIONS, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_offer), 0, dnd_actions, preferred_action);
}

struct wl_data_source;
struct wl_data_source_listener;
/* interface wl_data_source */
static inline void
lvkw_wl_data_source_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_source *wl_data_source, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_data_source, user_data);
}

static inline void *
lvkw_wl_data_source_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_source *wl_data_source)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_data_source);
}

static inline uint32_t
lvkw_wl_data_source_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_data_source *wl_data_source)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_data_source);
}

static inline int
lvkw_wl_data_source_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_data_source *wl_data_source, const struct wl_data_source_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_data_source, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_data_source_offer(const LVKW_Lib_WaylandClient *lib, struct wl_data_source *wl_data_source, const char *mime_type)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_source, WL_DATA_SOURCE_OFFER, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_source), 0, mime_type);
}

static inline void
lvkw_wl_data_source_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_data_source *wl_data_source)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_source, WL_DATA_SOURCE_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_source), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_data_source_set_actions(const LVKW_Lib_WaylandClient *lib, struct wl_data_source *wl_data_source, uint32_t dnd_actions)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_source, WL_DATA_SOURCE_SET_ACTIONS, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_source), 0, dnd_actions);
}

struct wl_data_device;
struct wl_data_device_listener;
struct wl_data_source;
struct wl_surface;
struct wl_surface;
struct wl_data_source;
/* interface wl_data_device */
static inline void
lvkw_wl_data_device_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_data_device, user_data);
}

static inline void *
lvkw_wl_data_device_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_data_device);
}

static inline uint32_t
lvkw_wl_data_device_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_data_device);
}

static inline int
lvkw_wl_data_device_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device, const struct wl_data_device_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_data_device, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_data_device_start_drag(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device, struct wl_data_source *source, struct wl_surface *origin, struct wl_surface *icon, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_device, WL_DATA_DEVICE_START_DRAG, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_device), 0, source, origin, icon, serial);
}

static inline void
lvkw_wl_data_device_set_selection(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device, struct wl_data_source *source, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_device, WL_DATA_DEVICE_SET_SELECTION, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_device), 0, source, serial);
}

static inline void
lvkw_wl_data_device_release(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_data_device, WL_DATA_DEVICE_RELEASE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_data_device), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_data_device_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_data_device *wl_data_device)
{
	lib->proxy_destroy((struct wl_proxy *) wl_data_device);
}

struct wl_data_device_manager;
struct wl_data_device_manager_listener;
struct wl_data_source;
struct wl_data_device;
struct wl_seat;
/* interface wl_data_device_manager */
static inline void
lvkw_wl_data_device_manager_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_device_manager *wl_data_device_manager, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_data_device_manager, user_data);
}

static inline void *
lvkw_wl_data_device_manager_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_data_device_manager *wl_data_device_manager)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_data_device_manager);
}

static inline uint32_t
lvkw_wl_data_device_manager_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_data_device_manager *wl_data_device_manager)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_data_device_manager);
}

static inline int
lvkw_wl_data_device_manager_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_data_device_manager *wl_data_device_manager, const struct wl_data_device_manager_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_data_device_manager, (void (**)(void)) listener, data);
}

static inline struct wl_data_source *
lvkw_wl_data_device_manager_create_data_source(const LVKW_Lib_WaylandClient *lib, struct wl_data_device_manager *wl_data_device_manager)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_data_device_manager, WL_DATA_DEVICE_MANAGER_CREATE_DATA_SOURCE, &wl_data_source_interface, lib->proxy_get_version((struct wl_proxy *) wl_data_device_manager), 0, NULL);
	return (struct wl_data_source *) id;
}

static inline struct wl_data_device *
lvkw_wl_data_device_manager_get_data_device(const LVKW_Lib_WaylandClient *lib, struct wl_data_device_manager *wl_data_device_manager, struct wl_seat *seat)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_data_device_manager, WL_DATA_DEVICE_MANAGER_GET_DATA_DEVICE, &wl_data_device_interface, lib->proxy_get_version((struct wl_proxy *) wl_data_device_manager), 0, NULL, seat);
	return (struct wl_data_device *) id;
}

static inline void
lvkw_wl_data_device_manager_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_data_device_manager *wl_data_device_manager)
{
	lib->proxy_destroy((struct wl_proxy *) wl_data_device_manager);
}

struct wl_shell;
struct wl_shell_listener;
struct wl_shell_surface;
struct wl_surface;
/* interface wl_shell */
static inline void
lvkw_wl_shell_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shell *wl_shell, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_shell, user_data);
}

static inline void *
lvkw_wl_shell_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shell *wl_shell)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_shell);
}

static inline uint32_t
lvkw_wl_shell_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_shell *wl_shell)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_shell);
}

static inline int
lvkw_wl_shell_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_shell *wl_shell, const struct wl_shell_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_shell, (void (**)(void)) listener, data);
}

static inline struct wl_shell_surface *
lvkw_wl_shell_get_shell_surface(const LVKW_Lib_WaylandClient *lib, struct wl_shell *wl_shell, struct wl_surface *surface)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_shell, WL_SHELL_GET_SHELL_SURFACE, &wl_shell_surface_interface, lib->proxy_get_version((struct wl_proxy *) wl_shell), 0, NULL, surface);
	return (struct wl_shell_surface *) id;
}

static inline void
lvkw_wl_shell_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_shell *wl_shell)
{
	lib->proxy_destroy((struct wl_proxy *) wl_shell);
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
lvkw_wl_shell_surface_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_shell_surface, user_data);
}

static inline void *
lvkw_wl_shell_surface_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_shell_surface);
}

static inline uint32_t
lvkw_wl_shell_surface_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_shell_surface);
}

static inline int
lvkw_wl_shell_surface_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, const struct wl_shell_surface_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_shell_surface, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_shell_surface_pong(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_PONG, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, serial);
}

static inline void
lvkw_wl_shell_surface_move(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, struct wl_seat *seat, uint32_t serial)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_MOVE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, seat, serial);
}

static inline void
lvkw_wl_shell_surface_resize(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, struct wl_seat *seat, uint32_t serial, uint32_t edges)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_RESIZE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, seat, serial, edges);
}

static inline void
lvkw_wl_shell_surface_set_toplevel(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_TOPLEVEL, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0);
}

static inline void
lvkw_wl_shell_surface_set_transient(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, struct wl_surface *parent, int32_t x, int32_t y, uint32_t flags)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_TRANSIENT, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, parent, x, y, flags);
}

static inline void
lvkw_wl_shell_surface_set_fullscreen(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, uint32_t method, uint32_t framerate, struct wl_output *output)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_FULLSCREEN, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, method, framerate, output);
}

static inline void
lvkw_wl_shell_surface_set_popup(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, struct wl_seat *seat, uint32_t serial, struct wl_surface *parent, int32_t x, int32_t y, uint32_t flags)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_POPUP, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, seat, serial, parent, x, y, flags);
}

static inline void
lvkw_wl_shell_surface_set_maximized(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, struct wl_output *output)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_MAXIMIZED, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, output);
}

static inline void
lvkw_wl_shell_surface_set_title(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, const char *title)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_TITLE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, title);
}

static inline void
lvkw_wl_shell_surface_set_class(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface, const char *class_)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_shell_surface, WL_SHELL_SURFACE_SET_CLASS, NULL, lib->proxy_get_version((struct wl_proxy *) wl_shell_surface), 0, class_);
}

static inline void
lvkw_wl_shell_surface_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_shell_surface *wl_shell_surface)
{
	lib->proxy_destroy((struct wl_proxy *) wl_shell_surface);
}

struct wl_surface;
struct wl_surface_listener;
struct wl_buffer;
struct wl_callback;
struct wl_region;
struct wl_region;
/* interface wl_surface */
static inline void
lvkw_wl_surface_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_surface, user_data);
}

static inline void *
lvkw_wl_surface_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_surface);
}

static inline uint32_t
lvkw_wl_surface_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_surface);
}

static inline int
lvkw_wl_surface_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, const struct wl_surface_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_surface, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_surface_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_surface_attach(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, struct wl_buffer *buffer, int32_t x, int32_t y)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_ATTACH, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, buffer, x, y);
}

static inline void
lvkw_wl_surface_damage(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_DAMAGE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, x, y, width, height);
}

static inline struct wl_callback *
lvkw_wl_surface_frame(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_FRAME, &wl_callback_interface, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, NULL);
	return (struct wl_callback *) id;
}

static inline void
lvkw_wl_surface_set_opaque_region(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, struct wl_region *region)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_OPAQUE_REGION, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, region);
}

static inline void
lvkw_wl_surface_set_input_region(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, struct wl_region *region)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_INPUT_REGION, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, region);
}

static inline void
lvkw_wl_surface_commit(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_COMMIT, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0);
}

static inline void
lvkw_wl_surface_set_buffer_transform(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, int32_t transform)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_BUFFER_TRANSFORM, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, transform);
}

static inline void
lvkw_wl_surface_set_buffer_scale(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, int32_t scale)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_SET_BUFFER_SCALE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, scale);
}

static inline void
lvkw_wl_surface_damage_buffer(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_DAMAGE_BUFFER, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, x, y, width, height);
}

static inline void
lvkw_wl_surface_offset(const LVKW_Lib_WaylandClient *lib, struct wl_surface *wl_surface, int32_t x, int32_t y)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_surface, WL_SURFACE_OFFSET, NULL, lib->proxy_get_version((struct wl_proxy *) wl_surface), 0, x, y);
}

struct wl_seat;
struct wl_seat_listener;
struct wl_pointer;
struct wl_keyboard;
struct wl_touch;
/* interface wl_seat */
static inline void
lvkw_wl_seat_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_seat, user_data);
}

static inline void *
lvkw_wl_seat_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_seat);
}

static inline uint32_t
lvkw_wl_seat_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_seat);
}

static inline int
lvkw_wl_seat_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat, const struct wl_seat_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_seat, (void (**)(void)) listener, data);
}

static inline struct wl_pointer *
lvkw_wl_seat_get_pointer(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_GET_POINTER, &wl_pointer_interface, lib->proxy_get_version((struct wl_proxy *) wl_seat), 0, NULL);
	return (struct wl_pointer *) id;
}

static inline struct wl_keyboard *
lvkw_wl_seat_get_keyboard(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_GET_KEYBOARD, &wl_keyboard_interface, lib->proxy_get_version((struct wl_proxy *) wl_seat), 0, NULL);
	return (struct wl_keyboard *) id;
}

static inline struct wl_touch *
lvkw_wl_seat_get_touch(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_GET_TOUCH, &wl_touch_interface, lib->proxy_get_version((struct wl_proxy *) wl_seat), 0, NULL);
	return (struct wl_touch *) id;
}

static inline void
lvkw_wl_seat_release(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_seat, WL_SEAT_RELEASE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_seat), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_seat_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_seat *wl_seat)
{
	lib->proxy_destroy((struct wl_proxy *) wl_seat);
}

struct wl_pointer;
struct wl_pointer_listener;
struct wl_surface;
/* interface wl_pointer */
static inline void
lvkw_wl_pointer_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_pointer *wl_pointer, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_pointer, user_data);
}

static inline void *
lvkw_wl_pointer_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_pointer *wl_pointer)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_pointer);
}

static inline uint32_t
lvkw_wl_pointer_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_pointer *wl_pointer)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_pointer);
}

static inline int
lvkw_wl_pointer_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_pointer *wl_pointer, const struct wl_pointer_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_pointer, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_pointer_set_cursor(const LVKW_Lib_WaylandClient *lib, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, int32_t hotspot_x, int32_t hotspot_y)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_pointer, WL_POINTER_SET_CURSOR, NULL, lib->proxy_get_version((struct wl_proxy *) wl_pointer), 0, serial, surface, hotspot_x, hotspot_y);
}

static inline void
lvkw_wl_pointer_release(const LVKW_Lib_WaylandClient *lib, struct wl_pointer *wl_pointer)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_pointer, WL_POINTER_RELEASE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_pointer), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_pointer_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_pointer *wl_pointer)
{
	lib->proxy_destroy((struct wl_proxy *) wl_pointer);
}

struct wl_keyboard;
struct wl_keyboard_listener;
/* interface wl_keyboard */
static inline void
lvkw_wl_keyboard_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_keyboard *wl_keyboard, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_keyboard, user_data);
}

static inline void *
lvkw_wl_keyboard_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_keyboard *wl_keyboard)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_keyboard);
}

static inline uint32_t
lvkw_wl_keyboard_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_keyboard *wl_keyboard)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_keyboard);
}

static inline int
lvkw_wl_keyboard_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_keyboard *wl_keyboard, const struct wl_keyboard_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_keyboard, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_keyboard_release(const LVKW_Lib_WaylandClient *lib, struct wl_keyboard *wl_keyboard)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_keyboard, WL_KEYBOARD_RELEASE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_keyboard), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_keyboard_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_keyboard *wl_keyboard)
{
	lib->proxy_destroy((struct wl_proxy *) wl_keyboard);
}

struct wl_touch;
struct wl_touch_listener;
/* interface wl_touch */
static inline void
lvkw_wl_touch_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_touch *wl_touch, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_touch, user_data);
}

static inline void *
lvkw_wl_touch_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_touch *wl_touch)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_touch);
}

static inline uint32_t
lvkw_wl_touch_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_touch *wl_touch)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_touch);
}

static inline int
lvkw_wl_touch_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_touch *wl_touch, const struct wl_touch_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_touch, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_touch_release(const LVKW_Lib_WaylandClient *lib, struct wl_touch *wl_touch)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_touch, WL_TOUCH_RELEASE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_touch), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_touch_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_touch *wl_touch)
{
	lib->proxy_destroy((struct wl_proxy *) wl_touch);
}

struct wl_output;
struct wl_output_listener;
/* interface wl_output */
static inline void
lvkw_wl_output_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_output *wl_output, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_output, user_data);
}

static inline void *
lvkw_wl_output_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_output *wl_output)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_output);
}

static inline uint32_t
lvkw_wl_output_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_output *wl_output)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_output);
}

static inline int
lvkw_wl_output_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_output *wl_output, const struct wl_output_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_output, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_output_release(const LVKW_Lib_WaylandClient *lib, struct wl_output *wl_output)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_output, WL_OUTPUT_RELEASE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_output), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_output_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_output *wl_output)
{
	lib->proxy_destroy((struct wl_proxy *) wl_output);
}

struct wl_region;
struct wl_region_listener;
/* interface wl_region */
static inline void
lvkw_wl_region_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_region *wl_region, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_region, user_data);
}

static inline void *
lvkw_wl_region_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_region *wl_region)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_region);
}

static inline uint32_t
lvkw_wl_region_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_region *wl_region)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_region);
}

static inline int
lvkw_wl_region_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_region *wl_region, const struct wl_region_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_region, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_region_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_region *wl_region)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_region, WL_REGION_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_region), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_region_add(const LVKW_Lib_WaylandClient *lib, struct wl_region *wl_region, int32_t x, int32_t y, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_region, WL_REGION_ADD, NULL, lib->proxy_get_version((struct wl_proxy *) wl_region), 0, x, y, width, height);
}

static inline void
lvkw_wl_region_subtract(const LVKW_Lib_WaylandClient *lib, struct wl_region *wl_region, int32_t x, int32_t y, int32_t width, int32_t height)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_region, WL_REGION_SUBTRACT, NULL, lib->proxy_get_version((struct wl_proxy *) wl_region), 0, x, y, width, height);
}

struct wl_subcompositor;
struct wl_subcompositor_listener;
struct wl_subsurface;
struct wl_surface;
struct wl_surface;
/* interface wl_subcompositor */
static inline void
lvkw_wl_subcompositor_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_subcompositor *wl_subcompositor, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_subcompositor, user_data);
}

static inline void *
lvkw_wl_subcompositor_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_subcompositor *wl_subcompositor)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_subcompositor);
}

static inline uint32_t
lvkw_wl_subcompositor_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_subcompositor *wl_subcompositor)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_subcompositor);
}

static inline int
lvkw_wl_subcompositor_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_subcompositor *wl_subcompositor, const struct wl_subcompositor_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_subcompositor, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_subcompositor_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_subcompositor *wl_subcompositor)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_subcompositor, WL_SUBCOMPOSITOR_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_subcompositor), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct wl_subsurface *
lvkw_wl_subcompositor_get_subsurface(const LVKW_Lib_WaylandClient *lib, struct wl_subcompositor *wl_subcompositor, struct wl_surface *surface, struct wl_surface *parent)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) wl_subcompositor, WL_SUBCOMPOSITOR_GET_SUBSURFACE, &wl_subsurface_interface, lib->proxy_get_version((struct wl_proxy *) wl_subcompositor), 0, NULL, surface, parent);
	return (struct wl_subsurface *) id;
}

struct wl_subsurface;
struct wl_subsurface_listener;
struct wl_surface;
struct wl_surface;
/* interface wl_subsurface */
static inline void
lvkw_wl_subsurface_set_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) wl_subsurface, user_data);
}

static inline void *
lvkw_wl_subsurface_get_user_data(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface)
{
	return lib->proxy_get_user_data((struct wl_proxy *) wl_subsurface);
}

static inline uint32_t
lvkw_wl_subsurface_get_version(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface)
{
	return lib->proxy_get_version((struct wl_proxy *) wl_subsurface);
}

static inline int
lvkw_wl_subsurface_add_listener(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface, const struct wl_subsurface_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) wl_subsurface, (void (**)(void)) listener, data);
}

static inline void
lvkw_wl_subsurface_destroy(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) wl_subsurface), WL_MARSHAL_FLAG_DESTROY);
}

static inline void
lvkw_wl_subsurface_set_position(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface, int32_t x, int32_t y)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_SET_POSITION, NULL, lib->proxy_get_version((struct wl_proxy *) wl_subsurface), 0, x, y);
}

static inline void
lvkw_wl_subsurface_place_above(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface, struct wl_surface *sibling)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_PLACE_ABOVE, NULL, lib->proxy_get_version((struct wl_proxy *) wl_subsurface), 0, sibling);
}

static inline void
lvkw_wl_subsurface_place_below(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface, struct wl_surface *sibling)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_PLACE_BELOW, NULL, lib->proxy_get_version((struct wl_proxy *) wl_subsurface), 0, sibling);
}

static inline void
lvkw_wl_subsurface_set_sync(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_SET_SYNC, NULL, lib->proxy_get_version((struct wl_proxy *) wl_subsurface), 0);
}

static inline void
lvkw_wl_subsurface_set_desync(const LVKW_Lib_WaylandClient *lib, struct wl_subsurface *wl_subsurface)
{
	lib->proxy_marshal_flags((struct wl_proxy *) wl_subsurface, WL_SUBSURFACE_SET_DESYNC, NULL, lib->proxy_get_version((struct wl_proxy *) wl_subsurface), 0);
}

#endif
