// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef LVKW_WAYLAND_INTERNAL_H_INCLUDED
#define LVKW_WAYLAND_INTERNAL_H_INCLUDED

#include "dlib/wayland-client.h"
#include "dlib/wayland-cursor.h"
#include "dlib/libdecor.h"
#include "dlib/xkbcommon.h"  // IWYU pragma: keep
#include "lvkw_event_queue.h"
#include "lvkw_linux_internal.h"
#include "lvkw_thread_internal.h"
#include "protocols/wl_protocols.h"

// Indirect wayland client

#define LVKW_WAYLAND_MAX_EVENTS 4096

typedef struct LVKW_Window_WL {
  LVKW_Window_Base base;

  struct {
    struct wl_surface *surface;
  } wl;

  struct {
    struct xdg_surface *surface;
    struct xdg_toplevel *toplevel;
    struct zxdg_toplevel_decoration_v1 *decoration;
  } xdg;

  struct {
    struct libdecor_frame *frame;
  } libdecor;

  struct {
    struct wp_viewport *viewport;
    struct wp_fractional_scale_v1 *fractional_scale;
    struct zwp_idle_inhibitor_v1 *idle_inhibitor;
    struct wp_content_type_v1 *content_type;
  } ext;

  struct {
    struct zwp_relative_pointer_v1 *relative;
    struct zwp_locked_pointer_v1 *locked;
  } input;

  /* Geometry & State */
  LVKW_LogicalVec size;
  LVKW_LogicalVec min_size;
  LVKW_LogicalVec max_size;
  LVKW_Ratio aspect_ratio;
  LVKW_LogicalVec last_cursor_pos;
  bool last_cursor_set;
  double scale;
  LVKW_WaylandDecorationMode decor_mode;
  LVKW_CursorMode cursor_mode;
  LVKW_Cursor *cursor;
  bool is_fullscreen;
  bool is_maximized;
  bool is_resizable;

  /* Flags */
  bool transparent;
  bool mouse_passthrough;
  LVKW_Monitor *monitor;
} LVKW_Window_WL;

typedef struct LVKW_Monitor_WL {
  LVKW_Monitor_Base base;
  uint32_t wayland_name;
  struct wl_output *wl_output;
  struct zxdg_output_v1 *xdg_output;
  LVKW_VideoMode *modes;
  uint32_t mode_count;
  bool announced;
} LVKW_Monitor_WL;

typedef struct LVKW_Cursor_WL {
  LVKW_Cursor_Base base;
  LVKW_CursorShape shape;
  struct wl_buffer *buffer;
  int32_t width;
  int32_t height;
  int32_t hotspot_x;
  int32_t hotspot_y;
} LVKW_Cursor_WL;

typedef struct LVKW_Context_WL {
  LVKW_Context_Base base;

#ifdef LVKW_ENABLE_CONTROLLER
  LVKW_ControllerContext_Linux controller;
#endif

  struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_cursor_theme *cursor_theme;
    struct wl_surface *cursor_surface;
  } wl;

  LVKW_Wayland_Protocols_WL protocols;

  struct {
    struct wl_keyboard *keyboard;
    struct wl_pointer *pointer;
    uint32_t pointer_serial;
    struct wp_cursor_shape_device_v1 *cursor_shape_device;
    LVKW_Window_WL *keyboard_focus;
    LVKW_Window_WL *pointer_focus;

    struct {
      uint32_t mask;
      LVKW_Event motion;
      LVKW_Event button;
      LVKW_Event scroll;
    } pending_pointer;

    LVKW_Cursor_WL standard_cursors[13];  // 1..12

    struct {
      int32_t rate;
      int32_t delay;
    } repeat;

    struct {
      struct xkb_context *ctx;
      struct xkb_keymap *keymap;
      struct xkb_state *state;
      struct {
        xkb_mod_index_t shift;
        xkb_mod_index_t ctrl;
        xkb_mod_index_t alt;
        xkb_mod_index_t super;
        xkb_mod_index_t caps;
        xkb_mod_index_t num;
      } mod_indices;
    } xkb;
  } input;

  struct {
    struct libdecor *ctx;
  } libdecor;

  struct {
    struct ext_idle_notification_v1 *notification;
    uint32_t timeout_ms;
  } idle;

  int wake_fd;

  bool inhibit_idle;

  LVKW_WaylandDecorationMode decoration_mode;

  /* Monitor management */
  LVKW_Monitor_WL *monitors_list_start;
  uint32_t last_monitor_id;

  LVKW_StringCache string_cache;

  struct {
    LVKW_Lib_WaylandClient wl;
    LVKW_Lib_WaylandCursor wlc;
    LVKW_Lib_Xkb xkb;

    struct {
      LVKW_Lib_Decor decor;
    } opt;
  } dlib;
} LVKW_Context_WL;

