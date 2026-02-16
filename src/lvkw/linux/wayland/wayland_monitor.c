// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "lvkw_diagnostic_internal.h"
#include "lvkw_mem_internal.h"
#include "lvkw_string_cache.h"
#include "lvkw_wayland_internal.h"

/* wl_output event handlers */

static void _output_handle_geometry(void *data, struct wl_output *wl_output, int32_t x, int32_t y,
                                    int32_t physical_width, int32_t physical_height,
                                    int32_t subpixel, const char *make, const char *model,
                                    int32_t transform) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)monitor->base.prv.ctx_base;

  monitor->base.pub.physical_size.x = (LVKW_Scalar)physical_width;
  monitor->base.pub.physical_size.y = (LVKW_Scalar)physical_height;

  if (!monitor->base.pub.name) {
    size_t len = strlen(make) + strlen(model) + 2;  // "Make Model\0"
    char buf[512];
    if (len < sizeof(buf)) {
      snprintf(buf, sizeof(buf), "%s %s", make, model);
      monitor->base.pub.name = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, buf);
    }
  }
}

static void _output_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags,
                                int32_t width, int32_t height, int32_t refresh) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)monitor->base.prv.ctx_base;

  LVKW_VideoMode mode;
  mode.size.x = (int32_t)width;
  mode.size.y = (int32_t)height;
  mode.refresh_rate_mhz = (uint32_t)refresh;

  if (flags & WL_OUTPUT_MODE_CURRENT) {
    monitor->base.pub.current_mode = mode;
  }

  for (uint32_t i = 0; i < monitor->mode_count; ++i) {
    LVKW_VideoMode *existing = &monitor->modes[i];
    if (existing->size.x == mode.size.x && existing->size.y == mode.size.y &&
        existing->refresh_rate_mhz == mode.refresh_rate_mhz) {
      return;
    }
  }

  // Grow modes array
  uint32_t new_count = monitor->mode_count + 1;
  size_t old_size = monitor->mode_count * sizeof(LVKW_VideoMode);
  size_t new_size = new_count * sizeof(LVKW_VideoMode);

  LVKW_VideoMode *new_modes =
      (LVKW_VideoMode *)lvkw_context_realloc(&ctx->base, monitor->modes, old_size, new_size);

  if (new_modes) {
    new_modes[monitor->mode_count] = mode;
    monitor->modes = new_modes;
    monitor->mode_count = new_count;
  }
}

static void _output_handle_done(void *data, struct wl_output *wl_output) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)monitor->base.prv.ctx_base;

  // Fallback for logical size if xdg-output is not available
  if (monitor->base.pub.logical_size.x == 0 && monitor->base.pub.current_mode.size.x > 0) {
    LVKW_Scalar scale = monitor->base.pub.scale > (LVKW_Scalar)0.0 ? monitor->base.pub.scale : (LVKW_Scalar)1.0;
    monitor->base.pub.logical_size.x = (LVKW_Scalar)monitor->base.pub.current_mode.size.x / scale;
    monitor->base.pub.logical_size.y = (LVKW_Scalar)monitor->base.pub.current_mode.size.y / scale;
  }

  LVKW_Event evt = {0};

  if (!monitor->announced) {
    monitor->announced = true;
    evt.monitor_connection.monitor = &monitor->base.pub;
    evt.monitor_connection.connected = true;

    lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MONITOR_CONNECTION, NULL,
                          &evt);
  }
  else {
    evt.monitor_mode.monitor = &monitor->base.pub;

    lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MONITOR_MODE, NULL, &evt);
  }
}

static void _output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  monitor->base.pub.scale = (LVKW_Scalar)factor;
}

static void _output_handle_name(void *data, struct wl_output *wl_output, const char *name) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)monitor->base.prv.ctx_base;

  monitor->base.pub.name = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, name);
}

static void _output_handle_description(void *data, struct wl_output *wl_output,
                                       const char *description) {
  _output_handle_name(data, wl_output, description);
}

static void _xdg_output_handle_logical_position(void *data, struct zxdg_output_v1 *xdg_output,
                                                int32_t x, int32_t y) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  monitor->base.pub.logical_position.x = x;
  monitor->base.pub.logical_position.y = y;
}

static void _xdg_output_handle_logical_size(void *data, struct zxdg_output_v1 *xdg_output,
                                            int32_t width, int32_t height) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  monitor->base.pub.logical_size.x = (LVKW_Scalar)width;
  monitor->base.pub.logical_size.y = (LVKW_Scalar)height;
}

static void _xdg_output_handle_done(void *data, struct zxdg_output_v1 *xdg_output) {
  // We rely on wl_output.done to trigger events, as it is the "master" sync point.
}

static void _xdg_output_handle_name(void *data, struct zxdg_output_v1 *xdg_output,
                                    const char *name) {}

static void _xdg_output_handle_description(void *data, struct zxdg_output_v1 *xdg_output,
                                           const char *description) {}

