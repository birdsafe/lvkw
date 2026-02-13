#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "dlib/libdecor.h"
#include "lvkw_assume.h"
#include "lvkw_wayland_internal.h"

void _lvkw_wayland_update_opaque_region(LVKW_Window_WL *window) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  // Opaque region
  if (window->transparent) {
    wl_surface_set_opaque_region(window->wl.surface, NULL);
  }
  else {
    struct wl_region *region = wl_compositor_create_region(ctx->protocols.wl_compositor);
    wl_region_add(region, 0, 0, (int32_t)window->size.x, (int32_t)window->size.y);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_region_destroy(region);
  }

  // Input region & Window Geometry
  struct wl_region *input_region = wl_compositor_create_region(ctx->protocols.wl_compositor);
  wl_region_add(input_region, 0, 0, (int32_t)window->size.x, (int32_t)window->size.y);
  wl_surface_set_input_region(window->wl.surface, input_region);
  wl_region_destroy(input_region);

  if (window->xdg.surface) {
    xdg_surface_set_window_geometry(window->xdg.surface, 0, 0, (int)window->size.x, (int)window->size.y);
  }
}

LVKW_Event _lvkw_wayland_make_window_resized_event(LVKW_Window_WL *window) {
  LVKW_Event evt;

  evt.resized.geometry.logicalSize = window->size;
  evt.resized.geometry.pixelSize.x = (int32_t)(window->size.x * window->scale);
  evt.resized.geometry.pixelSize.y = (int32_t)(window->size.y * window->scale);
  return evt;
}

LVKW_WaylandDecorationMode _lvkw_wayland_get_decoration_mode(const LVKW_ContextCreateInfo *create_info) {
  return create_info->tuning->wayland.decoration_mode;
}

/* wp_fractional_scale */

static void _fractional_scale_handle_preferred_scale(void *data, struct wp_fractional_scale_v1 *fractional_scale,
                                                     uint32_t scale) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;

  LVKW_WND_ASSUME(data, window != NULL, "Window handle must not be NULL in preferred scale handler");

  window->scale = scale / 120.0;

  wl_surface_set_buffer_scale(window->wl.surface, 1);

  // Trigger resize to update buffer size
  if (window->base.pub.flags & LVKW_WND_STATE_READY) {
    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
    LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
    _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_WINDOW_RESIZED, window, &evt);
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

    wl_surface_set_buffer_scale(window->wl.surface, factor);

    // Trigger resize
    if (window->base.pub.flags & LVKW_WND_STATE_READY) {
      LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
      LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
      _lvkw_wayland_push_event(ctx,LVKW_EVENT_TYPE_WINDOW_RESIZED, window, &evt);
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

  if (!(window->base.pub.flags & LVKW_WND_STATE_READY)) {
    window->base.pub.flags |= LVKW_WND_STATE_READY;

    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
    LVKW_Event evt = {0};
    _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_WINDOW_READY, window, &evt);
  }
}

const struct xdg_surface_listener _lvkw_wayland_xdg_surface_listener = {.configure = _xdg_surface_handle_configure};

/* xdg_toplevel */

static void _xdg_toplevel_handle_configure(void *userData, struct xdg_toplevel *toplevel, int32_t width, int32_t height,
                                           struct wl_array *states) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;

  LVKW_WND_ASSUME(userData, window != NULL, "Window handle must not be NULL in xdg toplevel configure handler");
  LVKW_WND_ASSUME(userData, toplevel != NULL, "XDG toplevel must not be NULL in configure handler");

  bool maximized = false;
  bool focused = false;
  uint32_t *state;
  wl_array_for_each(state, states) {
    if (*state == XDG_TOPLEVEL_STATE_MAXIMIZED) maximized = true;
    if (*state == XDG_TOPLEVEL_STATE_ACTIVATED) focused = true;
  }

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (window->is_maximized != maximized) {
    window->is_maximized = maximized;
    if (maximized)
      window->base.pub.flags |= LVKW_WND_STATE_MAXIMIZED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_MAXIMIZED;

    LVKW_Event evt = {0};
    evt.maximized.maximized = maximized;
    _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_WINDOW_MAXIMIZED, window, &evt);
  }

  bool old_focused = (window->base.pub.flags & LVKW_WND_STATE_FOCUSED) != 0;
  if (old_focused != focused) {
    if (focused)
      window->base.pub.flags |= LVKW_WND_STATE_FOCUSED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_FOCUSED;

    LVKW_Event evt = {0};
    evt.focus.focused = focused;
    _lvkw_wayland_push_event(ctx,LVKW_EVENT_TYPE_FOCUS, window, &evt);
  }

  bool size_changed = false;
  if (width == 0 && height == 0) {
    // Keep window as is.
  }
  else if ((uint32_t)width != window->size.x || (uint32_t)height != window->size.y) {
    window->size.x = (uint32_t)width;
    window->size.y = (uint32_t)height;
    _lvkw_wayland_update_opaque_region(window);
    size_changed = true;
  }

  if (size_changed || !(window->base.pub.flags & LVKW_WND_STATE_READY)) {
    LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
    _lvkw_wayland_push_event(ctx,LVKW_EVENT_TYPE_WINDOW_RESIZED, window, &evt);
  }
}

