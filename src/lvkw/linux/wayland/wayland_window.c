#include <stddef.h>
#include <string.h>

#include "dlib/libdecor.h"
#include "dlib/wayland-client.h"
#include "lvkw/lvkw.h"
#include "lvkw_api_constraints.h"
#include "lvkw_wayland_internal.h"

// Vulkan forward declarations
typedef enum VkResult {
  VK_SUCCESS = 0,
} VkResult;

#define VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR 1000006000
#define VK_NULL_HANDLE 0

typedef struct VkWaylandSurfaceCreateInfoKHR {
  int sType;
  const void *pNext;
  uint32_t flags;
  struct wl_display *display;
  struct wl_surface *surface;
} VkWaylandSurfaceCreateInfoKHR;

typedef void (*PFN_vkVoidFunction)(void);
typedef PFN_vkVoidFunction (*PFN_vkGetInstanceProcAddr)(VkInstance instance, const char *pName);
typedef VkResult (*PFN_vkCreateWaylandSurfaceKHR)(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                  const void *pAllocator, VkSurfaceKHR *pSurface);

extern PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char *pName);

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_wayland_backend;
#endif

LVKW_Status lvkw_ctx_createWindow_WL(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                     LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  *out_window_handle = NULL;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  LVKW_Window_WL *window = (LVKW_Window_WL *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Window_WL));
  if (!window) {
    return LVKW_ERROR;
  }
  memset(window, 0, sizeof(LVKW_Window_WL));

#ifdef LVKW_INDIRECT_BACKEND
  window->base.prv.backend = &_lvkw_wayland_backend;
