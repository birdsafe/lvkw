/* Generated from ext-idle-notify-v1.xml */

#ifndef LVKW_WAYLAND_HELPERS_EXT_IDLE_NOTIFY_V1_H
#define LVKW_WAYLAND_HELPERS_EXT_IDLE_NOTIFY_V1_H

#include <stdint.h>
#include <stddef.h>
#include "dlib/wayland-client.h"

struct ext_idle_notifier_v1;
struct ext_idle_notifier_v1_listener;
struct ext_idle_notification_v1;
struct wl_seat;
/* interface ext_idle_notifier_v1 */
static inline void
lvkw_ext_idle_notifier_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notifier_v1 *ext_idle_notifier_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) ext_idle_notifier_v1, user_data);
}

static inline void *
lvkw_ext_idle_notifier_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notifier_v1 *ext_idle_notifier_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) ext_idle_notifier_v1);
}

static inline uint32_t
lvkw_ext_idle_notifier_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notifier_v1 *ext_idle_notifier_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) ext_idle_notifier_v1);
}

static inline int
lvkw_ext_idle_notifier_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notifier_v1 *ext_idle_notifier_v1, const struct ext_idle_notifier_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) ext_idle_notifier_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_ext_idle_notifier_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notifier_v1 *ext_idle_notifier_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) ext_idle_notifier_v1, EXT_IDLE_NOTIFIER_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) ext_idle_notifier_v1), WL_MARSHAL_FLAG_DESTROY);
}

static inline struct ext_idle_notification_v1 *
lvkw_ext_idle_notifier_v1_get_idle_notification(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notifier_v1 *ext_idle_notifier_v1, uint32_t timeout, struct wl_seat *seat)
{
	struct wl_proxy *id;
	id = lib->proxy_marshal_flags((struct wl_proxy *) ext_idle_notifier_v1, EXT_IDLE_NOTIFIER_V1_GET_IDLE_NOTIFICATION, &ext_idle_notification_v1_interface, lib->proxy_get_version((struct wl_proxy *) ext_idle_notifier_v1), 0, NULL, timeout, seat);
	return (struct ext_idle_notification_v1 *) id;
}

struct ext_idle_notification_v1;
struct ext_idle_notification_v1_listener;
/* interface ext_idle_notification_v1 */
static inline void
lvkw_ext_idle_notification_v1_set_user_data(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notification_v1 *ext_idle_notification_v1, void *user_data)
{
	lib->proxy_set_user_data((struct wl_proxy *) ext_idle_notification_v1, user_data);
}

static inline void *
lvkw_ext_idle_notification_v1_get_user_data(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notification_v1 *ext_idle_notification_v1)
{
	return lib->proxy_get_user_data((struct wl_proxy *) ext_idle_notification_v1);
}

static inline uint32_t
lvkw_ext_idle_notification_v1_get_version(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notification_v1 *ext_idle_notification_v1)
{
	return lib->proxy_get_version((struct wl_proxy *) ext_idle_notification_v1);
}

static inline int
lvkw_ext_idle_notification_v1_add_listener(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notification_v1 *ext_idle_notification_v1, const struct ext_idle_notification_v1_listener *listener, void *data)
{
	return lib->proxy_add_listener((struct wl_proxy *) ext_idle_notification_v1, (void (**)(void)) listener, data);
}

static inline void
lvkw_ext_idle_notification_v1_destroy(const LVKW_Lib_WaylandClient *lib, struct ext_idle_notification_v1 *ext_idle_notification_v1)
{
	lib->proxy_marshal_flags((struct wl_proxy *) ext_idle_notification_v1, EXT_IDLE_NOTIFICATION_V1_DESTROY, NULL, lib->proxy_get_version((struct wl_proxy *) ext_idle_notification_v1), WL_MARSHAL_FLAG_DESTROY);
}

#endif
