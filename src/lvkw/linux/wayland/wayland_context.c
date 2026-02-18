// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "dlib/loader.h"
#include "api_constraints.h"
#include "assume.h"
#include "diagnostic_internal.h"
#include "wayland_internal.h"

#ifdef LVKW_ENABLE_CONTROLLER
#include "controller/controller_internal.h"
#endif

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_wayland_backend;
#endif

/* xdg_wm_base */

static void _wm_base_handle_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;
  LVKW_CONTEXT_ASSUME(data, wm_base != NULL, "wm_base is NULL in ping handler");

  lvkw_xdg_wm_base_pong(ctx, wm_base, serial);
}

const struct xdg_wm_base_listener _lvkw_wayland_wm_base_listener = {
    .ping = _wm_base_handle_ping,
};

/* libdecor */

static void _libdecor_handle_error(struct libdecor *context, enum libdecor_error error,
                                   const char *message) {
  (void)context;
  (void)error;
  (void)message;
}

static struct libdecor_interface _libdecor_interface = {
    .error = _libdecor_handle_error,
};

/* wl_registry */

static bool _wl_registry_try_bind(LVKW_Context_WL *ctx, struct wl_registry *registry,
                                  uint32_t name, const char *interface, uint32_t version,
                                  const char *target_name,
                                  const struct wl_interface *target_interface,
                                  uint32_t target_version, void **storage, const void *listener,
                                  void *data) {
  if (strcmp(interface, target_name) != 0) return false;
  if (*storage) return true;

#ifdef LVKW_ENABLE_FAULT_INJECTION
  const char *pretend_missing = getenv("LVKW_WAYLAND_PRETEND_MISSING");
  if (pretend_missing && pretend_missing[0] != '\0') {
    const char *it = pretend_missing;
    while (*it != '\0') {
      while (*it == ',' || *it == ' ' || *it == '\t') {
        ++it;
      }
      const char *start = it;
      while (*it != '\0' && *it != ',') {
        ++it;
      }
      const char *end = it;
      while (end > start && (end[-1] == ' ' || end[-1] == '\t')) {
        --end;
      }
      size_t len = (size_t)(end - start);
      if (len == 1u && start[0] == '*') {
        return true;
      }
      if (len == strlen(target_name) && strncmp(start, target_name, len) == 0) {
        return true;
      }
    }
  }
#endif

  uint32_t bind_version = (version < target_version) ? version : target_version;
  void *proxy = lvkw_wl_registry_bind(ctx, registry, name, target_interface, bind_version);
  if (!proxy) return true;

  if (listener) {
    if (lvkw_wl_proxy_add_listener(ctx, (struct wl_proxy *)proxy, (void (**)(void))listener, data) < 0) {
      LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_BACKEND_FAILURE,
                                 "Failed to add listener to Wayland proxy");
      lvkw_wl_proxy_destroy(ctx, (struct wl_proxy *)proxy);
      return true;
    }
  }

  *storage = proxy;
  return true;
}

static void _registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                                    const char *interface, uint32_t version) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)data;

  LVKW_CONTEXT_ASSUME(data, ctx != NULL, "Context handle must not be NULL in registry handler");
  LVKW_CONTEXT_ASSUME(data, registry != NULL, "Registry must not be NULL in registry handler");
  LVKW_CONTEXT_ASSUME(data, interface != NULL, "Interface must not be NULL in registry handler");

  // N.B. wl_output is not a singleton interface...
  // We COULD handle this through the macros with some extra logic, but since wl_output
  // is the only multiple-instance interface we currently care about, it's simpler to
  // just special case it here.
  if (strcmp(interface, "wl_output") == 0) {
    _lvkw_wayland_bind_output(ctx, name, version);
    return;
  }

#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener_ptr)                  \
  if (_wl_registry_try_bind(ctx, registry, name, interface, version, #iface_name, \
                            &iface_name##_interface, iface_version,                         \
                            (void **)&ctx->protocols.iface_name, listener_ptr, ctx))        \
    return;
  WL_REGISTRY_REQUIRED_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY

#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener_ptr)                      \
  if (_wl_registry_try_bind(ctx, registry, name, interface, version, #iface_name,        \
                            &iface_name##_interface, iface_version,                             \
                            (void **)&ctx->protocols.opt.iface_name, listener_ptr, ctx))        \
    return;
  WL_REGISTRY_OPTIONAL_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY
}
static void _registry_handle_global_remove(void *data, struct wl_registry *registry,
                                           uint32_t name) {
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
    lvkw_##iface_name##_destroy(ctx, ctx->protocols.iface_name); \
  }
  WL_REGISTRY_REQUIRED_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY

#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener) \
  if (ctx->protocols.opt.iface_name) {                                 \
    lvkw_##iface_name##_destroy(ctx, ctx->protocols.opt.iface_name); \
  }
  WL_REGISTRY_OPTIONAL_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY

  lvkw_wl_registry_destroy(ctx, ctx->wl.registry);
}

static inline bool _required_wl_ifaces_bound(LVKW_Context_WL *ctx) {
  bool result = true;
#define WL_REGISTRY_BINDING_ENTRY(iface_name, iface_version, listener)                      \
  if (!ctx->protocols.iface_name) {                                                         \
    result = false;                                                                         \
    LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,                    \
                               "required Wayland protocol: " #iface_name " was not found"); \
  }
  WL_REGISTRY_REQUIRED_BINDINGS
#undef WL_REGISTRY_BINDING_ENTRY
  return result;
}

LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info,
                               LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;

  LVKW_Status result = LVKW_ERROR;

  LVKW_Context_WL *ctx =
      (LVKW_Context_WL *)lvkw_context_alloc_bootstrap(create_info, sizeof(LVKW_Context_WL));

  if (!ctx) {
    result = LVKW_ERROR;
    goto cleanup_symbols;
  }

  memset(ctx, 0, sizeof(*ctx));
  ctx->input.dnd.async.fd = -1;

  if (_lvkw_context_init_base(&ctx->linux_base.base, create_info) != LVKW_SUCCESS) {
    lvkw_context_free(&ctx->linux_base.base, ctx);
    goto cleanup_symbols;
  }

  if (!lvkw_load_wayland_symbols(&ctx->linux_base.base, &ctx->dlib.wl, &ctx->dlib.wlc, &ctx->dlib.xkb,
                                 &ctx->dlib.opt.decor)) {
    goto cleanup_ctx;
  }

#ifdef LVKW_INDIRECT_BACKEND
  ctx->linux_base.base.prv.backend = &_lvkw_wayland_backend;
#endif

  ctx->decoration_mode = _lvkw_wayland_get_decoration_mode(create_info);
  ctx->dnd_post_drop_timeout_ms = create_info->tuning->wayland.dnd_post_drop_timeout_ms;
  ctx->enforce_client_side_constraints =
      create_info->tuning->wayland.enforce_client_side_constraints;

  if (!_lvkw_wayland_connect_display(ctx)) {
    goto cleanup_ctx;
  }

  ctx->linux_base.xkb.ctx = lvkw_xkb_context_new(ctx, XKB_CONTEXT_NO_FLAGS);
  if (!ctx->linux_base.xkb.ctx) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Failed to create xkb context");
    goto cleanup_display;
  }

  ctx->wl.registry = lvkw_wl_display_get_registry(ctx, ctx->wl.display);
  if (!ctx->wl.registry) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "Failed to obtain wayland registry");
    goto cleanup_xkb;
  }

  int res = lvkw_wl_registry_add_listener(ctx, ctx->wl.registry, &_registry_listener, ctx);
  if (res == -1) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "wl_registry_add_listener() failure");
    goto cleanup_registry;
  }

  res = lvkw_wl_display_roundtrip(ctx, ctx->wl.display);
  if (res == -1) {
#ifdef LVKW_ENABLE_DIAGNOSTICS
    char msg[256];
    snprintf(msg, sizeof(msg), "wl_display_roundtrip() failure: %s", strerror(errno));
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, msg);
#endif
    goto cleanup_registry;
  }

  if (!_required_wl_ifaces_bound(ctx)) {
    result = LVKW_ERROR;
    goto cleanup_registry;
  }

  // roundtrip one more time so that the wl_output creation events are processed immediately.
  res = lvkw_wl_display_roundtrip(ctx, ctx->wl.display);
  if (res == -1) {
#ifdef LVKW_ENABLE_DIAGNOSTICS
    char msg[256];
    snprintf(msg, sizeof(msg), "wl_display_roundtrip() failure (second pass): %s", strerror(errno));
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, msg);
#endif
    goto cleanup_registry;
  }

  if (ctx->dlib.opt.decor.base.available) {
    ctx->libdecor.ctx =
        lvkw_libdecor_new(ctx, ctx->wl.display, &_libdecor_interface);
  }

  ctx->wl.cursor_theme = lvkw_wl_cursor_theme_load(ctx, NULL, 24, ctx->protocols.wl_shm);
  ctx->wl.cursor_surface = lvkw_wl_compositor_create_surface(ctx, ctx->protocols.wl_compositor);

  for (int i = 1; i <= 12; i++) {
    ctx->linux_base.base.prv.standard_cursors[i].pub.flags = LVKW_CURSOR_FLAG_SYSTEM;
    ctx->linux_base.base.prv.standard_cursors[i].prv.ctx_base = &ctx->linux_base.base;
#ifdef LVKW_INDIRECT_BACKEND
    ctx->linux_base.base.prv.standard_cursors[i].prv.backend = ctx->linux_base.base.prv.backend;
#endif
    ctx->linux_base.base.prv.standard_cursors[i].prv.shape = (LVKW_CursorShape)i;
  }

  if (ctx->protocols.opt.ext_idle_notifier_v1 && ctx->protocols.wl_seat) {
    ctx->idle.timeout_ms = 60000;
    ctx->idle.notification =
        lvkw_ext_idle_notifier_v1_get_idle_notification(ctx, ctx->protocols.opt.ext_idle_notifier_v1,
                                                        ctx->idle.timeout_ms, ctx->protocols.wl_seat);
    if (ctx->idle.notification) {
      lvkw_ext_idle_notification_v1_add_listener(ctx, ctx->idle.notification,
                                                 &_lvkw_wayland_idle_listener, ctx);
    }
  }

  *out_ctx_handle = (LVKW_Context *)ctx;

  // Apply initial attributes
  result = lvkw_ctx_update_WL((LVKW_Context *)ctx, 0xFFFFFFFF, &create_info->attributes);
  if (result != LVKW_SUCCESS) {
    *out_ctx_handle = NULL;
    lvkw_ctx_destroy_WL((LVKW_Context *)ctx);
    return result;
  }

  ctx->linux_base.base.pub.flags |= LVKW_CONTEXT_STATE_READY;

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_init_context_Linux(&ctx->linux_base.base, &ctx->linux_base.controller, _lvkw_wayland_push_event_cb);
#endif

  return LVKW_SUCCESS;

