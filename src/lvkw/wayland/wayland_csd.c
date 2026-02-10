#include "wayland_csd.h"

#include "dlib/libdecor.h"
#include "lvkw_wayland_internal.h"
#include "viewporter-client-protocol.h"

static void _libdecor_handle_diagnosis(struct libdecor *context, enum libdecor_error error, const char *message) {
  (void)context;
  (void)error;
  (void)message;
  // No context available for reporting, and LKVM_ERR_SCOPE_NULL was a no-op
  // anyway.
}

static struct libdecor_interface _libdecor_interface = {
    .error = _libdecor_handle_diagnosis,
};

static void _libdecor_frame_handle_configure(struct libdecor_frame *frame, struct libdecor_configuration *configuration,
                                             void *userdata) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userdata;

  LVKW_WND_ASSUME(userdata, window != NULL, "Window handle must not be NULL in libdecor configure handler");

  int width, height;
  if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height)) {
    width = (int)window->size.width;
    height = (int)window->size.height;
  }

  if ((uint32_t)width != window->size.width || (uint32_t)height != window->size.height) {
    window->size.width = (uint32_t)width;
    window->size.height = (uint32_t)height;
    _lvkw_wayland_update_opaque_region(window);
  }

  if (window->ext.viewport) {
    wp_viewport_set_destination(window->ext.viewport, (int)window->size.width, (int)window->size.height);
  }

  struct libdecor_state *state = libdecor_state_new(width, height);
  libdecor_frame_commit(frame, state, configuration);
  libdecor_state_free(state);

  if (!window->base.pub.is_ready) {
    window->base.pub.is_ready = true;

    LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
    LVKW_Event evt = {.type = LVKW_EVENT_TYPE_WINDOW_READY};
    evt.window_ready.window = (LVKW_Window *)window;
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
  LVKW_Event evt = {.type = LVKW_EVENT_TYPE_CLOSE_REQUESTED};
  evt.close_requested.window = (LVKW_Window *)window;
  _lvkw_wayland_push_event(ctx, &evt);
}

static void _libdecor_frame_handle_commit(struct libdecor_frame *frame, void *userdata) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userdata;

  LVKW_WND_ASSUME(userdata, window != NULL, "Window handle must not be NULL in libdecor commit handler");

  wl_surface_commit(window->wl.surface);
}

static void _libdecor_frame_handle_dismiss_popup(struct libdecor_frame *frame, const char *seat_name, void *userdata) {
}

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
  }

  if (!ctx->libdecor.ctx) return false;

  window->libdecor.frame = libdecor_decorate(ctx->libdecor.ctx, window->wl.surface, &_libdecor_frame_interface, window);

  if (create_info->title) {
    libdecor_frame_set_title(window->libdecor.frame, create_info->title);
  }
  else {
    libdecor_frame_set_title(window->libdecor.frame, "Lvkw");
  }

  if (create_info->app_id) {
    libdecor_frame_set_app_id(window->libdecor.frame, create_info->app_id);
  }

  libdecor_frame_map(window->libdecor.frame);

  window->xdg.surface = libdecor_frame_get_xdg_surface(window->libdecor.frame);
  window->xdg.toplevel = libdecor_frame_get_xdg_toplevel(window->libdecor.frame);
  window->decor_mode = LVKW_DECORATION_MODE_CSD;

  return true;
}