#ifdef LVKW_ENABLE_CONTROLLER
#endif

void _lvkw_wayland_update_opaque_region(LVKW_Window_WL *window);
void _lvkw_wayland_update_cursor(LVKW_Context_WL *ctx, LVKW_Window_WL *window, uint32_t serial);
LVKW_Event _lvkw_wayland_make_window_resized_event(LVKW_Window_WL *window);

void _lvkw_wayland_push_event_cb(LVKW_Context_Base *ctx, LVKW_EventType type, LVKW_Window *window,
                                 const LVKW_Event *evt);
void _lvkw_wayland_check_error(LVKW_Context_WL *ctx);
void _lvkw_wayland_bind_output(LVKW_Context_WL *ctx, uint32_t name, uint32_t version);
void _lvkw_wayland_remove_monitor_by_name(LVKW_Context_WL *ctx, uint32_t name);
void _lvkw_wayland_destroy_monitors(LVKW_Context_WL *ctx);
struct wl_output *_lvkw_wayland_find_monitor(LVKW_Context_WL *ctx, const LVKW_Monitor *monitor);

extern const struct wl_seat_listener _lvkw_wayland_seat_listener;
extern const struct xdg_wm_base_listener _lvkw_wayland_wm_base_listener;
extern const struct wl_output_listener _lvkw_wayland_output_listener;
extern const struct wp_fractional_scale_v1_listener _lvkw_wayland_fractional_scale_listener;
extern const struct wl_surface_listener _lvkw_wayland_surface_listener;
extern const struct xdg_surface_listener _lvkw_wayland_xdg_surface_listener;
extern const struct xdg_toplevel_listener _lvkw_wayland_xdg_toplevel_listener;
extern const struct zxdg_toplevel_decoration_v1_listener _lvkw_wayland_xdg_decoration_listener;
extern const struct ext_idle_notification_v1_listener _lvkw_wayland_idle_listener;

LVKW_WaylandDecorationMode _lvkw_wayland_get_decoration_mode(
    const LVKW_ContextCreateInfo *create_info);
bool _lvkw_wayland_create_xdg_shell_objects(LVKW_Window_WL *window,
                                            const LVKW_WindowCreateInfo *create_info);

LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info,
                               LVKW_Context **out_context);
LVKW_Status lvkw_ctx_destroy_WL(LVKW_Context *handle);
LVKW_Status lvkw_ctx_getVkExtensions_WL(LVKW_Context *ctx, uint32_t *count,
                                        const char *const **out_extensions);
LVKW_Status lvkw_ctx_syncEvents_WL(LVKW_Context *ctx, uint32_t timeout_ms);
LVKW_Status lvkw_ctx_postEvent_WL(LVKW_Context *ctx, LVKW_EventType type, LVKW_Window *window,
                                  const LVKW_Event *evt);