static void _xdg_toplevel_handle_close(void *userData, struct xdg_toplevel *toplevel) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;

  LVKW_WND_ASSUME(userData, window != NULL, "Window handle must not be NULL in xdg toplevel close handler");
  LVKW_WND_ASSUME(userData, toplevel != NULL, "XDG toplevel must not be NULL in close handler");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  LVKW_Event evt = {0};
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_CLOSE_REQUESTED, window, &evt);
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

/* libdecor */

static void _libdecor_frame_handle_configure(struct libdecor_frame *frame, struct libdecor_configuration *configuration,
                                             void *user_data) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)user_data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  int width, height;

  enum libdecor_window_state state_flags;
  bool maximized = false;
  bool focused = false;

  if (libdecor_configuration_get_window_state(configuration, &state_flags)) {
    if (state_flags & LIBDECOR_WINDOW_STATE_MAXIMIZED) maximized = true;
    if (state_flags & LIBDECOR_WINDOW_STATE_ACTIVE) focused = true;
  }

  if (window->is_maximized != maximized) {
    window->is_maximized = maximized;
    if (maximized)
      window->base.pub.flags |= LVKW_WND_STATE_MAXIMIZED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_MAXIMIZED;

    LVKW_Event evt = {0};
    evt.maximized.maximized = maximized;
    _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_WINDOW_MAXIMIZED, window, &evt);
  }

  bool old_focused = (window->base.pub.flags & LVKW_WND_STATE_FOCUSED) != 0;
  if (old_focused != focused) {
    if (focused)
      window->base.pub.flags |= LVKW_WND_STATE_FOCUSED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_FOCUSED;

    LVKW_Event evt = {0};
    evt.focus.focused = focused;
    _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_FOCUS, window, &evt);
  }

  if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height)) {
    width = (int)window->size.x;
    height = (int)window->size.y;
  }

  window->size.x = (uint32_t)width;
  window->size.y = (uint32_t)height;

  struct libdecor_state *state = libdecor_state_new(width, height);
  libdecor_frame_commit(frame, state, configuration);
  libdecor_state_free(state);

  _lvkw_wayland_update_opaque_region(window);

  if (!(window->base.pub.flags & LVKW_WND_STATE_READY)) {
    window->base.pub.flags |= LVKW_WND_STATE_READY;
    LVKW_Event evt = {0};
    _lvkw_wayland_push_event(ctx,  LVKW_EVENT_TYPE_WINDOW_READY, window, &evt);
  }

  LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_WINDOW_RESIZED, window, &evt);
}

static void _libdecor_frame_handle_close(struct libdecor_frame *frame, void *user_data) {
  (void)frame;
  LVKW_Window_WL *window = (LVKW_Window_WL *)user_data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_Event evt = {0};
  _lvkw_wayland_push_event(ctx, LVKW_EVENT_TYPE_CLOSE_REQUESTED, window, &evt);
}

static void _libdecor_frame_handle_commit(struct libdecor_frame *frame, void *user_data) {
  (void)frame;
  LVKW_Window_WL *window = (LVKW_Window_WL *)user_data;
  wl_surface_commit(window->wl.surface);
}

static struct libdecor_frame_interface _libdecor_frame_interface = {
    .configure = _libdecor_frame_handle_configure,
    .close = _libdecor_frame_handle_close,
    .commit = _libdecor_frame_handle_commit,
};

