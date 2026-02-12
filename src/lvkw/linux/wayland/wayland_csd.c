#include "wayland_csd.h"

#include "dlib/libdecor.h"
#include "lvkw_wayland_internal.h"
#include "viewporter-client-protocol.h"

static void _libdecor_handle_error(struct libdecor *context, enum libdecor_error error, const char *message) {
  LVKW_Context_WL *ctx = NULL;
  if (lvkw_lib_decor.opt.get_userdata) {
    ctx = (LVKW_Context_WL *)libdecor_get_userdata(context);
  }
  LVKW_REPORT_CTX_DIAGNOSTIC(ctx, LVKW_DIAGNOSTIC_INTERNAL, message);
}

static struct libdecor_interface _libdecor_interface = {
    .error = _libdecor_handle_error,
};

static void _libdecor_frame_handle_configure(struct libdecor_frame *frame, struct libdecor_configuration *configuration,
                                             void *userdata) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userdata;

  LVKW_WND_ASSUME(userdata, window != NULL, "Window handle must not be NULL in libdecor configure handler");

  enum libdecor_window_state state_flags;
  if (libdecor_configuration_get_window_state(configuration, &state_flags)) {
    bool maximized = (state_flags & LIBDECOR_WINDOW_STATE_MAXIMIZED) != 0;
    bool focused = (state_flags & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;

    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

    if (window->is_maximized != maximized) {
      window->is_maximized = maximized;
      if (maximized)
        window->base.pub.flags |= LVKW_WND_STATE_MAXIMIZED;
      else
        window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_MAXIMIZED;

      LVKW_Event evt = {.type = LVKW_EVENT_TYPE_WINDOW_MAXIMIZED, .window = (LVKW_Window *)window};
      evt.maximized.maximized = maximized;
      _lvkw_wayland_push_event(ctx, &evt);
    }

    bool old_focused = (window->base.pub.flags & LVKW_WND_STATE_FOCUSED) != 0;
    if (old_focused != focused) {
      if (focused)
        window->base.pub.flags |= LVKW_WND_STATE_FOCUSED;
      else
        window->base.pub.flags &= (uint32_t)~LVKW_WND_STATE_FOCUSED;

      LVKW_Event evt = {.type = LVKW_EVENT_TYPE_FOCUS, .window = (LVKW_Window *)window};
      evt.focus.focused = focused;
      _lvkw_wayland_push_event(ctx, &evt);
    }
  }

  int width, height;
  if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height)) {
    width = (int)window->size.x;
    height = (int)window->size.y;
  }

  if ((uint32_t)width != window->size.x || (uint32_t)height != window->size.y) {
    window->size.x = (uint32_t)width;
    window->size.y = (uint32_t)height;
    _lvkw_wayland_update_opaque_region(window);
  }

  if (window->ext.viewport) {
    wp_viewport_set_destination(window->ext.viewport, (int)window->size.x, (int)window->size.y);
  }

  struct libdecor_state *state = libdecor_state_new(width, height);
  libdecor_frame_commit(frame, state, configuration);
  libdecor_state_free(state);

  if (!(window->base.pub.flags & LVKW_WND_STATE_READY)) {
    window->base.pub.flags |= LVKW_WND_STATE_READY;

    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
    LVKW_Event evt = {.type = LVKW_EVENT_TYPE_WINDOW_READY, .window = (LVKW_Window *)window};
    _lvkw_wayland_push_event(ctx, &evt);
  }

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
  _lvkw_wayland_push_event(ctx, &evt);
}

static void _libdecor_frame_handle_close(struct libdecor_frame *frame, void *userdata) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userdata;

  LVKW_WND_ASSUME(userdata, window != NULL, "Window handle must not be NULL in libdecor close handler");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  LVKW_Event evt = {.type = LVKW_EVENT_TYPE_CLOSE_REQUESTED, .window = (LVKW_Window *)window};
  _lvkw_wayland_push_event(ctx, &evt);
}

static void _libdecor_frame_handle_commit(struct libdecor_frame *frame, void *userdata) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userdata;

  LVKW_WND_ASSUME(userdata, window != NULL, "Window handle must not be NULL in libdecor commit handler");

  wl_surface_commit(window->wl.surface);
}

static void _libdecor_frame_handle_dismiss_popup(struct libdecor_frame *frame, const char *seat_name, void *userdata) {}

static struct libdecor_frame_interface _libdecor_frame_interface = {
    .configure = _libdecor_frame_handle_configure,
    .close = _libdecor_frame_handle_close,
    .commit = _libdecor_frame_handle_commit,
    .dismiss_popup = _libdecor_frame_handle_dismiss_popup,
};

bool _lvkw_wayland_create_csd_frame(LVKW_Context_WL *ctx, LVKW_Window_WL *window,
                                    const LVKW_WindowCreateInfo *create_info) {
  if (!lvkw_lib_decor.base.available) return false;

  if (!ctx->libdecor.ctx) {
    ctx->libdecor.ctx = libdecor_new(ctx->wl.display, &_libdecor_interface);
    if (ctx->libdecor.ctx && lvkw_lib_decor.opt.set_userdata) {
      libdecor_set_userdata(ctx->libdecor.ctx, ctx);
    }
  }

  if (!ctx->libdecor.ctx) return false;

  window->libdecor.frame = libdecor_decorate(ctx->libdecor.ctx, window->wl.surface, &_libdecor_frame_interface, window);

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

  window->xdg.surface = libdecor_frame_get_xdg_surface(window->libdecor.frame);
  window->xdg.toplevel = libdecor_frame_get_xdg_toplevel(window->libdecor.frame);
  window->decor_mode = LVKW_DECORATION_MODE_CSD;

  return true;
}