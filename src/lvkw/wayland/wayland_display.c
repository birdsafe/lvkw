#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "lvkw_wayland_internal.h"

void _lvkw_wayland_update_opaque_region(LVKW_Window_WL *window) {
  if (window->flags & LVKW_WINDOW_TRANSPARENT) {
    wl_surface_set_opaque_region(window->wl.surface, NULL);
  }
  else {
    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
    struct wl_region *region = wl_compositor_create_region(ctx->protocols.wl_compositor);
    wl_region_add(region, 0, 0, (int32_t)window->size.width, (int32_t)window->size.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_region_destroy(region);
  }
}

LVKW_Event _lvkw_wayland_make_window_resized_event(LVKW_Window_WL *window) {
  LVKW_Event evt;
  evt.type = LVKW_EVENT_TYPE_WINDOW_RESIZED;
  evt.window = (LVKW_Window *)window;

  evt.resized.size = window->size;
  evt.resized.framebufferSize.width = (uint32_t)(window->size.width * window->scale);
  evt.resized.framebufferSize.height = (uint32_t)(window->size.height * window->scale);

  return evt;
}

LVKW_DecorationMode _lvkw_wayland_get_decoration_mode(void) {
  const char *env = getenv("LVKW_DECORATION_MODE");
  if (!env) return LVKW_DECORATION_MODE_AUTO;
  if (strcmp(env, "ssd") == 0) return LVKW_DECORATION_MODE_SSD;
  if (strcmp(env, "csd") == 0) return LVKW_DECORATION_MODE_CSD;
  if (strcmp(env, "none") == 0) return LVKW_DECORATION_MODE_NONE;
  return LVKW_DECORATION_MODE_AUTO;
}

/* wp_fractional_scale */

static void _fractional_scale_handle_preferred_scale(void *data, struct wp_fractional_scale_v1 *fractional_scale,
                                                     uint32_t scale) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;

  LVKW_WND_ASSUME(data, window != NULL, "Window handle must not be NULL in preferred scale handler");

  window->scale = scale / 120.0;

  // Trigger resize to update buffer size
  if (window->base.pub.is_ready) {
    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
    LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
    _lvkw_wayland_push_event(ctx, &evt);
  }
}

const struct wp_fractional_scale_v1_listener _lvkw_wayland_fractional_scale_listener = {
    .preferred_scale = _fractional_scale_handle_preferred_scale,
};

/* wl_surface */

static void _wl_surface_handle_enter(void *data, struct wl_surface *surface, struct wl_output *output) {}

static void _wl_surface_handle_leave(void *data, struct wl_surface *surface, struct wl_output *output) {}

static void _wl_surface_handle_preferred_buffer_scale(void *data, struct wl_surface *surface, int32_t factor) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;

  LVKW_WND_ASSUME(data, window != NULL, "Window handle must not be NULL in preferred buffer scale handler");

  // If fractional scale is active, ignore integer scale
  if (window->ext.fractional_scale) return;

  if (window->scale != (double)factor) {
    window->scale = (double)factor;

    // Trigger resize
    if (window->base.pub.is_ready) {
      LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
      LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
      _lvkw_wayland_push_event(ctx, &evt);
    }
  }
}

static void _wl_surface_handle_preferred_buffer_transform(void *data, struct wl_surface *surface, uint32_t transform) {}

const struct wl_surface_listener _lvkw_wayland_surface_listener = {
    .enter = _wl_surface_handle_enter,
    .leave = _wl_surface_handle_leave,
    .preferred_buffer_scale = _wl_surface_handle_preferred_buffer_scale,
    .preferred_buffer_transform = _wl_surface_handle_preferred_buffer_transform};

/* xdg_surface */

