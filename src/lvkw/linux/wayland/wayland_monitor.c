#include <stdio.h>
#include <string.h>

#include "dlib/wayland-client.h"
#include "lvkw_diagnostic_internal.h"
#include "lvkw_mem_internal.h"
#include "lvkw_string_cache.h"
#include "lvkw_wayland_internal.h"
#include "wayland-client-protocol.h"

/* Monitor state during enumeration */
typedef struct {
  LVKW_Context_WL *ctx;
  LVKW_MonitorInfo info;
  uint32_t monitor_index;
} MonitorState;

/* wl_output event handlers */

static void _output_handle_geometry(void *data, struct wl_output *wl_output, int32_t x, int32_t y,
                                    int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make,
                                    const char *model, int32_t transform) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = monitor->ctx;

  monitor->info.physical_size.x = (double)physical_width;
  monitor->info.physical_size.y = (double)physical_height;

  if (!monitor->info.name) {
    size_t len = strlen(make) + strlen(model) + 2;  // "Make Model\0"
    char buf[512];
    if (len < sizeof(buf)) {
      snprintf(buf, sizeof(buf), "%s %s", make, model);
      monitor->info.name = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, buf);
    }
  }
}

static void _output_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height,
                                int32_t refresh) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = monitor->ctx;

  LVKW_VideoMode mode;
  mode.size.x = (int32_t)width;
  mode.size.y = (int32_t)height;
  mode.refresh_rate_mhz = (uint32_t)refresh;

  if (flags & WL_OUTPUT_MODE_CURRENT) {
    monitor->info.current_mode = mode;
  }

  // Grow modes array
  uint32_t new_count = monitor->mode_count + 1;
  size_t old_size = monitor->mode_count * sizeof(LVKW_VideoMode);
  size_t new_size = new_count * sizeof(LVKW_VideoMode);

  LVKW_VideoMode *new_modes = (LVKW_VideoMode *)lvkw_context_realloc(&ctx->base, monitor->modes, old_size, new_size);

  if (new_modes) {
    new_modes[monitor->mode_count] = mode;
    monitor->modes = new_modes;
    monitor->mode_count = new_count;
  }
}

static void _output_handle_done(void *data, struct wl_output *wl_output) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = monitor->ctx;

  // Fallback for logical size if xdg-output is not available
  if (monitor->info.logical_size.x == 0 && monitor->info.current_mode.size.x > 0) {
    double scale = monitor->info.scale > 0.0 ? monitor->info.scale : 1.0;
    monitor->info.logical_size.x = (double)monitor->info.current_mode.size.x / scale;
    monitor->info.logical_size.y = (double)monitor->info.current_mode.size.y / scale;
  }

  LVKW_Event evt = {0};

  if (!monitor->announced) {
    monitor->announced = true;
    evt.type = LVKW_EVENT_TYPE_MONITOR_CONNECTION;
    evt.monitor_connection.monitor = monitor->info;
    evt.monitor_connection.connected = true;
  }
  else {
    evt.type = LVKW_EVENT_TYPE_MONITOR_MODE;
    evt.monitor_mode.monitor = monitor->info;
  }

  _lvkw_wayland_push_event(ctx, &evt);
}

static void _output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  monitor->info.scale = (double)factor;
}

static void _output_handle_name(void *data, struct wl_output *wl_output, const char *name) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = monitor->ctx;

  monitor->info.name = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, name);
}

static void _output_handle_description(void *data, struct wl_output *wl_output, const char *description) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  LVKW_Context_WL *ctx = monitor->ctx;

  monitor->info.name = _lvkw_string_cache_intern(&ctx->string_cache, &ctx->base, description);
}

static void _xdg_output_handle_logical_position(void *data, struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  monitor->info.logical_position.x = x;
  monitor->info.logical_position.y = y;
}

static void _xdg_output_handle_logical_size(void *data, struct zxdg_output_v1 *xdg_output, int32_t width,
                                            int32_t height) {
  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)data;
  monitor->info.logical_size.x = (double)width;
  monitor->info.logical_size.y = (double)height;
}

static void _xdg_output_handle_done(void *data, struct zxdg_output_v1 *xdg_output) {
  // We rely on wl_output.done to trigger events, as it is the "master" sync point.
  // xdg_output data is auxiliary.
}

static void _xdg_output_handle_name(void *data, struct zxdg_output_v1 *xdg_output, const char *name) {
  // Use wl_output name as primary if available, but this is a good fallback/supplement.
}

