#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dlib/libdecor.h"
#include "dlib/loader.h"
#include "dlib/wayland-client.h"
#include "dlib/wayland-cursor.h"
#include "dlib/xkbcommon.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_checks.h"
#include "lvkw_diagnostic_internal.h"
#include "lvkw_wayland_internal.h"

#ifdef LVKW_CONTROLLER_ENABLED
#include "controller/lvkw_controller_internal.h"
#endif

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_wayland_backend;
#endif

/* xdg_wm_base */

static void _wm_base_handle_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial) {
  LVKW_CTX_ASSUME(data, wm_base != NULL, "wm_base is NULL in ping handler");

  xdg_wm_base_pong(wm_base, serial);
}

const struct xdg_wm_base_listener _lvkw_wayland_wm_base_listener = {
    .ping = _wm_base_handle_ping,
};

/* wl_registry */

static bool _wl_registry_try_bind(struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version,
                                  const char *target_name, const struct wl_interface *target_interface,
                                  uint32_t target_version, void **storage, const void *listener, void *data) {
  if (strcmp(interface, target_name) != 0) return false;

  uint32_t bind_version = (version < target_version) ? version : target_version;
  void *proxy = wl_registry_bind(registry, name, target_interface, bind_version);
  *storage = proxy;

  if (listener) {
    wl_proxy_add_listener((struct wl_proxy *)proxy, (void (**)(void))listener, data);
  }

  return true;
}

static void _registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                                    uint32_t version) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CTX_ASSUME(data, ctx != NULL, "Context handle must not be NULL in registry handler");
  LVKW_CTX_ASSUME(data, registry != NULL, "Registry must not be NULL in registry handler");
  LVKW_CTX_ASSUME(data, interface != NULL, "Interface must not be NULL in registry handler");

  // N.B. wl_output is not a singleton interface...
  // We COULD handle this through the macros with some extra logic, but since wl_output
  // is the only multiple-instance interface we currently care about, it's simpler to
  // just special case it here.
  if (strcmp(interface, "wl_output") == 0) {
    _lvkw_wayland_bind_output(ctx, name, version);
    return;
  }

#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener_ptr)                                           \
  if (_wl_registry_try_bind(registry, name, interface, version, #iface_name, &iface_name##_interface, iface_version, \
                            (void **)&ctx->protocols.iface_name, listener_ptr, ctx))                                 \
    return;
  WL_REGISTRY_REQUIRED_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY

#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener_ptr)                                           \
  if (_wl_registry_try_bind(registry, name, interface, version, #iface_name, &iface_name##_interface, iface_version, \
                            (void **)&ctx->protocols.opt.iface_name, listener_ptr, ctx))                             \
    return;
  WL_REGISTRY_OPTIONAL_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY

  printf("Unrecognized global: %s (name=%u, version=%u)\n", interface, name, version);
}
static void _registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  _lvkw_wayland_remove_monitor_by_name(ctx, name);
}

static const struct wl_registry_listener _registry_listener = {
    .global = _registry_handle_global,
    .global_remove = _registry_handle_global_remove,
};

static void _destroy_registry(LVKW_Context_WL *ctx) {
  /* Clean up monitors first */
  _lvkw_wayland_destroy_monitors(ctx);

#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener) \
  if (ctx->protocols.iface_name) {                                     \
    iface_name##_destroy(ctx->protocols.iface_name);                   \
  }
  WL_REGISTRY_REQUIRED_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY

#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener) \
  if (ctx->protocols.opt.iface_name) {                                 \
    iface_name##_destroy(ctx->protocols.opt.iface_name);               \
  }
  WL_REGISTRY_OPTIONAL_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY

  wl_registry_destroy(ctx->wl.registry);
}

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

static inline bool _required_wl_ifaces_bound(LVKW_Context_WL *ctx) {
  bool result = true;
#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener)                     \
  if (!ctx->protocols.iface_name) {                                                        \
    result = false;                                                                        \
    LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,                     \
                              "required Wayland protocol: " #iface_name " was not found"); \
  }
  WL_REGISTRY_REQUIRED_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY
  return result;
}

LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  *out_ctx_handle = NULL;

  if (!lvkw_load_wayland_symbols()) {
    char msg[512];
    snprintf(msg, sizeof(msg), "Failed to load a required dynamic library: %s", lvkw_wayland_loader_get_diagnostic());
    LVKW_REPORT_BOOTSTRAP_DIAGNOSTIC(create_info, LVKW_DIAGNOSTIC_DYNAMIC_LIB_FAILURE, msg);
    return LVKW_ERROR;
  }

  LVKW_Status result = LVKW_ERROR;

  LVKW_Allocator allocator = {.alloc_cb = _lvkw_default_alloc, .free_cb = _lvkw_default_free};
  if (create_info->allocator.alloc_cb) {
    allocator = create_info->allocator;
  }

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)lvkw_alloc(&allocator, create_info->userdata, sizeof(LVKW_Context_WL));

  if (!ctx) {
    result = LVKW_ERROR;
    goto cleanup_symbols;
  }

  memset(ctx, 0, sizeof(*ctx));

  _lvkw_context_init_base(&ctx->base, create_info);
  ctx->base.prv.alloc_cb = allocator;

#ifdef LVKW_INDIRECT_BACKEND
  ctx->base.prv.backend = &_lvkw_wayland_backend;
#endif

  ctx->wl.display = wl_display_connect(NULL);
  if (!ctx->wl.display) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "Failed to connect to wayland display");
    goto cleanup_ctx;
  }

  ctx->input.xkb.ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!ctx->input.xkb.ctx) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "Failed to create xkb context");
    goto cleanup_display;
  }

  ctx->wl.registry = wl_display_get_registry(ctx->wl.display);
  if (!ctx->wl.registry) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "Failed to obtain wayland registry");
    goto cleanup_xkb;
  }

  int res = wl_registry_add_listener(ctx->wl.registry, &_registry_listener, ctx);
  if (res == -1) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "wl_registry_add_listener() failure");
    goto cleanup_registry;
  }

  res = wl_display_roundtrip(ctx->wl.display);
  if (res == -1) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "wl_display_roundtrip() failure");
    goto cleanup_registry;
  }

  if (!_required_wl_ifaces_bound(ctx)) {
    result = LVKW_ERROR;
    goto cleanup_registry;
  }

  // roundtrip one more time so that the wl_output creation events are processed immediately.
  res = wl_display_roundtrip(ctx->wl.display);
  if (res == -1) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "wl_display_roundtrip() failure");
    goto cleanup_registry;
  }

  ctx->wl.cursor_theme = wl_cursor_theme_load(NULL, 24, ctx->protocols.wl_shm);
  ctx->wl.cursor_surface = wl_compositor_create_surface(ctx->protocols.wl_compositor);

  _LVKW_EventTuning tuning = _lvkw_get_event_tuning(create_info);
  LVKW_Status q_res = lvkw_event_queue_init(&ctx->base, &ctx->events.queue, tuning.initial_capacity,
                                            tuning.max_capacity, tuning.growth_factor);
  if (q_res != LVKW_SUCCESS) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_OUT_OF_MEMORY, "Failed to allocate event queue pool");
    goto cleanup_registry;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

  // Apply initial attributes
  lvkw_ctx_update_WL((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);

  ctx->base.pub.flags |= LVKW_CTX_STATE_READY;

#ifdef LVKW_CONTROLLER_ENABLED
  _lvkw_ctrl_init_context_Linux(&ctx->base, &ctx->controller, (void (*)(LVKW_Context_Base *, const LVKW_Event *))_lvkw_wayland_push_event);
#endif

  return LVKW_SUCCESS;

cleanup_registry:
  _destroy_registry(ctx);
cleanup_xkb:
  if (ctx->input.xkb.ctx) xkb_context_unref(ctx->input.xkb.ctx);
cleanup_display:
  wl_display_disconnect(ctx->wl.display);
cleanup_ctx:
  lvkw_context_free(&ctx->base, ctx);
cleanup_symbols:
  lvkw_unload_wayland_symbols();
  return result;
}

