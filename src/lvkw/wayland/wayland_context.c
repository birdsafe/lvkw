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
#include "lvkw_wayland_internal.h"

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
}
static void _registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {}

static const struct wl_registry_listener _registry_listener = {
    .global = _registry_handle_global,
    .global_remove = _registry_handle_global_remove,
};

static void _destroy_registry(LVKW_Context_WL *ctx) {
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
    LVKW_REPORT_CTX_DIAGNOSIS(ctx, LVKW_DIAGNOSIS_FEATURE_UNSUPPORTED,                     \
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
    snprintf(msg, sizeof(msg), "Failed to load a required dynamic library: %s", lvkw_wayland_loader_get_diagnosis());
    LVKW_REPORT_BOOTSTRAP_DIAGNOSIS(create_info, LVKW_DIAGNOSIS_DYNAMIC_LIB_FAILURE, msg);
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

  _lvkw_context_init_base(&ctx->base, create_info);
#ifdef LVKW_INDIRECT_BACKEND
  ctx->base.prv.backend = &_lvkw_wayland_backend;
#endif
  ctx->base.prv.alloc_cb = allocator;

  ctx->wl.display = wl_display_connect(NULL);
  if (!ctx->wl.display) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "Failed to connect to wayland display");
    goto cleanup_ctx;
  }

  ctx->input.xkb.ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!ctx->input.xkb.ctx) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "Failed to create xkb context");
    goto cleanup_display;
  }

  ctx->wl.registry = wl_display_get_registry(ctx->wl.display);
  if (!ctx->wl.registry) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "Failed to obtain wayland registry");
    goto cleanup_xkb;
  }

  int res = wl_registry_add_listener(ctx->wl.registry, &_registry_listener, ctx);
  if (res == -1) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "wl_registry_add_listener() failure");
    goto cleanup_registry;
  }

  res = wl_display_roundtrip(ctx->wl.display);
  if (res == -1) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_RESOURCE_UNAVAILABLE, "wl_display_roundtrip() failure");
    goto cleanup_registry;
  }

  if (!_required_wl_ifaces_bound(ctx)) {
    result = LVKW_ERROR;
    goto cleanup_registry;
  }

  ctx->wl.cursor_theme = wl_cursor_theme_load(NULL, 24, ctx->protocols.wl_shm);
  ctx->wl.cursor_surface = wl_compositor_create_surface(ctx->protocols.wl_compositor);

  LVKW_Status q_res = lvkw_event_queue_init(&ctx->base, &ctx->events.queue, 64, LVKW_WAYLAND_MAX_EVENTS);
  if (q_res != LVKW_SUCCESS) {
    LVKW_REPORT_CTX_DIAGNOSIS(&ctx->base, LVKW_DIAGNOSIS_OUT_OF_MEMORY, "Failed to allocate event queue pool");
    goto cleanup_registry;
  }

  *out_ctx_handle = (LVKW_Context *)ctx;
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

  lvkw_event_queue_cleanup(&ctx->base, &ctx->events.queue);

  wl_display_flush(ctx->wl.display);
  wl_display_disconnect(ctx->wl.display);

  lvkw_context_free(&ctx->base, ctx);

  lvkw_unload_wayland_symbols();
}

void lvkw_ctx_getVkExtensions_WL(LVKW_Context *ctx_handle, uint32_t *count,
                                                 const char **out_extensions) {
  static const char *extensions[] = {
      "VK_KHR_surface",
      "VK_KHR_wayland_surface",
  };

  uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

  if (out_extensions == NULL) {
    *count = extension_count;
    return;
  }

  uint32_t to_copy = (*count < extension_count) ? *count : extension_count;
  for (uint32_t i = 0; i < to_copy; ++i) {
    out_extensions[i] = extensions[i];
  }
  *count = to_copy;
}