static void _xdg_output_handle_description(void *data, struct zxdg_output_v1 *xdg_output, const char *description) {
  // Similar to name.
}

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
  // TODO: create a helper function fow wayland version number resolving.
  // TODO: review that logic, it's a bit suspect.
  uint32_t bind_version = (version < 4) ? version : 4;

  struct wl_output *output =
      (struct wl_output *)wl_registry_bind(ctx->wl.registry, name, &wl_output_interface, bind_version);

  if (!output) {
    // This is recoverable enough that we can just skip it and continue without monitor info.
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE, "Failed to bind wl_output");
    return;
  }

  LVKW_Monitor_WL *monitor = (LVKW_Monitor_WL *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Monitor_WL));
  if (!monitor) {
    wl_output_destroy(output);
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_OUT_OF_MEMORY, "Failed to allocate monitor metadata");
    return;
  }

  memset(monitor, 0, sizeof(LVKW_Monitor_WL));
  monitor->ctx = ctx;
  monitor->global_id = name;
  monitor->wl_output = output;
  monitor->info.id = ++ctx->last_monitor_id;
  monitor->info.is_primary = (ctx->monitors_list_start == NULL);  //?????
  monitor->info.scale = 1.0;                                      // ????????
  monitor->next = ctx->monitors_list_start;
  ctx->monitors_list_start = monitor;

  // TODO: Check to see when these get invoked, and in particular if we need to roundtrip
  // the display to get them to fire before we can return from context creation.
  // Ultimately, we want to make sure that lvkw_getMonitors() can be called immediately after context
  // creation and return valid data.
  wl_output_add_listener(output, &_lvkw_wayland_output_listener, monitor);

  if (ctx->protocols.opt.zxdg_output_manager_v1) {
    monitor->xdg_output = zxdg_output_manager_v1_get_xdg_output(ctx->protocols.opt.zxdg_output_manager_v1, output);
    zxdg_output_v1_add_listener(monitor->xdg_output, &_xdg_output_listener, monitor);
  }
}

void _lvkw_wayland_remove_monitor_by_name(LVKW_Context_WL *ctx, uint32_t name) {
  LVKW_Monitor_WL **curr = &ctx->monitors_list_start;
  while (*curr) {
    LVKW_Monitor_WL *entry = *curr;
    if (entry->global_id == name) {
      *curr = entry->next;

      // Notify the user about the disconnection
      LVKW_Event evt = {0};
      evt.type = LVKW_EVENT_TYPE_MONITOR_CONNECTION;
      evt.monitor_connection.monitor = entry->info;
      evt.monitor_connection.connected = false;
      _lvkw_wayland_push_event(ctx, &evt);

      if (entry->wl_output) {
        if (wl_output_get_version(entry->wl_output) >= WL_OUTPUT_RELEASE_SINCE_VERSION) {
          wl_output_release(entry->wl_output);
        }
        else {
          wl_output_destroy(entry->wl_output);
        }
      }

      if (entry->xdg_output) {
        zxdg_output_v1_destroy(entry->xdg_output);
      }

      if (entry->modes) {
        lvkw_context_free(&ctx->base, entry->modes);
      }

      lvkw_context_free(&ctx->base, entry);
      return;
    }
    curr = &(*curr)->next;
  }
}

void _lvkw_wayland_destroy_monitors(LVKW_Context_WL *ctx) {
  LVKW_Monitor_WL *current = ctx->monitors_list_start;
  while (current) {
    LVKW_Monitor_WL *next = current->next;

    if (current->wl_output) {
      if (wl_output_get_version(current->wl_output) >= WL_OUTPUT_RELEASE_SINCE_VERSION) {
        wl_output_release(current->wl_output);
      }
      else {
        wl_output_destroy(current->wl_output);
      }
    }

    if (current->xdg_output) {
      zxdg_output_v1_destroy(current->xdg_output);
    }

    if (current->modes) {
      lvkw_context_free(&ctx->base, current->modes);
    }

    lvkw_context_free(&ctx->base, current);
    current = next;
  }
  ctx->monitors_list_start = NULL;
}

struct wl_output *_lvkw_wayland_find_monitor(LVKW_Context_WL *ctx, LVKW_MonitorId id) {
  LVKW_Monitor_WL *current = ctx->monitors_list_start;
  while (current) {
    if (current->info.id == id) {
      return current->wl_output;
    }
    current = current->next;
  }
  return NULL;
}