static const struct zxdg_output_v1_listener _xdg_output_listener = {
    .logical_position = _xdg_output_handle_logical_position,
    .logical_size = _xdg_output_handle_logical_size,
    .done = _xdg_output_handle_done,
    .name = _xdg_output_handle_name,
    .description = _xdg_output_handle_description,
};

const struct wl_output_listener _lvkw_wayland_output_listener = {
    .geometry = _output_handle_geometry,
    .mode = _output_handle_mode,
    .done = _output_handle_done,
    .scale = _output_handle_scale,
    .name = _output_handle_name,
    .description = _output_handle_description,
};

void _lvkw_wayland_bind_output(LVKW_Context_WL *ctx, uint32_t name, uint32_t version) {
  uint32_t bind_version = (version < 4) ? version : 4;

  struct wl_output *output = (struct wl_output *)lvkw_wl_registry_bind(
      ctx, ctx->wl.registry, name, &wl_output_interface, bind_version);

  if (!output) {
#ifdef LVKW_ENABLE_DIAGNOSTICS
    char msg[256];
    snprintf(msg, sizeof(msg), "Failed to bind wl_output global (name %u): %s", name, strerror(errno));
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, msg);
#endif
    return;
  }

  LVKW_Monitor_WL *monitor =
      (LVKW_Monitor_WL *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Monitor_WL));
  if (!monitor) {
    lvkw_wl_output_destroy(ctx, output);
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_OUT_OF_MEMORY,
                               "Failed to allocate monitor metadata");
    return;
  }

  memset(monitor, 0, sizeof(LVKW_Monitor_WL));
  monitor->base.prv.ctx_base = &ctx->base;
  monitor->wayland_name = name;
  monitor->wl_output = output;
  monitor->base.pub.is_primary = (ctx->base.prv.monitor_list == NULL);
  monitor->base.pub.scale = 1.0;

  // Add to monitor list
  monitor->base.prv.next = ctx->base.prv.monitor_list;
  ctx->base.prv.monitor_list = &monitor->base;

  lvkw_wl_output_add_listener(ctx, output, &_lvkw_wayland_output_listener, monitor);

  if (ctx->protocols.opt.zxdg_output_manager_v1) {
    monitor->xdg_output =
        lvkw_zxdg_output_manager_v1_get_xdg_output(ctx, ctx->protocols.opt.zxdg_output_manager_v1, output);
    lvkw_zxdg_output_v1_add_listener(ctx, monitor->xdg_output, &_xdg_output_listener, monitor);
  }
}

static void _release_wl_output(LVKW_Context_WL *ctx, struct wl_output *output) {
  if (!output) return;
  if (lvkw_wl_output_get_version(ctx, output) >= WL_OUTPUT_RELEASE_SINCE_VERSION) {
    lvkw_wl_output_release(ctx, output);
  }
  else {
    lvkw_wl_output_destroy(ctx, output);
  }
}

void _lvkw_wayland_remove_monitor_by_name(LVKW_Context_WL *ctx, uint32_t name) {
  for (LVKW_Monitor_Base *m = ctx->base.prv.monitor_list; m != NULL; m = m->prv.next) {
    LVKW_Monitor_WL *mwl = (LVKW_Monitor_WL *)m;
    if (mwl->wayland_name == name) {
      // Mark as lost rather than removing from list
      m->pub.flags |= LVKW_MONITOR_STATE_LOST;

      // Notify the user about the disconnection
      LVKW_Event evt = {0};
      evt.monitor_connection.monitor = &m->pub;
      evt.monitor_connection.connected = false;
      lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MONITOR_CONNECTION,
                            NULL, &evt);

      if (mwl->wl_output) {
        _release_wl_output(ctx, mwl->wl_output);
        mwl->wl_output = NULL;
      }

      if (mwl->xdg_output) {
        lvkw_zxdg_output_v1_destroy(ctx, mwl->xdg_output);
        mwl->xdg_output = NULL;
      }
      return;
    }
  }
}

void _lvkw_wayland_destroy_monitors(LVKW_Context_WL *ctx) {
  LVKW_Monitor_Base *current = ctx->base.prv.monitor_list;
  while (current) {
    LVKW_Monitor_Base *next = current->prv.next;
    LVKW_Monitor_WL *mwl = (LVKW_Monitor_WL *)current;

    _release_wl_output(ctx, mwl->wl_output);

    if (mwl->xdg_output) {
      lvkw_zxdg_output_v1_destroy(ctx, mwl->xdg_output);
    }

    if (mwl->modes) {
      lvkw_context_free(&ctx->base, mwl->modes);
    }

    lvkw_context_free(&ctx->base, mwl);
    current = next;
  }
  ctx->base.prv.monitor_list = NULL;
}

struct wl_output *_lvkw_wayland_find_monitor(LVKW_Context_WL *ctx, const LVKW_Monitor *monitor) {
  for (LVKW_Monitor_Base *m = ctx->base.prv.monitor_list; m != NULL; m = m->prv.next) {
    if (&m->pub == monitor) {
      return ((LVKW_Monitor_WL *)m)->wl_output;
    }
  }
  return NULL;
}