static void _xdg_surface_handle_configure(void *userData, struct xdg_surface *surface, uint32_t serial) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;

  LVKW_WND_ASSUME(userData, window != NULL, "Window handle must not be NULL in xdg surface configure handler");
  LVKW_WND_ASSUME(userData, surface != NULL, "XDG surface must not be NULL in configure handler");

  xdg_surface_ack_configure(surface, serial);

  if (!window->base.pub.is_ready) {
    window->base.pub.is_ready = true;

    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
    LVKW_Event evt = {.type = LVKW_EVENT_TYPE_WINDOW_READY, .window = (LVKW_Window *)window};
    _lvkw_wayland_push_event(ctx, &evt);
  }
}

const struct xdg_surface_listener _lvkw_wayland_xdg_surface_listener = {.configure = _xdg_surface_handle_configure};

/* xdg_toplevel */

static void _xdg_toplevel_handle_configure(void *userData, struct xdg_toplevel *toplevel, int32_t width, int32_t height,
                                           struct wl_array *states) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;

  LVKW_WND_ASSUME(userData, window != NULL, "Window handle must not be NULL in xdg toplevel configure handler");
  LVKW_WND_ASSUME(userData, toplevel != NULL, "XDG toplevel must not be NULL in configure handler");

  if (width == 0 && height == 0) {
    // Keep window as is.
  }
  else if ((uint32_t)width != window->size.width || (uint32_t)height != window->size.height) {
    window->size.width = (uint32_t)width;
    window->size.height = (uint32_t)height;
    _lvkw_wayland_update_opaque_region(window);
  }

  if (window->ext.viewport) {
    wp_viewport_set_destination(window->ext.viewport, (int)window->size.width, (int)window->size.height);
  }

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
  _lvkw_wayland_push_event(ctx, &evt);
}

static void _xdg_toplevel_handle_close(void *userData, struct xdg_toplevel *toplevel) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;

  LVKW_WND_ASSUME(userData, window != NULL, "Window handle must not be NULL in xdg toplevel close handler");
  LVKW_WND_ASSUME(userData, toplevel != NULL, "XDG toplevel must not be NULL in close handler");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  LVKW_Event evt = {.type = LVKW_EVENT_TYPE_CLOSE_REQUESTED, .window = (LVKW_Window *)window};
  _lvkw_wayland_push_event(ctx, &evt);
}

const struct xdg_toplevel_listener _lvkw_wayland_xdg_toplevel_listener = {.configure = _xdg_toplevel_handle_configure,
                                                                          .close = _xdg_toplevel_handle_close,
                                                                          .configure_bounds = NULL,
                                                                          .wm_capabilities = NULL};

/* xdg_decoration */

static void _xdg_decoration_handle_configure(void *data, struct zxdg_toplevel_decoration_v1 *decoration,
                                             uint32_t mode) {}

const struct zxdg_toplevel_decoration_v1_listener _lvkw_wayland_xdg_decoration_listener = {
    .configure = _xdg_decoration_handle_configure,
};

