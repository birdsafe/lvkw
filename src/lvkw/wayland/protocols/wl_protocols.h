#ifndef LVKW_WAYLAND_PROTOCOLS_H_INCLUDED
#define LVKW_WAYLAND_PROTOCOLS_H_INCLUDED

// Necessary so that the API calls get rerouted to our dynamic table
#include "dlib/wayland-client.h"  // IWYU pragma: keep

//
#include "protocols/generated/content-type-v1-client-protocol.h"
#include "protocols/generated/cursor-shape-v1-client-protocol.h"
#include "protocols/generated/ext-idle-notify-v1-client-protocol.h"
#include "protocols/generated/fractional-scale-v1-client-protocol.h"
#include "protocols/generated/idle-inhibit-unstable-v1-client-protocol.h"
#include "protocols/generated/pointer-constraints-unstable-v1-client-protocol.h"
#include "protocols/generated/relative-pointer-unstable-v1-client-protocol.h"
#include "protocols/generated/tablet-v2-client-protocol.h"
#include "protocols/generated/viewporter-client-protocol.h"
#include "protocols/generated/wayland-client-protocol.h"
#include "protocols/generated/xdg-activation-v1-client-protocol.h"
#include "protocols/generated/xdg-decoration-unstable-v1-client-protocol.h"
#include "protocols/generated/xdg-shell-client-protocol.h"

// Required interfaces will cause a context creation failure if not available
// They also do not have to be null-checked to use.
// They are available at ctx->protocols.<interface_name>
// WL_REGISTRY_BINDING_ENTRY(interface_name, interface_version, listener)
#define WL_REGISTRY_REQUIRED_BINDINGS                                 \
  WL_REGISTRY_BINDING_ENTRY(wl_compositor, 4, NULL)                   \
  WL_REGISTRY_BINDING_ENTRY(wl_shm, 1, NULL)                          \
  WL_REGISTRY_BINDING_ENTRY(wl_seat, 5, &_lvkw_wayland_seat_listener) \
  WL_REGISTRY_BINDING_ENTRY(xdg_wm_base, 1, &_lvkw_wayland_wm_base_listener)
// end of table

// Optional interfaces are... optional!
// They should always be null-checked before use.
// They are available at ctx->protocols.opt.<interface_name>
#define WL_REGISTRY_OPTIONAL_BINDINGS                                 \
  WL_REGISTRY_BINDING_ENTRY(zxdg_decoration_manager_v1, 1, NULL)      \
  WL_REGISTRY_BINDING_ENTRY(zwp_relative_pointer_manager_v1, 1, NULL) \
  WL_REGISTRY_BINDING_ENTRY(zwp_pointer_constraints_v1, 1, NULL)      \
  WL_REGISTRY_BINDING_ENTRY(wp_viewporter, 1, NULL)                   \
  WL_REGISTRY_BINDING_ENTRY(wp_fractional_scale_manager_v1, 1, NULL)  \
  WL_REGISTRY_BINDING_ENTRY(zwp_idle_inhibit_manager_v1, 1, NULL)     \
  WL_REGISTRY_BINDING_ENTRY(xdg_activation_v1, 1, NULL)               \
  WL_REGISTRY_BINDING_ENTRY(wp_cursor_shape_manager_v1, 1, NULL)      \
  WL_REGISTRY_BINDING_ENTRY(wp_content_type_manager_v1, 1, NULL)      \
  WL_REGISTRY_BINDING_ENTRY(ext_idle_notifier_v1, 1, NULL)
// end of table

#define WL_REGISTRY_BINDING_ENTRY(name, version, listener) struct name *name;
typedef struct LVKW_Wayland_Protocols_WL {
  WL_REGISTRY_REQUIRED_BINDINGS
  struct {
    WL_REGISTRY_OPTIONAL_BINDINGS
  } opt;
} LVKW_Wayland_Protocols_WL;
#undef WL_REGISTRY_BINDING_ENTRY

#endif