bool _lvkw_wayland_create_xdg_shell_objects(LVKW_Window_WL *window, const LVKW_WindowCreateInfo *create_info) {
  LVKW_WND_ASSUME(window, window != NULL, "Window handle must not be NULL in create_xdg_shell_objects");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_WND_ASSUME(window, ctx != NULL, "Context must not be NULL in create_xdg_shell_objects");
  LVKW_WND_ASSUME(window, window->wl.surface != NULL, "Wayland surface must not be NULL in create_xdg_shell_objects");

  LVKW_WaylandDecorationMode mode = ctx->decoration_mode;

  bool try_ssd = (mode == LVKW_WAYLAND_DECORATION_MODE_AUTO || mode == LVKW_WAYLAND_DECORATION_MODE_SSD);
  bool try_csd = (mode == LVKW_WAYLAND_DECORATION_MODE_AUTO || mode == LVKW_WAYLAND_DECORATION_MODE_CSD);

  if (try_ssd && ctx->protocols.opt.zxdg_decoration_manager_v1) {
    window->xdg.surface = xdg_wm_base_get_xdg_surface(ctx->protocols.xdg_wm_base, window->wl.surface);
    if (!window->xdg.surface) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN, "xdg_wm_base_get_xdg_surface() failed");
      return false;
    }

    xdg_surface_add_listener(window->xdg.surface, &_lvkw_wayland_xdg_surface_listener, window);

    window->xdg.toplevel = xdg_surface_get_toplevel(window->xdg.surface);
    if (!window->xdg.toplevel) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN, "xdg_surface_get_toplevel() failed");
      return false;
    }

    xdg_toplevel_add_listener(window->xdg.toplevel, &_lvkw_wayland_xdg_toplevel_listener, window);

    if (create_info->attributes.title) {
      xdg_toplevel_set_title(window->xdg.toplevel, create_info->attributes.title);
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

    if (create_info->attributes.fullscreen) {
      struct wl_output *target_output = _lvkw_wayland_find_monitor(ctx, create_info->attributes.monitor);
      xdg_toplevel_set_fullscreen(window->xdg.toplevel, target_output);
    }
    else if (create_info->attributes.maximized) {
      xdg_toplevel_set_maximized(window->xdg.toplevel);
    }

    window->decor_mode = LVKW_WAYLAND_DECORATION_MODE_SSD;
  }

  if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_AUTO && try_csd && ctx->libdecor.ctx) {
    window->libdecor.frame =
        libdecor_decorate(ctx->libdecor.ctx, window->wl.surface, &_libdecor_frame_interface, window);

    if (window->libdecor.frame) {
      if (create_info->attributes.title) {
        libdecor_frame_set_title(window->libdecor.frame, create_info->attributes.title);
      }
      else {
        libdecor_frame_set_title(window->libdecor.frame, "Lvkw");
      }

      if (create_info->app_id) {
        libdecor_frame_set_app_id(window->libdecor.frame, create_info->app_id);
      }

      if (create_info->attributes.fullscreen) {
        struct wl_output *target_output = _lvkw_wayland_find_monitor(ctx, create_info->attributes.monitor);
        libdecor_frame_set_fullscreen(window->libdecor.frame, target_output);
      }
      else if (create_info->attributes.maximized) {
        libdecor_frame_set_maximized(window->libdecor.frame);
      }

      libdecor_frame_map(window->libdecor.frame);
      window->decor_mode = LVKW_WAYLAND_DECORATION_MODE_CSD;
    }
  }

  if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_AUTO) {
    // Fallback to no decorations (raw xdg_shell)
    window->xdg.surface = xdg_wm_base_get_xdg_surface(ctx->protocols.xdg_wm_base, window->wl.surface);
    if (!window->xdg.surface) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN, "xdg_wm_base_get_xdg_surface() failed");
      return false;
    }

    xdg_surface_add_listener(window->xdg.surface, &_lvkw_wayland_xdg_surface_listener, window);

    window->xdg.toplevel = xdg_surface_get_toplevel(window->xdg.surface);
    if (!window->xdg.toplevel) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN, "xdg_surface_get_toplevel() failed");
      return false;
    }

    xdg_toplevel_add_listener(window->xdg.toplevel, &_lvkw_wayland_xdg_toplevel_listener, window);

    if (create_info->attributes.title) {
      xdg_toplevel_set_title(window->xdg.toplevel, create_info->attributes.title);
    }
    else {
      xdg_toplevel_set_title(window->xdg.toplevel, "Lvkw");
    }

    if (create_info->app_id) {
      xdg_toplevel_set_app_id(window->xdg.toplevel, create_info->app_id);
    }

    if (create_info->attributes.fullscreen) {
      struct wl_output *target_output = _lvkw_wayland_find_monitor(ctx, create_info->attributes.monitor);
      xdg_toplevel_set_fullscreen(window->xdg.toplevel, target_output);
    }
    else if (create_info->attributes.maximized) {
      xdg_toplevel_set_maximized(window->xdg.toplevel);
    }

    window->decor_mode = LVKW_WAYLAND_DECORATION_MODE_NONE;
  }

  if (ctx->protocols.opt.wp_viewporter) {
    window->ext.viewport = wp_viewporter_get_viewport(ctx->protocols.opt.wp_viewporter, window->wl.surface);
  }

  if (ctx->protocols.opt.wp_fractional_scale_manager_v1) {
    window->ext.fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(
        ctx->protocols.opt.wp_fractional_scale_manager_v1, window->wl.surface);
    wp_fractional_scale_v1_add_listener(window->ext.fractional_scale, &_lvkw_wayland_fractional_scale_listener, window);
  }

  _lvkw_wayland_update_opaque_region(window);

  if (ctx->inhibit_idle && ctx->protocols.opt.zwp_idle_inhibit_manager_v1) {
    window->ext.idle_inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(
        ctx->protocols.opt.zwp_idle_inhibit_manager_v1, window->wl.surface);
  }

  return true;
}