bool _lvkw_wayland_create_xdg_shell_objects(LVKW_Window_WL *window, const LVKW_WindowCreateInfo *create_info) {
  LVKW_WND_ASSUME(window, window != NULL, "Window handle must not be NULL in create_xdg_shell_objects");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_WND_ASSUME(window, ctx != NULL, "Context must not be NULL in create_xdg_shell_objects");
  LVKW_WND_ASSUME(window, window->wl.surface != NULL, "Wayland surface must not be NULL in create_xdg_shell_objects");

  LVKW_DecorationMode mode = _lvkw_wayland_get_decoration_mode();

  bool try_ssd = (mode == LVKW_DECORATION_MODE_AUTO || mode == LVKW_DECORATION_MODE_SSD);
  bool try_libdecor = (mode == LVKW_DECORATION_MODE_AUTO || mode == LVKW_DECORATION_MODE_CSD);

  if (try_ssd && ctx->protocols.opt.zxdg_decoration_manager_v1) {
    window->xdg.surface = xdg_wm_base_get_xdg_surface(ctx->protocols.xdg_wm_base, window->wl.surface);
    if (!window->xdg.surface) {
      LVKW_REPORT_WIND_DIAGNOSIS(window, LVKW_DIAGNOSIS_UNKNOWN, "xdg_wm_base_get_xdg_surface() failed");
      return false;
    }

    xdg_surface_add_listener(window->xdg.surface, &_lvkw_wayland_xdg_surface_listener, window);

    window->xdg.toplevel = xdg_surface_get_toplevel(window->xdg.surface);
    if (!window->xdg.toplevel) {
      LVKW_REPORT_WIND_DIAGNOSIS(window, LVKW_DIAGNOSIS_UNKNOWN, "xdg_surface_get_toplevel() failed");
      return false;
    }

    xdg_toplevel_add_listener(window->xdg.toplevel, &_lvkw_wayland_xdg_toplevel_listener, window);

    if (create_info->title) {
      xdg_toplevel_set_title(window->xdg.toplevel, create_info->title);
    }
    else {
      xdg_toplevel_set_title(window->xdg.toplevel, "Lvkw");
    }

    if (create_info->app_id) {
      xdg_toplevel_set_app_id(window->xdg.toplevel, create_info->app_id);
    }

    window->xdg.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
        ctx->protocols.opt.zxdg_decoration_manager_v1, window->xdg.toplevel);
    zxdg_toplevel_decoration_v1_add_listener(window->xdg.decoration, &_lvkw_wayland_xdg_decoration_listener, window);
    zxdg_toplevel_decoration_v1_set_mode(window->xdg.decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    window->decor_mode = LVKW_DECORATION_MODE_SSD;
  }
  else if (try_libdecor) {
    _lvkw_wayland_create_csd_frame(ctx, window, create_info);
  }

  if (window->decor_mode == LVKW_DECORATION_MODE_AUTO) {
    // Fallback to no decorations (raw xdg_shell)
    window->xdg.surface = xdg_wm_base_get_xdg_surface(ctx->protocols.xdg_wm_base, window->wl.surface);
    if (!window->xdg.surface) {
      LVKW_REPORT_WIND_DIAGNOSIS(window, LVKW_DIAGNOSIS_UNKNOWN, "xdg_wm_base_get_xdg_surface() failed");
      return false;
    }

    xdg_surface_add_listener(window->xdg.surface, &_lvkw_wayland_xdg_surface_listener, window);

    window->xdg.toplevel = xdg_surface_get_toplevel(window->xdg.surface);
    if (!window->xdg.toplevel) {
      LVKW_REPORT_WIND_DIAGNOSIS(window, LVKW_DIAGNOSIS_UNKNOWN, "xdg_surface_get_toplevel() failed");
      return false;
    }

    xdg_toplevel_add_listener(window->xdg.toplevel, &_lvkw_wayland_xdg_toplevel_listener, window);

    if (create_info->title) {
      xdg_toplevel_set_title(window->xdg.toplevel, create_info->title);
    }
    else {
      xdg_toplevel_set_title(window->xdg.toplevel, "Lvkw");
    }

    if (create_info->app_id) {
      xdg_toplevel_set_app_id(window->xdg.toplevel, create_info->app_id);
    }

    window->decor_mode = LVKW_DECORATION_MODE_NONE;
  }

  if (ctx->protocols.opt.wp_viewporter) {
    window->ext.viewport = wp_viewporter_get_viewport(ctx->protocols.opt.wp_viewporter, window->wl.surface);
  }

  if (ctx->protocols.opt.wp_fractional_scale_manager_v1) {
    window->ext.fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(
        ctx->protocols.opt.wp_fractional_scale_manager_v1, window->wl.surface);
    wp_fractional_scale_v1_add_listener(window->ext.fractional_scale, &_lvkw_wayland_fractional_scale_listener, window);
  }

  if (ctx->protocols.opt.zwp_idle_inhibit_manager_v1) {
    window->ext.idle_inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(
        ctx->protocols.opt.zwp_idle_inhibit_manager_v1, window->wl.surface);
  }

  return true;
}