cleanup_registry:
  _destroy_registry(ctx);
cleanup_xkb:
  if (ctx->linux_base.xkb.ctx) lvkw_xkb_context_unref(ctx, ctx->linux_base.xkb.ctx);
cleanup_display:
  _lvkw_wayland_disconnect_display(ctx);
cleanup_ctx:
  lvkw_unload_wayland_symbols(&ctx->dlib.wl, &ctx->dlib.wlc, &ctx->dlib.xkb, &ctx->dlib.opt.decor);
  lvkw_context_free(&ctx->linux_base.base, ctx);
cleanup_symbols:
  return result;
}

LVKW_Status lvkw_ctx_destroy_WL(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  _lvkw_wayland_dnd_reset(ctx, false);

  for (int target = 0; target < 2; ++target) {
    LVKW_WaylandSelectionState *state = &ctx->input.selections[target];
    if (target == 0 && state->source) {
      lvkw_wl_data_source_destroy(ctx, state->source);
    } else if (target == 1 && state->primary_source) {
      lvkw_zwp_primary_selection_source_v1_destroy(ctx, state->primary_source);
    }

    if (target == 0 && state->offer) {
      if (ctx->input.dnd.offer == state->offer) {
        ctx->input.dnd.offer = NULL;
      }
      _lvkw_wayland_offer_destroy(ctx, state->offer);
    } else if (target == 1 && state->primary_offer) {
      _lvkw_wayland_primary_offer_destroy(ctx, state->primary_offer);
    }

    for (uint32_t i = 0; i < state->owned_mime_count; ++i) {
      lvkw_context_free(&ctx->linux_base.base, state->owned_mimes[i].bytes);
    }
    if (state->owned_mimes) {
      lvkw_context_free(&ctx->linux_base.base, state->owned_mimes);
    }
    if (state->read_cache) {
      lvkw_context_free(&ctx->linux_base.base, state->read_cache);
    }
    if (state->mime_query_ptr) {
      lvkw_context_free(&ctx->linux_base.base, (void *)state->mime_query_ptr);
    }
  }

  if (ctx->input.text_input) {
    lvkw_zwp_text_input_v3_destroy(ctx, ctx->input.text_input);
  }
  if (ctx->input.data_device) {
    lvkw_wl_data_device_destroy(ctx, ctx->input.data_device);
  }
  if (ctx->input.keyboard) {
    lvkw_wl_keyboard_destroy(ctx, ctx->input.keyboard);
  }
  if (ctx->input.cursor_shape_device) {
    lvkw_wp_cursor_shape_device_v1_destroy(ctx, ctx->input.cursor_shape_device);
  }
  if (ctx->input.pointer) {
    lvkw_wl_pointer_destroy(ctx, ctx->input.pointer);
  }
  if (ctx->wl.cursor_surface) {
    lvkw_wl_surface_destroy(ctx, ctx->wl.cursor_surface);
  }
  if (ctx->wl.cursor_theme) {
    lvkw_wl_cursor_theme_destroy(ctx, ctx->wl.cursor_theme);
  }

  if (ctx->idle.notification) {
    lvkw_ext_idle_notification_v1_destroy(ctx, ctx->idle.notification);
  }

  if (ctx->libdecor.ctx) {
    lvkw_libdecor_unref(ctx, ctx->libdecor.ctx);
  }


  if (ctx->linux_base.xkb.state) lvkw_xkb_state_unref(ctx, ctx->linux_base.xkb.state);
  if (ctx->linux_base.xkb.keymap) lvkw_xkb_keymap_unref(ctx, ctx->linux_base.xkb.keymap);
  if (ctx->linux_base.xkb.ctx) lvkw_xkb_context_unref(ctx, ctx->linux_base.xkb.ctx);

  _destroy_registry(ctx);

  LVKW_WaylandDndPayload *payload = ctx->dnd_payloads;
  while (payload) {
    LVKW_WaylandDndPayload *next = payload->next;
    for (uint16_t i = 0; i < payload->path_count; i++) {
      lvkw_context_free(&ctx->linux_base.base, (void *)payload->paths[i]);
    }
    lvkw_context_free(&ctx->linux_base.base, (void *)payload->paths);
    lvkw_context_free(&ctx->linux_base.base, payload);
    payload = next;
  }

#ifdef LVKW_ENABLE_CONTROLLER
  _lvkw_ctrl_cleanup_context_Linux(&ctx->linux_base.base, &ctx->linux_base.controller);
#endif

  _lvkw_context_cleanup_base(&ctx->linux_base.base);

  _lvkw_wayland_disconnect_display(ctx);

  lvkw_unload_wayland_symbols(&ctx->dlib.wl, &ctx->dlib.wlc, &ctx->dlib.xkb, &ctx->dlib.opt.decor);

  lvkw_context_free(&ctx->linux_base.base, ctx);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitors_WL(LVKW_Context *ctx, LVKW_MonitorRef **out_refs,
                                    uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx, out_refs, count);

  LVKW_Context_WL *wl_ctx = (LVKW_Context_WL *)ctx;

  if (!out_refs) {
    uint32_t monitor_count = 0;
    for (LVKW_Monitor_Base *m = wl_ctx->linux_base.base.prv.monitor_list; m != NULL; m = m->prv.next) {
      if (!(m->pub.flags & LVKW_MONITOR_STATE_LOST)) {
        monitor_count++;
      }
    }
    *count = monitor_count;
    return LVKW_SUCCESS;
  }

  uint32_t room = *count;
  uint32_t filled = 0;
  for (LVKW_Monitor_Base *m = wl_ctx->linux_base.base.prv.monitor_list; m != NULL; m = m->prv.next) {
    if (m->pub.flags & LVKW_MONITOR_STATE_LOST) continue;

    if (filled < room) {
      out_refs[filled++] = (LVKW_MonitorRef *)&m->pub;
    }
    else {
      break;
    }
  }
  *count = filled;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitorModes_WL(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                        LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx, monitor, out_modes, count);

  LVKW_Monitor_WL *target_monitor = (LVKW_Monitor_WL *)monitor;

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

LVKW_Status lvkw_ctx_getVkExtensions_WL(LVKW_Context *ctx_handle, uint32_t *count,
                                        const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);

  static const char *extensions[] = {
      "VK_KHR_surface",
      "VK_KHR_wayland_surface",
      NULL,
  };

  *count = 2;
  *out_extensions = extensions;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMetrics_WL(LVKW_Context *ctx, LVKW_MetricsCategory category,
                                     void *out_data, bool reset) {
  LVKW_API_VALIDATE(ctx_getMetrics, ctx, category, out_data, reset);

  LVKW_Context_WL *wl_ctx = (LVKW_Context_WL *)ctx;

  switch (category) {
    case LVKW_METRICS_CATEGORY_EVENTS:
      lvkw_event_queue_get_metrics(&wl_ctx->linux_base.base.prv.event_queue, (LVKW_EventMetrics *)out_data, reset);
      return LVKW_SUCCESS;
    default:
      LVKW_REPORT_CTX_DIAGNOSTIC(&wl_ctx->linux_base.base, LVKW_DIAGNOSTIC_FEATURE_UNSUPPORTED,
                                 "Unknown metrics category");
      return LVKW_ERROR;
  }
}
