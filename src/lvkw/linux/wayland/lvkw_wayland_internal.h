#ifndef LVKW_WAYLAND_INTERNAL_H_INCLUDED
#define LVKW_WAYLAND_INTERNAL_H_INCLUDED

#include "dlib/wayland-client.h"

// For xkb_mod_index_t
#include "dlib/xkbcommon.h"  // IWYU pragma: keep
#include "lvkw_event_queue.h"
#include "lvkw_linux_internal.h"
#include "lvkw_thread_internal.h"
#include "protocols/wl_protocols.h"

// Indirect wayland client

#define LVKW_WAYLAND_MAX_EVENTS 4096

typedef enum LVKW_DecorationMode {
  LVKW_DECORATION_MODE_AUTO,
  LVKW_DECORATION_MODE_SSD,
  LVKW_DECORATION_MODE_CSD,
  LVKW_DECORATION_MODE_NONE
} LVKW_DecorationMode;

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
  double scale;
  LVKW_DecorationMode decor_mode;
  LVKW_CursorMode cursor_mode;
  LVKW_CursorShape cursor_shape;
  bool is_fullscreen;
  bool is_maximized;

  /* Flags */
  bool transparent;
  LVKW_MonitorId monitor_id;
} LVKW_Window_WL;

typedef struct LVKW_EventDispatchContext_WL {
  LVKW_EventCallback callback;
  void *userdata;
  LVKW_EventType evt_mask;
} LVKW_EventDispatchContext_WL;

typedef struct LVKW_Monitor_WL {
  struct LVKW_Context_WL *ctx;
  LVKW_MonitorInfo info;
  uint32_t global_id;
  struct wl_output *wl_output;
  struct zxdg_output_v1 *xdg_output;
  LVKW_VideoMode *modes;
  uint32_t mode_count;
  struct LVKW_Monitor_WL *next;
  bool announced;
} LVKW_Monitor_WL;

typedef struct LVKW_Context_WL {
  LVKW_Context_Base base;

#ifdef LVKW_CONTROLLER_ENABLED
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

  bool inhibit_idle;

  struct {
    LVKW_EventQueue queue;
    LVKW_EventDispatchContext_WL *dispatch_ctx;
  } events;

  /* Monitor management */
  LVKW_Monitor_WL *monitors_list_start;
  uint32_t last_monitor_id;

  LVKW_StringCache string_cache;
} LVKW_Context_WL;

#ifdef LVKW_CONTROLLER_ENABLED
#endif

void _lvkw_wayland_update_opaque_region(LVKW_Window_WL *window);
LVKW_Event _lvkw_wayland_make_window_resized_event(LVKW_Window_WL *window);
void _lvkw_wayland_push_event(LVKW_Context_WL *ctx, const LVKW_Event *evt);
void _lvkw_wayland_flush_event_pool(LVKW_Context_WL *ctx);
void _lvkw_wayland_check_error(LVKW_Context_WL *ctx);
void _lvkw_wayland_bind_output(LVKW_Context_WL *ctx, uint32_t name, uint32_t version);
void _lvkw_wayland_remove_monitor_by_name(LVKW_Context_WL *ctx, uint32_t name);
void _lvkw_wayland_destroy_monitors(LVKW_Context_WL *ctx);
struct wl_output *_lvkw_wayland_find_monitor(LVKW_Context_WL *ctx, LVKW_MonitorId id);

extern const struct wl_seat_listener _lvkw_wayland_seat_listener;
extern const struct xdg_wm_base_listener _lvkw_wayland_wm_base_listener;
extern const struct wl_output_listener _lvkw_wayland_output_listener;
extern const struct wp_fractional_scale_v1_listener _lvkw_wayland_fractional_scale_listener;
extern const struct wl_surface_listener _lvkw_wayland_surface_listener;
extern const struct xdg_surface_listener _lvkw_wayland_xdg_surface_listener;
extern const struct xdg_toplevel_listener _lvkw_wayland_xdg_toplevel_listener;
extern const struct zxdg_toplevel_decoration_v1_listener _lvkw_wayland_xdg_decoration_listener;
extern const struct ext_idle_notification_v1_listener _lvkw_wayland_idle_listener;

LVKW_DecorationMode _lvkw_wayland_get_decoration_mode(void);
bool _lvkw_wayland_create_xdg_shell_objects(LVKW_Window_WL *window, const LVKW_WindowCreateInfo *create_info);

LVKW_Status lvkw_ctx_create_WL(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_context);
void lvkw_ctx_destroy_WL(LVKW_Context *handle);
const char *const *lvkw_ctx_getVkExtensions_WL(LVKW_Context *ctx, uint32_t *count);
LVKW_Status lvkw_ctx_pollEvents_WL(LVKW_Context *ctx, LVKW_EventType event_mask, LVKW_EventCallback callback,
                                   void *userdata);
LVKW_Status lvkw_ctx_waitEvents_WL(LVKW_Context *ctx, uint32_t timeout_ms, LVKW_EventType event_mask,
                                   LVKW_EventCallback callback, void *userdata);
LVKW_Status lvkw_ctx_update_WL(LVKW_Context *ctx, uint32_t field_mask, const LVKW_ContextAttributes *attributes);
LVKW_Status lvkw_ctx_getMonitors_WL(LVKW_Context *ctx, LVKW_MonitorInfo *out_monitors, uint32_t *count);
LVKW_Status lvkw_ctx_getMonitorModes_WL(LVKW_Context *ctx, LVKW_MonitorId monitor, LVKW_VideoMode *out_modes,
                                        uint32_t *count);
LVKW_Status lvkw_ctx_createWindow_WL(LVKW_Context *ctx, const LVKW_WindowCreateInfo *create_info,
                                     LVKW_Window **out_window);
void lvkw_wnd_destroy_WL(LVKW_Window *handle);
LVKW_Status lvkw_wnd_createVkSurface_WL(LVKW_Window *window, VkInstance instance, VkSurfaceKHR *out_surface);
LVKW_Status lvkw_wnd_getGeometry_WL(LVKW_Window *window, LVKW_WindowGeometry *out_geometry);
LVKW_Status lvkw_wnd_update_WL(LVKW_Window *window, uint32_t field_mask, const LVKW_WindowAttributes *attributes);
LVKW_Status lvkw_wnd_requestFocus_WL(LVKW_Window *window);

#endif