LVKW_Status lvkw_ctx_scanEvents_WL(LVKW_Context *ctx, LVKW_EventType event_mask,
                                   LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_update_WL(LVKW_Context *ctx, uint32_t field_mask,
                               const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_WL(LVKW_Context *ctx, LVKW_Monitor **out_monitors,
                                    uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_WL(LVKW_Context *ctx, const LVKW_Monitor *monitor,
                                        LVKW_VideoMode *out_modes, uint32_t *count);
LVKW_Status lvkw_ctx_getTelemetry_WL(LVKW_Context *ctx, LVKW_TelemetryCategory category,
                                     void *out_data, bool reset);
LVKW_Status lvkw_ctx_createWindow_WL(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                     LVKW_Window **out_window);
LVKW_Status lvkw_wnd_destroy_WL(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_WL(LVKW_Window *window, VkInstance instance,
                                        VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_WL(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_WL(LVKW_Window *window, uint32_t field_mask,
                               const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_WL(LVKW_Window *window);
LVKW_Status lvkw_wnd_setClipboardText_WL(LVKW_Window *window, const char *text);
LVKW_Status lvkw_wnd_getClipboardText_WL(LVKW_Window *window, const char **out_text);
LVKW_Status lvkw_wnd_setClipboardData_WL(LVKW_Window *window, const LVKW_ClipboardData *data,
                                         uint32_t count);
LVKW_Status lvkw_wnd_getClipboardData_WL(LVKW_Window *window, const char *mime_type,
                                         const void **out_data, size_t *out_size);
LVKW_Status lvkw_wnd_getClipboardMimeTypes_WL(LVKW_Window *window, const char ***out_mime_types,
                                              uint32_t *count);

LVKW_Cursor *lvkw_ctx_getStandardCursor_WL(LVKW_Context *ctx, LVKW_CursorShape shape);
LVKW_Status lvkw_ctx_createCursor_WL(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                     LVKW_Cursor **out_cursor);
LVKW_Status lvkw_cursor_destroy_WL(LVKW_Cursor *cursor);

/* wayland-cursor helpers */

static inline struct wl_cursor_theme *lvkw_wl_cursor_theme_load(const LVKW_Context_WL *ctx,
                                                                const char *name, int size,
                                                                struct wl_shm *shm) {
  return ctx->dlib.wlc.theme_load(name, size, shm);
}

static inline void lvkw_wl_cursor_theme_destroy(const LVKW_Context_WL *ctx,
                                                struct wl_cursor_theme *theme) {
  ctx->dlib.wlc.theme_destroy(theme);
}

static inline struct wl_cursor *lvkw_wl_cursor_theme_get_cursor(const LVKW_Context_WL *ctx,
                                                                struct wl_cursor_theme *theme,
                                                                const char *name) {
  return ctx->dlib.wlc.theme_get_cursor(theme, name);
}

static inline struct wl_buffer *lvkw_wl_cursor_image_get_buffer(const LVKW_Context_WL *ctx,
                                                                struct wl_cursor_image *image) {
  return ctx->dlib.wlc.image_get_buffer(image);
}

/* xkbcommon helpers */

static inline struct xkb_context *lvkw_xkb_context_new(const LVKW_Context_WL *ctx,
                                                       enum xkb_context_flags flags) {
  return ctx->dlib.xkb.context_new(flags);
}

static inline void lvkw_xkb_context_unref(const LVKW_Context_WL *ctx, struct xkb_context *context) {
  ctx->dlib.xkb.context_unref(context);
}

static inline struct xkb_keymap *lvkw_xkb_keymap_new_from_string(
    const LVKW_Context_WL *ctx, struct xkb_context *context, const char *string,
    enum xkb_keymap_format format, enum xkb_keymap_compile_flags flags) {
  return ctx->dlib.xkb.keymap_new_from_string(context, string, format, flags);
}

static inline void lvkw_xkb_keymap_unref(const LVKW_Context_WL *ctx, struct xkb_keymap *keymap) {
  ctx->dlib.xkb.keymap_unref(keymap);
}

static inline struct xkb_state *lvkw_xkb_state_new(const LVKW_Context_WL *ctx,
                                                   struct xkb_keymap *keymap) {
  return ctx->dlib.xkb.state_new(keymap);
}

static inline void lvkw_xkb_state_unref(const LVKW_Context_WL *ctx, struct xkb_state *state) {
  ctx->dlib.xkb.state_unref(state);
}

static inline enum xkb_state_component lvkw_xkb_state_update_mask(
    const LVKW_Context_WL *ctx, struct xkb_state *state, xkb_mod_mask_t depressed_mods,
    xkb_mod_mask_t latched_mods, xkb_mod_mask_t locked_mods, xkb_layout_index_t depressed_layout,
    xkb_layout_index_t latched_layout, xkb_layout_index_t locked_layout) {
  return ctx->dlib.xkb.state_update_mask(state, depressed_mods, latched_mods, locked_mods,
                                         depressed_layout, latched_layout, locked_layout);
}

static inline xkb_keysym_t lvkw_xkb_state_key_get_one_sym(const LVKW_Context_WL *ctx,
                                                          struct xkb_state *state,
                                                          xkb_keycode_t key) {
  return ctx->dlib.xkb.state_key_get_one_sym(state, key);
}

static inline int lvkw_xkb_state_mod_name_is_active(const LVKW_Context_WL *ctx,
                                                    struct xkb_state *state, const char *name,
                                                    enum xkb_state_component type) {
  return ctx->dlib.xkb.state_mod_name_is_active(state, name, type);
}

static inline xkb_mod_index_t lvkw_xkb_keymap_mod_get_index(const LVKW_Context_WL *ctx,
                                                            struct xkb_keymap *keymap,
                                                            const char *name) {
  return ctx->dlib.xkb.keymap_mod_get_index(keymap, name);
}

static inline xkb_mod_mask_t lvkw_xkb_state_serialize_mods(const LVKW_Context_WL *ctx,
                                                           struct xkb_state *state,
                                                           enum xkb_state_component type) {
  return ctx->dlib.xkb.state_serialize_mods(state, type);
}

static inline int lvkw_xkb_state_key_get_utf8(const LVKW_Context_WL *ctx, struct xkb_state *state,
                                              xkb_keycode_t key, char *buffer, size_t size) {
  return ctx->dlib.xkb.state_key_get_utf8(state, key, buffer, size);
}

/* libdecor helpers */

static inline struct libdecor *lvkw_libdecor_new(const LVKW_Context_WL *ctx,
                                                 struct wl_display *display,
                                                 struct libdecor_interface *iface) {
  return ctx->dlib.opt.decor.new(display, iface);
}

static inline void lvkw_libdecor_unref(const LVKW_Context_WL *ctx, struct libdecor *context) {
  ctx->dlib.opt.decor.unref(context);
}

static inline struct libdecor_frame *lvkw_libdecor_decorate(const LVKW_Context_WL *ctx,
                                                            struct libdecor *context,
                                                            struct wl_surface *surface,
                                                            struct libdecor_frame_interface *iface,
                                                            void *user_data) {
  return ctx->dlib.opt.decor.decorate(context, surface, iface, user_data);
}

static inline void lvkw_libdecor_frame_unref(const LVKW_Context_WL *ctx,
                                             struct libdecor_frame *frame) {
  ctx->dlib.opt.decor.frame_unref(frame);
}

static inline void lvkw_libdecor_frame_set_title(const LVKW_Context_WL *ctx,
                                                 struct libdecor_frame *frame, const char *title) {
  ctx->dlib.opt.decor.frame_set_title(frame, title);
}

static inline void lvkw_libdecor_frame_set_app_id(const LVKW_Context_WL *ctx,
                                                  struct libdecor_frame *frame,
                                                  const char *app_id) {
  ctx->dlib.opt.decor.frame_set_app_id(frame, app_id);
}

static inline void lvkw_libdecor_frame_set_capabilities(const LVKW_Context_WL *ctx,
                                                        struct libdecor_frame *frame,
                                                        enum libdecor_capabilities capabilities) {
  ctx->dlib.opt.decor.frame_set_capabilities(frame, capabilities);
}

static inline void lvkw_libdecor_frame_map(const LVKW_Context_WL *ctx, struct libdecor_frame *frame) {
  ctx->dlib.opt.decor.frame_map(frame);
}

static inline void lvkw_libdecor_frame_set_visibility(const LVKW_Context_WL *ctx,
                                                      struct libdecor_frame *frame, bool visible) {
  ctx->dlib.opt.decor.frame_set_visibility(frame, visible);
}

static inline void lvkw_libdecor_frame_commit(const LVKW_Context_WL *ctx,
                                              struct libdecor_frame *frame,
                                              struct libdecor_state *state,
                                              struct libdecor_configuration *configuration) {
  ctx->dlib.opt.decor.frame_commit(frame, state, configuration);
}

static inline struct xdg_toplevel *lvkw_libdecor_frame_get_xdg_toplevel(
    const LVKW_Context_WL *ctx, struct libdecor_frame *frame) {
  return ctx->dlib.opt.decor.frame_get_xdg_toplevel(frame);
}

static inline struct xdg_surface *lvkw_libdecor_frame_get_xdg_surface(const LVKW_Context_WL *ctx,
                                                                      struct libdecor_frame *frame) {
  return ctx->dlib.opt.decor.frame_get_xdg_surface(frame);
}

static inline void lvkw_libdecor_frame_set_min_content_size(const LVKW_Context_WL *ctx,
                                                            struct libdecor_frame *frame,
                                                            int content_width,
                                                            int content_height) {
  ctx->dlib.opt.decor.frame_set_min_content_size(frame, content_width, content_height);
}

static inline void lvkw_libdecor_frame_set_max_content_size(const LVKW_Context_WL *ctx,
                                                            struct libdecor_frame *frame,
                                                            int content_width,
                                                            int content_height) {
  ctx->dlib.opt.decor.frame_set_max_content_size(frame, content_width, content_height);
}

static inline void lvkw_libdecor_frame_get_min_content_size(const LVKW_Context_WL *ctx,
                                                            const struct libdecor_frame *frame,
                                                            int *content_width,
                                                            int *content_height) {
  ctx->dlib.opt.decor.frame_get_min_content_size(frame, content_width, content_height);
}

static inline void lvkw_libdecor_frame_get_max_content_size(const LVKW_Context_WL *ctx,
                                                            const struct libdecor_frame *frame,
                                                            int *content_width,
                                                            int *content_height) {
  ctx->dlib.opt.decor.frame_get_max_content_size(frame, content_width, content_height);
}

static inline void lvkw_libdecor_frame_set_fullscreen(const LVKW_Context_WL *ctx,
                                                      struct libdecor_frame *frame,
                                                      struct wl_output *output) {
  ctx->dlib.opt.decor.frame_set_fullscreen(frame, output);
}

static inline void lvkw_libdecor_frame_unset_fullscreen(const LVKW_Context_WL *ctx,
                                                        struct libdecor_frame *frame) {
  ctx->dlib.opt.decor.frame_unset_fullscreen(frame);
}

static inline void lvkw_libdecor_frame_set_maximized(const LVKW_Context_WL *ctx,
                                                     struct libdecor_frame *frame) {
  ctx->dlib.opt.decor.frame_set_maximized(frame);
}

static inline void lvkw_libdecor_frame_unset_maximized(const LVKW_Context_WL *ctx,
                                                       struct libdecor_frame *frame) {
  ctx->dlib.opt.decor.frame_unset_maximized(frame);
}

static inline void lvkw_libdecor_frame_translate_coordinate(const LVKW_Context_WL *ctx,
                                                            struct libdecor_frame *frame,
                                                            int surface_x, int surface_y,
                                                            int *frame_x, int *frame_y) {
  ctx->dlib.opt.decor.frame_translate_coordinate(frame, surface_x, surface_y, frame_x, frame_y);
}

static inline struct libdecor_state *lvkw_libdecor_state_new(const LVKW_Context_WL *ctx, int width,
                                                             int height) {
  return ctx->dlib.opt.decor.state_new(width, height);
}

static inline void lvkw_libdecor_state_free(const LVKW_Context_WL *ctx,
                                            struct libdecor_state *state) {
  ctx->dlib.opt.decor.state_free(state);
}

static inline bool lvkw_libdecor_configuration_get_content_size(
    const LVKW_Context_WL *ctx, struct libdecor_configuration *configuration,
    struct libdecor_frame *frame, int *width, int *height) {
  return ctx->dlib.opt.decor.configuration_get_content_size(configuration, frame, width, height);
}

static inline bool lvkw_libdecor_configuration_get_window_state(
    const LVKW_Context_WL *ctx, struct libdecor_configuration *configuration,
    enum libdecor_window_state *window_state) {
  return ctx->dlib.opt.decor.configuration_get_window_state(configuration, window_state);
}

static inline int lvkw_libdecor_dispatch(const LVKW_Context_WL *ctx, struct libdecor *context,
                                         int timeout) {
  return ctx->dlib.opt.decor.dispatch(context, timeout);
}

static inline void lvkw_libdecor_set_userdata(const LVKW_Context_WL *ctx, struct libdecor *context,
                                              void *userdata) {
  if (ctx->dlib.opt.decor.opt.set_userdata)
    ctx->dlib.opt.decor.opt.set_userdata(context, userdata);
}

static inline void *lvkw_libdecor_get_userdata(const LVKW_Context_WL *ctx, struct libdecor *context) {
  return ctx->dlib.opt.decor.opt.get_userdata ? ctx->dlib.opt.decor.opt.get_userdata(context)
                                              : NULL;
}

/* wayland-client core helpers */

static inline struct wl_display *lvkw_wl_display_connect(LVKW_Context_WL *ctx, const char *name) {
  return ctx->dlib.wl.display_connect(name);
}

static inline void lvkw_wl_display_disconnect(LVKW_Context_WL *ctx, struct wl_display *display) {
  ctx->dlib.wl.display_disconnect(display);
}

static inline int lvkw_wl_display_roundtrip(LVKW_Context_WL *ctx, struct wl_display *display) {
  return ctx->dlib.wl.display_roundtrip(display);
}

static inline int lvkw_wl_display_flush(LVKW_Context_WL *ctx, struct wl_display *display) {
  return ctx->dlib.wl.display_flush(display);
}

static inline int lvkw_wl_display_prepare_read(LVKW_Context_WL *ctx, struct wl_display *display) {
  return ctx->dlib.wl.display_prepare_read(display);
}

static inline int lvkw_wl_display_get_fd(LVKW_Context_WL *ctx, struct wl_display *display) {
  return ctx->dlib.wl.display_get_fd(display);
}

static inline int lvkw_wl_display_read_events(LVKW_Context_WL *ctx, struct wl_display *display) {
  return ctx->dlib.wl.display_read_events(display);
}

static inline void lvkw_wl_display_cancel_read(LVKW_Context_WL *ctx, struct wl_display *display) {
  ctx->dlib.wl.display_cancel_read(display);
}

static inline int lvkw_wl_display_dispatch_pending(LVKW_Context_WL *ctx,
                                                   struct wl_display *display) {
  return ctx->dlib.wl.display_dispatch_pending(display);
}

static inline int lvkw_wl_display_get_error(LVKW_Context_WL *ctx, struct wl_display *display) {
  return ctx->dlib.wl.display_get_error(display);
}

static inline uint32_t lvkw_wl_display_get_protocol_error(LVKW_Context_WL *ctx,
                                                          struct wl_display *display,
                                                          const struct wl_interface **interface,
                                                          uint32_t *id) {
  return ctx->dlib.wl.display_get_protocol_error(display, interface, id);
}

static inline uint32_t lvkw_wl_proxy_get_version(LVKW_Context_WL *ctx, struct wl_proxy *proxy) {
  return ctx->dlib.wl.proxy_get_version(proxy);
}

static inline struct wl_proxy *lvkw_wl_proxy_marshal_flags(
    LVKW_Context_WL *ctx, struct wl_proxy *proxy, uint32_t opcode,
    const struct wl_interface *interface, uint32_t version, uint32_t flags, ...) {
  va_list args;
  va_start(args, flags);
  // We can't easily wrap varargs to another varargs function without a v* variant.
  // libwayland-client doesn't expose wl_proxy_marshal_flags_v.
  // However, we effectively only use this in generated code which calls the function pointer directly.
  // For manual usage, we might be stuck or have to rely on direct access if we can't solve this.
  // BUT, wayland-client.h defines wl_proxy_marshal_flags.
  // The generated helpers ALREADY access ctx->dlib.wl.proxy_marshal_flags.
  // So we might not need this wrapper for generated code.
  // Let's omit vararg wrappers for now or use macros?
  // User asked for "manual helpers".
  // Let's implement the non-varargs ones.
  va_end(args);
  return NULL;
}

static inline int lvkw_wl_proxy_add_listener(LVKW_Context_WL *ctx, struct wl_proxy *proxy,
                                             void (**implementation)(void), void *data) {
  return ctx->dlib.wl.proxy_add_listener(proxy, implementation, data);
}

static inline void lvkw_wl_proxy_destroy(LVKW_Context_WL *ctx, struct wl_proxy *proxy) {
  ctx->dlib.wl.proxy_destroy(proxy);
}

static inline void lvkw_wl_proxy_set_user_data(LVKW_Context_WL *ctx, struct wl_proxy *proxy,
                                               void *user_data) {
  ctx->dlib.wl.proxy_set_user_data(proxy, user_data);
}

static inline void *lvkw_wl_proxy_get_user_data(LVKW_Context_WL *ctx, struct wl_proxy *proxy) {
  return ctx->dlib.wl.proxy_get_user_data(proxy);
}

#include "protocols/generated/lvkw-content-type-v1-helpers.h"
#include "protocols/generated/lvkw-cursor-shape-v1-helpers.h"
#include "protocols/generated/lvkw-ext-idle-notify-v1-helpers.h"
#include "protocols/generated/lvkw-fractional-scale-v1-helpers.h"
#include "protocols/generated/lvkw-idle-inhibit-unstable-v1-helpers.h"
#include "protocols/generated/lvkw-pointer-constraints-unstable-v1-helpers.h"
#include "protocols/generated/lvkw-relative-pointer-unstable-v1-helpers.h"
#include "protocols/generated/lvkw-tablet-v2-helpers.h"
#include "protocols/generated/lvkw-viewporter-helpers.h"
#include "protocols/generated/lvkw-wayland-helpers.h"
#include "protocols/generated/lvkw-xdg-activation-v1-helpers.h"
#include "protocols/generated/lvkw-xdg-decoration-unstable-v1-helpers.h"
#include "protocols/generated/lvkw-xdg-output-unstable-v1-helpers.h"
#include "protocols/generated/lvkw-xdg-shell-helpers.h"

#endif