#endif
  window->base.prv.ctx_base = &ctx->base;
  window->base.pub.userdata = create_info->userdata;
  window->size = create_info->attributes.logicalSize;
  window->scale = 1.0;
  window->cursor = create_info->attributes.cursor;

  window->transparent = create_info->transparent;
  window->monitor_id = create_info->attributes.monitor;
  window->is_fullscreen = create_info->attributes.fullscreen;
  window->is_maximized = create_info->attributes.maximized;

  window->wl.surface = wl_compositor_create_surface(ctx->protocols.wl_compositor);
  wl_surface_set_buffer_scale(window->wl.surface, 1);

  if (!window->wl.surface) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "wl_compositor_create_surface() failure");
    lvkw_context_free(&ctx->base, window);
    return LVKW_ERROR;
  }

  wl_proxy_set_user_data((struct wl_proxy *)window->wl.surface, window);
  wl_surface_add_listener(window->wl.surface, &_lvkw_wayland_surface_listener, window);

  if (!_lvkw_wayland_create_xdg_shell_objects(window, create_info)) {
    if (window->ext.content_type) {
      wp_content_type_v1_destroy(window->ext.content_type);
    }
    wl_surface_destroy(window->wl.surface);
    lvkw_context_free(&ctx->base, window);
    return LVKW_ERROR;
  }

  _lvkw_wayland_update_opaque_region(window);

  wl_surface_commit(window->wl.surface);
  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  // Add to context window list
  _lvkw_window_list_add(&ctx->base, &window->base);

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_destroy_WL(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (ctx->input.keyboard_focus == window) {
    ctx->input.keyboard_focus = NULL;
  }

  if (ctx->input.pointer_focus == window) {
    ctx->input.pointer_focus = NULL;
  }

  lvkw_event_queue_remove_window_events(&ctx->events.queue, window_handle);

  // Remove from linked list
  _lvkw_window_list_remove(&ctx->base, &window->base);

  if (window->xdg.decoration) {
    zxdg_toplevel_decoration_v1_destroy(window->xdg.decoration);
  }

  if (window->ext.fractional_scale) {
    wp_fractional_scale_v1_destroy(window->ext.fractional_scale);
  }

  if (window->ext.viewport) {
    wp_viewport_destroy(window->ext.viewport);
  }

  if (window->ext.idle_inhibitor) {
    zwp_idle_inhibitor_v1_destroy(window->ext.idle_inhibitor);
  }

  if (window->ext.content_type) {
    wp_content_type_v1_destroy(window->ext.content_type);
  }

  if (window->decor_mode != LVKW_WAYLAND_DECORATION_MODE_CSD) {
    if (window->xdg.toplevel) xdg_toplevel_destroy(window->xdg.toplevel);
    if (window->xdg.surface) xdg_surface_destroy(window->xdg.surface);
  }
  else {
    if (window->libdecor.frame) {
      libdecor_frame_unref(window->libdecor.frame);
    }
  }

  //  LVKW_WND_ASSUME(&window->base, window->wl.surface != NULL, "Window surface must not be NULL during destruction");
  wl_surface_destroy(window->wl.surface);

  lvkw_context_free(&ctx->base, window);
  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_WL(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setMaximized_WL(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setCursorMode_WL(LVKW_Window *window_handle, LVKW_CursorMode mode);
static LVKW_Status _lvkw_wnd_setCursor_WL(LVKW_Window *window_handle, LVKW_Cursor *cursor);

LVKW_Status lvkw_wnd_update_WL(LVKW_Window *window_handle, uint32_t field_mask,
                               const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (field_mask & LVKW_WND_ATTR_TITLE) {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      libdecor_frame_set_title(window->libdecor.frame, attributes->title);
    }
    else if (window->xdg.toplevel) {
      xdg_toplevel_set_title(window->xdg.toplevel, attributes->title);
    }
  }

  if (field_mask & LVKW_WND_ATTR_LOGICAL_SIZE) {
    if (window->size.x != attributes->logicalSize.x || window->size.y != attributes->logicalSize.y) {
      window->size = attributes->logicalSize;

      // For SSD or No decorations, there is no way to "ask" for a resize in the protocol.
      // We update our internal size and trigger a resize event.
      // If the user then recreates their swapchain and attaches a new buffer, the
      // compositor will typically resize the window to match the buffer.
      LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
      _lvkw_wayland_push_event(ctx, &evt);

      _lvkw_wayland_update_opaque_region(window);
    }
  }

  if (field_mask & LVKW_WND_ATTR_FULLSCREEN) {
    _lvkw_wnd_setFullscreen_WL(window_handle, attributes->fullscreen);
  }

  if (field_mask & LVKW_WND_ATTR_MAXIMIZED) {
    _lvkw_wnd_setMaximized_WL(window_handle, attributes->maximized);
  }

  if (field_mask & LVKW_WND_ATTR_CURSOR_MODE) {
    _lvkw_wnd_setCursorMode_WL(window_handle, attributes->cursor_mode);
  }

  if (field_mask & LVKW_WND_ATTR_CURSOR) {
    _lvkw_wnd_setCursor_WL(window_handle, attributes->cursor);
  }

  if (field_mask & LVKW_WND_ATTR_MONITOR) {
    window->monitor_id = attributes->monitor;
    if (window->is_fullscreen) {
      _lvkw_wnd_setFullscreen_WL(window_handle, true);
    }
  }

  _lvkw_wayland_check_error(ctx);
  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_WL(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  // We re-apply fullscreen if enabled is true, to handle monitor changes
  if (window->is_fullscreen == enabled && !enabled) return LVKW_SUCCESS;

  struct wl_output *target_output = NULL;
  if (enabled && window->monitor_id != LVKW_MONITOR_ID_INVALID) {
    target_output = _lvkw_wayland_find_monitor(ctx, window->monitor_id);
  }

  if (enabled) {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      libdecor_frame_set_fullscreen(window->libdecor.frame, target_output);
    }
    else {
      xdg_toplevel_set_fullscreen(window->xdg.toplevel, target_output);
    }
  }
  else {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      libdecor_frame_unset_fullscreen(window->libdecor.frame);
    }
    else {
      xdg_toplevel_unset_fullscreen(window->xdg.toplevel);
    }
  }

  window->is_fullscreen = enabled;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setMaximized_WL(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;

  if (window->is_maximized == enabled) return LVKW_SUCCESS;

  if (enabled) {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      libdecor_frame_set_maximized(window->libdecor.frame);
    }
    else {
      xdg_toplevel_set_maximized(window->xdg.toplevel);
    }
  }
  else {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      libdecor_frame_unset_maximized(window->libdecor.frame);
    }
    else {
      xdg_toplevel_unset_maximized(window->xdg.toplevel);
    }
  }

  window->is_maximized = enabled;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursorMode_WL(LVKW_Window *window_handle, LVKW_CursorMode mode) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (window->cursor_mode == mode) return LVKW_SUCCESS;

  if (mode == LVKW_CURSOR_LOCKED) {
    if (ctx->protocols.opt.zwp_relative_pointer_manager_v1 && ctx->protocols.opt.zwp_pointer_constraints_v1) {
      window->input.relative = zwp_relative_pointer_manager_v1_get_relative_pointer(
          ctx->protocols.opt.zwp_relative_pointer_manager_v1, ctx->input.pointer);
      window->input.locked = zwp_pointer_constraints_v1_lock_pointer(ctx->protocols.opt.zwp_pointer_constraints_v1,
                                                                     window->wl.surface, ctx->input.pointer, NULL,
                                                                     ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    }
  }
  else {
    if (window->input.relative) {
      zwp_relative_pointer_v1_destroy(window->input.relative);
      window->input.relative = NULL;
    }
    if (window->input.locked) {
      zwp_locked_pointer_v1_destroy(window->input.locked);
      window->input.locked = NULL;
    }
  }

  window->cursor_mode = mode;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursor_WL(LVKW_Window *window_handle, LVKW_Cursor *cursor) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;

  window->cursor = cursor;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_createVkSurface_WL(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  *out_surface = VK_NULL_HANDLE;
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  PFN_vkCreateWaylandSurfaceKHR create_surface_fn =
      (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");

  if (!create_surface_fn) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE, "vkCreateWaylandSurfaceKHR not found");

    window->base.pub.flags |= LVKW_WND_STATE_LOST;

    return LVKW_ERROR_WINDOW_LOST;
  }

  VkWaylandSurfaceCreateInfoKHR cinfo = {
      .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
      .pNext = NULL,
      .flags = 0,
      .display = ctx->wl.display,
      .surface = window->wl.surface,
  };

  VkResult vk_res = create_surface_fn(instance, &cinfo, NULL, out_surface);

  if (vk_res != VK_SUCCESS) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE, "vkCreateWaylandSurfaceKHR failure");

    window->base.pub.flags |= LVKW_WND_STATE_LOST;

    return LVKW_ERROR_WINDOW_LOST;
  }

  _lvkw_wayland_check_error(ctx);

  if (ctx->base.pub.flags & LVKW_CTX_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getGeometry_WL(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  const LVKW_Window_WL *window = (const LVKW_Window_WL *)window_handle;

  *out_geometry = (LVKW_WindowGeometry){
      .logicalSize = window->size,
      .pixelSize =
          {
              .x = (int32_t)(window->size.x * window->scale),
              .y = (int32_t)(window->size.y * window->scale),
          },
  };

  return LVKW_SUCCESS;
}