void lvkw_ctx_destroy_WL(LVKW_Context *ctx_handle) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  LVKW_CTX_ASSUME(&ctx->base, ctx->base.prv.window_list == NULL,
                  "All windows must be destroyed before context destruction");

  if (ctx->input.keyboard) {
    wl_keyboard_destroy(ctx->input.keyboard);
  }
  if (ctx->input.cursor_shape_device) {
    wp_cursor_shape_device_v1_destroy(ctx->input.cursor_shape_device);
  }
  if (ctx->input.pointer) {
    wl_pointer_destroy(ctx->input.pointer);
  }
  if (ctx->wl.cursor_surface) {
    wl_surface_destroy(ctx->wl.cursor_surface);
  }
  if (ctx->wl.cursor_theme) {
    wl_cursor_theme_destroy(ctx->wl.cursor_theme);
  }

  if (ctx->idle.notification) {
    ext_idle_notification_v1_destroy(ctx->idle.notification);
  }

  if (ctx->libdecor.ctx) {
    libdecor_unref(ctx->libdecor.ctx);
  }

  if (ctx->input.xkb.state) xkb_state_unref(ctx->input.xkb.state);
  if (ctx->input.xkb.keymap) xkb_keymap_unref(ctx->input.xkb.keymap);
  if (ctx->input.xkb.ctx) xkb_context_unref(ctx->input.xkb.ctx);

  _destroy_registry(ctx);

  _lvkw_string_cache_destroy(&ctx->string_cache, &ctx->base);

#ifdef LVKW_CONTROLLER_ENABLED
  _lvkw_ctrl_cleanup_context_Linux(&ctx->base, &ctx->controller);
#endif

  lvkw_event_queue_cleanup(&ctx->base, &ctx->events.queue);
  _lvkw_context_cleanup_base(&ctx->base);

  wl_display_flush(ctx->wl.display);
  wl_display_disconnect(ctx->wl.display);

  lvkw_context_free(&ctx->base, ctx);

  lvkw_unload_wayland_symbols();
}

LVKW_Status lvkw_ctx_getMonitors_WL(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors, uint32_t *count) {
  LVKW_Context_WL *wl_ctx = (LVKW_Context_WL *)ctx;

  // N.B. We COULD cache the count. But since:
  // 1) A count request is almost always followed by a retrieval request, so we are in O(N) already
  // 2) This method is flagged as being in the cold path
  // We can just count on the fly here without worrying about it.
  if (!out_monitors) {
    uint32_t monitor_count = 0;
    for (LVKW_Monitor_WL *m = wl_ctx->monitors_list_start; m != NULL; m = m->next) {
      monitor_count++;
    }
    *count = monitor_count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *count;
  uint32_t filled = 0;
  for (LVKW_Monitor_WL *m = wl_ctx->monitors_list_start; m != NULL; m = m->next) {
    if (filled < room) {
      out_monitors[filled++] = m->info;
    }
    else {
      break;
    }
  }
  *count = filled;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitorModes_WL(LVKW_Context *ctx, LVKW_MonitorId monitor, LVKW_VideoMode *out_modes,
                                        uint32_t *count) {
  LVKW_Context_WL *wl_ctx = (LVKW_Context_WL *)ctx;

  LVKW_Monitor_WL *target_monitor = NULL;
  for (LVKW_Monitor_WL *m = wl_ctx->monitors_list_start; m != NULL; m = m->next) {
    if (m->info.id == monitor) {
      target_monitor = m;
      break;
    }
  }

  if (!target_monitor) {
    *count = 0;
    LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "Monitor not found");
    return LVKW_ERROR;
  }

  if (!out_modes) {
    *count = target_monitor->mode_count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *count;
  uint32_t filled = 0;
  for (uint32_t i = 0; i < target_monitor->mode_count && filled < room; i++) {
    out_modes[filled++] = target_monitor->modes[i];
  }
  *count = filled;
  return LVKW_SUCCESS;
}

const char *const *lvkw_ctx_getVkExtensions_WL(LVKW_Context *ctx_handle, uint32_t *count) {
  (void)ctx_handle;
  static const char *extensions[] = {
      "VK_KHR_surface",
      "VK_KHR_wayland_surface",
      NULL,
  };

  if (count) {
    *count = 2;
  }
  return extensions;
}