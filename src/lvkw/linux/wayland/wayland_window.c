// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <stdlib.h>
#include <string.h>

#include "api_constraints.h"
#include "assume.h"
#include "wayland_internal.h"

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
typedef VkResult (*PFN_vkCreateWaylandSurfaceKHR)(VkInstance instance,
                                                  const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                  const void *pAllocator, VkSurfaceKHR *pSurface);

extern __attribute__((weak)) PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance,
                                                                      const char *pName);

#ifdef LVKW_INDIRECT_BACKEND
extern const LVKW_Backend _lvkw_wayland_backend;
#endif

static LVKW_Status _lvkw_wnd_setFullscreen_WL(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setMaximized_WL(LVKW_Window *window_handle, bool enabled);
static LVKW_Status _lvkw_wnd_setCursorMode_WL(LVKW_Window *window_handle, LVKW_CursorMode mode);
static LVKW_Status _lvkw_wnd_setCursor_WL(LVKW_Window *window_handle, LVKW_Cursor *cursor);

LVKW_Status lvkw_ctx_createWindow_WL(LVKW_Context *ctx_handle,
                                     const LVKW_WindowCreateInfo *create_info,
                                     LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  *out_window_handle = NULL;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)ctx_handle;

  LVKW_Window_WL *window = (LVKW_Window_WL *)lvkw_context_alloc(&ctx->linux_base.base, sizeof(LVKW_Window_WL));
  if (!window) {
    return LVKW_ERROR;
  }
  memset(window, 0, sizeof(LVKW_Window_WL));

#ifdef LVKW_INDIRECT_BACKEND
  window->base.prv.backend = &_lvkw_wayland_backend;
#endif
  window->base.prv.ctx_base = &ctx->linux_base.base;
  window->base.pub.context = &ctx->linux_base.base.pub;
  window->base.pub.userdata = create_info->userdata;
  window->size = create_info->attributes.logical_size;
  window->min_size = create_info->attributes.min_size;
  window->max_size = create_info->attributes.max_size;
  window->aspect_ratio = create_info->attributes.aspect_ratio;
  window->scale = 1.0;
  window->buffer_transform = WL_OUTPUT_TRANSFORM_NORMAL;
  window->cursor_mode = LVKW_CURSOR_NORMAL;
  window->cursor = create_info->attributes.cursor;

  window->transparent = create_info->transparent;
  window->mouse_passthrough = create_info->attributes.mouse_passthrough;
  window->monitor = create_info->attributes.monitor;
  window->is_fullscreen = create_info->attributes.fullscreen;
  window->is_maximized = create_info->attributes.maximized;
  window->is_resizable = create_info->attributes.resizable;
  window->is_decorated = create_info->attributes.decorated;
  window->accept_dnd = create_info->attributes.accept_dnd;
  window->text_input_type = create_info->attributes.text_input_type;
  window->text_input_rect = create_info->attributes.text_input_rect;

  window->wl.surface =
      lvkw_wl_compositor_create_surface(ctx, ctx->protocols.wl_compositor);

  if (!window->wl.surface) {
    LVKW_REPORT_CTX_DIAGNOSTIC(&ctx->linux_base.base, LVKW_DIAGNOSTIC_RESOURCE_UNAVAILABLE,
                               "wl_compositor_create_surface() failure");
    lvkw_context_free(&ctx->linux_base.base, window);
    return LVKW_ERROR;
  }

  lvkw_wl_surface_set_buffer_scale(ctx, window->wl.surface, 1);
  lvkw_wl_surface_set_buffer_transform(ctx, window->wl.surface, WL_OUTPUT_TRANSFORM_NORMAL);
  lvkw_wl_surface_set_user_data(ctx, window->wl.surface, window);
  lvkw_wl_surface_add_listener(ctx, window->wl.surface, &_lvkw_wayland_surface_listener,
                               window);

  if (!_lvkw_wayland_create_xdg_shell_objects(window, create_info)) {
    if (window->ext.content_type) {
      lvkw_wp_content_type_v1_destroy(ctx, window->ext.content_type);
    }
    lvkw_wl_surface_destroy(ctx, window->wl.surface);
    lvkw_context_free(&ctx->linux_base.base, window);
    return LVKW_ERROR;
  }

  _lvkw_wayland_update_opaque_region(window);
  _lvkw_wnd_setCursorMode_WL((LVKW_Window *)window, create_info->attributes.cursor_mode);

  lvkw_wl_surface_commit(ctx, window->wl.surface);
  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  // Add to context window list
  _lvkw_window_list_add(&ctx->linux_base.base, &window->base);

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_destroy_WL(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (ctx->input.keyboard_focus == window) {
    _lvkw_wayland_sync_text_input_state(ctx, NULL);
    ctx->input.keyboard_focus = NULL;
  }

  if (ctx->input.pointer_focus == window) {
    ctx->input.pointer_focus = NULL;
  }

  if (ctx->input.dnd.window == window) {
    _lvkw_wayland_dnd_reset(ctx, true);
  }

  lvkw_event_queue_remove_window_events(&ctx->linux_base.base.prv.event_queue, window_handle);

  // Remove from linked list
  _lvkw_window_list_remove(&ctx->linux_base.base, &window->base);

  if (window->xdg.decoration) {
    lvkw_zxdg_toplevel_decoration_v1_destroy(ctx, window->xdg.decoration);
  }

  if (window->ext.fractional_scale) {
    lvkw_wp_fractional_scale_v1_destroy(ctx, window->ext.fractional_scale);
  }

  if (window->ext.viewport) {
    lvkw_wp_viewport_destroy(ctx, window->ext.viewport);
  }

  if (window->ext.idle_inhibitor) {
    lvkw_zwp_idle_inhibitor_v1_destroy(ctx, window->ext.idle_inhibitor);
  }

  if (window->ext.content_type) {
    lvkw_wp_content_type_v1_destroy(ctx, window->ext.content_type);
  }

  if (window->input.relative) {
    lvkw_zwp_relative_pointer_v1_destroy(ctx, window->input.relative);
    window->input.relative = NULL;
  }
  if (window->input.locked) {
    lvkw_zwp_locked_pointer_v1_destroy(ctx, window->input.locked);
    window->input.locked = NULL;
  }

  if (window->decor_mode != LVKW_WAYLAND_DECORATION_MODE_CSD) {
    if (window->xdg.toplevel) lvkw_xdg_toplevel_destroy(ctx, window->xdg.toplevel);
    if (window->xdg.surface) lvkw_xdg_surface_destroy(ctx, window->xdg.surface);
  }
  else {
    if (window->libdecor.frame) {
      window->libdecor.last_configuration = NULL;
      lvkw_libdecor_frame_unref(ctx, window->libdecor.frame);
    }
  }

  lvkw_wl_proxy_set_user_data(ctx, (struct wl_proxy *)window->wl.surface, NULL);
  lvkw_wl_surface_destroy(ctx, window->wl.surface);

  lvkw_context_free(&ctx->linux_base.base, window);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_update_WL(LVKW_Window *window_handle, uint32_t field_mask,
                               const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (field_mask & LVKW_WINDOW_ATTR_TITLE) {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      lvkw_libdecor_frame_set_title(ctx, window->libdecor.frame,
                                    attributes->title);
    }
    else if (window->xdg.toplevel) {
      lvkw_xdg_toplevel_set_title(ctx, window->xdg.toplevel, attributes->title);
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_LOGICAL_SIZE) {
    if (window->size.x != attributes->logical_size.x ||
        window->size.y != attributes->logical_size.y) {
      window->size = attributes->logical_size;
      _lvkw_wayland_apply_size_constraints(window);

      LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
      lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                          LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window,
                                          &evt);

      _lvkw_wayland_update_opaque_region(window);
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_FULLSCREEN) {
    _lvkw_wnd_setFullscreen_WL(window_handle, attributes->fullscreen);
  }

  if (field_mask & LVKW_WINDOW_ATTR_MAXIMIZED) {
    _lvkw_wnd_setMaximized_WL(window_handle, attributes->maximized);
  }

  if (field_mask & LVKW_WINDOW_ATTR_CURSOR_MODE) {
    _lvkw_wnd_setCursorMode_WL(window_handle, attributes->cursor_mode);
  }

  if (field_mask & LVKW_WINDOW_ATTR_CURSOR) {
    _lvkw_wnd_setCursor_WL(window_handle, attributes->cursor);
  }

  if (field_mask & LVKW_WINDOW_ATTR_MONITOR) {
    window->monitor = attributes->monitor;
    if (window->is_fullscreen) {
      _lvkw_wnd_setFullscreen_WL(window_handle, true);
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_MIN_SIZE) {
    window->min_size = attributes->min_size;
    _lvkw_wayland_apply_size_constraints(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_MAX_SIZE) {
    window->max_size = attributes->max_size;
    _lvkw_wayland_apply_size_constraints(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_ASPECT_RATIO) {
    window->aspect_ratio = attributes->aspect_ratio;
    _lvkw_wayland_apply_size_constraints(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_RESIZABLE) {
    window->is_resizable = attributes->resizable;
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      enum libdecor_capabilities caps =
          LIBDECOR_ACTION_CLOSE | LIBDECOR_ACTION_MOVE | LIBDECOR_ACTION_MINIMIZE;
      if (window->is_resizable) {
        caps |= LIBDECOR_ACTION_RESIZE | LIBDECOR_ACTION_FULLSCREEN;
      }
      lvkw_libdecor_frame_set_capabilities(ctx, window->libdecor.frame, caps);
    }
    _lvkw_wayland_apply_size_constraints(window);
  }

  if (field_mask & LVKW_WINDOW_ATTR_DECORATED) {
    window->is_decorated = attributes->decorated;
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD && window->libdecor.frame) {
      lvkw_libdecor_frame_set_visibility(ctx, window->libdecor.frame, window->is_decorated);
    }
    else if (window->xdg.decoration) {
      lvkw_zxdg_toplevel_decoration_v1_set_mode(
          ctx, window->xdg.decoration,
          window->is_decorated ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                               : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_ACCEPT_DND) {
    window->accept_dnd = attributes->accept_dnd;

    if (!window->accept_dnd && ctx->input.dnd.window == window) {
      if (ctx->input.dnd.offer) {
        lvkw_wl_data_offer_accept(ctx, ctx->input.dnd.offer, ctx->input.dnd.serial, NULL);
      }
      _lvkw_wayland_dnd_reset(ctx, true);
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE) {
    window->text_input_type = attributes->text_input_type;
    if (ctx->input.keyboard_focus == window) {
      _lvkw_wayland_sync_text_input_state(ctx, window);
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_TEXT_INPUT_RECT) {
    window->text_input_rect = attributes->text_input_rect;
    if (ctx->input.keyboard_focus == window) {
      _lvkw_wayland_sync_text_input_state(ctx, window);
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_MOUSE_PASSTHROUGH) {
    window->mouse_passthrough = attributes->mouse_passthrough;
    _lvkw_wayland_update_opaque_region(window);
  }

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setFullscreen_WL(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (window->is_fullscreen == enabled && !enabled) return LVKW_SUCCESS;

  struct wl_output *target_output = NULL;
  if (enabled && window->monitor != NULL) {
    target_output = _lvkw_wayland_find_monitor(ctx, window->monitor);
  }

  if (enabled) {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      lvkw_libdecor_frame_set_fullscreen(ctx, window->libdecor.frame, target_output);
    }
    else {
      lvkw_xdg_toplevel_set_fullscreen(ctx, window->xdg.toplevel, target_output);
    }
  }
  else {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      lvkw_libdecor_frame_unset_fullscreen(ctx, window->libdecor.frame);
    }
    else {
      lvkw_xdg_toplevel_unset_fullscreen(ctx, window->xdg.toplevel);
    }
  }

  window->is_fullscreen = enabled;
  if (enabled)
    window->base.pub.flags |= LVKW_WINDOW_STATE_FULLSCREEN;
  else
    window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_FULLSCREEN;

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setMaximized_WL(LVKW_Window *window_handle, bool enabled) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (window->is_maximized == enabled) return LVKW_SUCCESS;

  if (enabled) {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      lvkw_libdecor_frame_set_maximized(ctx, window->libdecor.frame);
    }
    else {
      lvkw_xdg_toplevel_set_maximized(ctx, window->xdg.toplevel);
    }
  }
  else {
    if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD) {
      lvkw_libdecor_frame_unset_maximized(ctx, window->libdecor.frame);
    }
    else {
      lvkw_xdg_toplevel_unset_maximized(ctx, window->xdg.toplevel);
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
    if (ctx->protocols.opt.zwp_relative_pointer_manager_v1 &&
        ctx->protocols.opt.zwp_pointer_constraints_v1 && ctx->input.pointer) {
      window->input.relative = lvkw_zwp_relative_pointer_manager_v1_get_relative_pointer(
          ctx, ctx->protocols.opt.zwp_relative_pointer_manager_v1, ctx->input.pointer);
      window->input.locked = lvkw_zwp_pointer_constraints_v1_lock_pointer(
          ctx, ctx->protocols.opt.zwp_pointer_constraints_v1, window->wl.surface,
          ctx->input.pointer, NULL, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

      if (window->input.relative) {
        lvkw_zwp_relative_pointer_v1_add_listener(ctx, window->input.relative,
                                                  &_lvkw_wayland_relative_pointer_listener,
                                                  window);
      }

      if (window->input.locked) {
        lvkw_zwp_locked_pointer_v1_add_listener(ctx, window->input.locked,
                                                &_lvkw_wayland_locked_pointer_listener, window);
      }
    }
  }
  else {
    if (window->input.relative) {
      lvkw_zwp_relative_pointer_v1_destroy(ctx, window->input.relative);
      window->input.relative = NULL;
    }
    if (window->input.locked) {
      lvkw_zwp_locked_pointer_v1_destroy(ctx, window->input.locked);
      window->input.locked = NULL;
    }
  }

  window->cursor_mode = mode;

  if (mode != LVKW_CURSOR_LOCKED && ctx->input.pointer_focus == window) {
    _lvkw_wayland_update_cursor(ctx, window, ctx->input.pointer_serial);
  }

  return LVKW_SUCCESS;
}

static LVKW_Status _lvkw_wnd_setCursor_WL(LVKW_Window *window_handle, LVKW_Cursor *cursor) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  window->cursor = cursor;

  if (window->cursor_mode != LVKW_CURSOR_LOCKED && ctx->input.pointer_focus == window) {
    _lvkw_wayland_update_cursor(ctx, window, ctx->input.pointer_serial);
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_createVkSurface_WL(LVKW_Window *window_handle, VkInstance instance,
                                        VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  *out_surface = VK_NULL_HANDLE;
  LVKW_Window_WL *window = (LVKW_Window_WL *)window_handle;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  PFN_vkGetInstanceProcAddr vk_loader = (PFN_vkGetInstanceProcAddr)ctx->linux_base.base.prv.vk_loader;
  if (!vk_loader) vk_loader = vkGetInstanceProcAddr;

  if (!vk_loader) {
    LVKW_REPORT_WIND_DIAGNOSTIC(
        &window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE,
        "No Vulkan loader available. Provide vk_loader in context tuning or link against "
        "Vulkan.");
    window->base.pub.flags |= LVKW_WINDOW_STATE_LOST;
    return LVKW_ERROR_WINDOW_LOST;
  }

  PFN_vkCreateWaylandSurfaceKHR create_surface_fn =
      (PFN_vkCreateWaylandSurfaceKHR)vk_loader(instance, "vkCreateWaylandSurfaceKHR");

  if (!create_surface_fn) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE,
                                "vkCreateWaylandSurfaceKHR not found");
    window->base.pub.flags |= LVKW_WINDOW_STATE_LOST;
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
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE,
                                "vkCreateWaylandSurfaceKHR failure");
    window->base.pub.flags |= LVKW_WINDOW_STATE_LOST;
    return LVKW_ERROR_WINDOW_LOST;
  }

  _lvkw_wayland_check_error(ctx);
  if (ctx->linux_base.base.pub.flags & LVKW_CONTEXT_STATE_LOST) return LVKW_ERROR_CONTEXT_LOST;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getGeometry_WL(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  const LVKW_Window_WL *window = (const LVKW_Window_WL *)window_handle;

  *out_geometry = (LVKW_WindowGeometry){
      .origin = {0, 0},
      .logical_size = window->size,
      .pixel_size =
          {
              .x = (int32_t)(window->size.x * window->scale),
              .y = (int32_t)(window->size.y * window->scale),
          },
  };

  return LVKW_SUCCESS;
}

void _lvkw_wayland_update_opaque_region(LVKW_Window_WL *window) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (window->transparent) {
    lvkw_wl_surface_set_opaque_region(ctx, window->wl.surface, NULL);
  }
  else {
    struct wl_region *region =
        lvkw_wl_compositor_create_region(ctx, ctx->protocols.wl_compositor);
    lvkw_wl_region_add(ctx, region, 0, 0, (int32_t)window->size.x,
                       (int32_t)window->size.y);
    lvkw_wl_surface_set_opaque_region(ctx, window->wl.surface, region);
    lvkw_wl_region_destroy(ctx, region);
  }

  struct wl_region *input_region =
      lvkw_wl_compositor_create_region(ctx, ctx->protocols.wl_compositor);
  if (!window->mouse_passthrough) {
    lvkw_wl_region_add(ctx, input_region, 0, 0, (int32_t)window->size.x,
                       (int32_t)window->size.y);
  }
  lvkw_wl_surface_set_input_region(ctx, window->wl.surface, input_region);
  lvkw_wl_region_destroy(ctx, input_region);

  if (window->xdg.surface) {
    lvkw_xdg_surface_set_window_geometry(ctx, window->xdg.surface, 0, 0,
                                         (int)window->size.x, (int)window->size.y);
  }
}

LVKW_Event _lvkw_wayland_make_window_resized_event(LVKW_Window_WL *window) {
  LVKW_Event evt;
  evt.resized.geometry.logical_size = window->size;
  evt.resized.geometry.pixel_size.x = (int32_t)(window->size.x * window->scale);
  evt.resized.geometry.pixel_size.y = (int32_t)(window->size.y * window->scale);
  return evt;
}

static void _lvkw_wayland_enforce_aspect_ratio(uint32_t *width, uint32_t *height,
                                               const LVKW_Window_WL *window);

void _lvkw_wayland_apply_size_constraints(LVKW_Window_WL *window) {
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_LogicalVec min_size = window->min_size;
  LVKW_LogicalVec max_size = window->max_size;

  if (!window->is_resizable) {
    min_size = window->size;
    max_size = window->size;
  }

  if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_CSD && window->libdecor.frame) {
    lvkw_libdecor_frame_set_min_content_size(ctx, window->libdecor.frame, (int)min_size.x,
                                             (int)min_size.y);
    lvkw_libdecor_frame_set_max_content_size(ctx, window->libdecor.frame, (int)max_size.x,
                                             (int)max_size.y);

    // Some compositors/libdecor stacks only apply updated constraints promptly when
    // the underlying xdg_toplevel role receives the min/max update as well.
    struct xdg_toplevel *xdg_toplevel =
        lvkw_libdecor_frame_get_xdg_toplevel(ctx, window->libdecor.frame);
    if (xdg_toplevel) {
      lvkw_xdg_toplevel_set_min_size(ctx, xdg_toplevel, (int)min_size.x, (int)min_size.y);
      lvkw_xdg_toplevel_set_max_size(ctx, xdg_toplevel, (int)max_size.x, (int)max_size.y);
    }

    if (!window->is_fullscreen && !window->is_maximized) {
      uint32_t content_width = (uint32_t)window->size.x;
      uint32_t content_height = (uint32_t)window->size.y;

      if (min_size.x > 0 && content_width < (uint32_t)min_size.x) {
        content_width = (uint32_t)min_size.x;
      }
      if (min_size.y > 0 && content_height < (uint32_t)min_size.y) {
        content_height = (uint32_t)min_size.y;
      }

      if (max_size.x > 0 && content_width > (uint32_t)max_size.x) {
        content_width = (uint32_t)max_size.x;
      }
      if (max_size.y > 0 && content_height > (uint32_t)max_size.y) {
        content_height = (uint32_t)max_size.y;
      }

      if (ctx->enforce_client_side_constraints) {
        _lvkw_wayland_enforce_aspect_ratio(&content_width, &content_height, window);
      }

      struct libdecor_state *state =
          lvkw_libdecor_state_new(ctx, (int)content_width, (int)content_height);
      struct libdecor_configuration *commit_config = window->libdecor.last_configuration;
      lvkw_libdecor_frame_commit(ctx, window->libdecor.frame, state, commit_config);
      lvkw_libdecor_state_free(ctx, state);

      LVKW_Scalar new_width = (LVKW_Scalar)content_width;
      LVKW_Scalar new_height = (LVKW_Scalar)content_height;
      bool forced_size_changed =
          window->size.x != new_width || window->size.y != new_height;
      window->size.x = new_width;
      window->size.y = new_height;
      if (forced_size_changed) {
        _lvkw_wayland_update_opaque_region(window);
        LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
        lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                           LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window, &evt);
      }
    }
  }
  else if (window->xdg.toplevel) {
    lvkw_xdg_toplevel_set_min_size(ctx, window->xdg.toplevel, (int)min_size.x, (int)min_size.y);
    lvkw_xdg_toplevel_set_max_size(ctx, window->xdg.toplevel, (int)max_size.x, (int)max_size.y);

  }

  // xdg/libdecor role state is applied on wl_surface.commit; push immediately so
  // new constraints take effect without waiting for a later redraw/resize cycle.
  lvkw_wl_surface_commit(ctx, window->wl.surface);
}

LVKW_WaylandDecorationMode _lvkw_wayland_get_decoration_mode(
    const LVKW_ContextCreateInfo *create_info) {
  return create_info->tuning->wayland.decoration_mode;
}

static void _fractional_scale_handle_preferred_scale(
    void *data, struct wp_fractional_scale_v1 *fractional_scale, uint32_t scale) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_WINDOW_ASSUME(data, window != NULL,
                  "Window handle must not be NULL in preferred scale handler");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  window->scale = scale / (LVKW_Scalar)120.0;

  lvkw_wl_surface_set_buffer_scale(ctx, window->wl.surface, 1);

  if (window->base.pub.flags & LVKW_WINDOW_STATE_READY) {
    LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
    lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                        LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window,
                                        &evt);
  }
}

const struct wp_fractional_scale_v1_listener _lvkw_wayland_fractional_scale_listener = {
    .preferred_scale = _fractional_scale_handle_preferred_scale,
};

static void _wl_surface_handle_enter(void *data, struct wl_surface *surface,
                                     struct wl_output *output) {}
static void _wl_surface_handle_leave(void *data, struct wl_surface *surface,
                                     struct wl_output *output) {}

static void _wl_surface_handle_preferred_buffer_scale(void *data, struct wl_surface *surface,
                                                      int32_t factor) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_WINDOW_ASSUME(data, window != NULL,
                  "Window handle must not be NULL in preferred buffer scale handler");

  if (window->ext.fractional_scale) return;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  if (window->scale != (LVKW_Scalar)factor) {
    window->scale = (LVKW_Scalar)factor;
    lvkw_wl_surface_set_buffer_scale(ctx, window->wl.surface, factor);

    if (window->base.pub.flags & LVKW_WINDOW_STATE_READY) {
      LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
      lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                          LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window,
                                          &evt);
    }
  }
}

static void _wl_surface_handle_preferred_buffer_transform(void *data, struct wl_surface *surface,
                                                          uint32_t transform) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  LVKW_WINDOW_ASSUME(data, window != NULL,
                  "Window handle must not be NULL in preferred buffer transform handler");

  if (window->buffer_transform == transform) return;
  window->buffer_transform = transform;

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  lvkw_wl_surface_set_buffer_transform(ctx, window->wl.surface, (int32_t)transform);

  if (window->base.pub.flags & LVKW_WINDOW_STATE_READY) {
    LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
    lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                       LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window,
                                       &evt);
  }
}

const struct wl_surface_listener _lvkw_wayland_surface_listener = {
    .enter = _wl_surface_handle_enter,
    .leave = _wl_surface_handle_leave,
    .preferred_buffer_scale = _wl_surface_handle_preferred_buffer_scale,
    .preferred_buffer_transform = _wl_surface_handle_preferred_buffer_transform};

static uint32_t _lvkw_wayland_clamp_size(uint32_t value, uint32_t min_value, uint32_t max_value) {
  if (value < min_value) return min_value;
  if (value > max_value) return max_value;
  return value;
}

static void _lvkw_wayland_enforce_aspect_ratio(uint32_t *width, uint32_t *height,
                                               const LVKW_Window_WL *window) {
  if (!width || !height) return;
  if (*width == 0 || *height == 0) return;
  if (window->aspect_ratio.numerator == 0 || window->aspect_ratio.denominator == 0) return;

  uint32_t min_width = window->min_size.x > 0 ? (uint32_t)window->min_size.x : 0u;
  uint32_t min_height = window->min_size.y > 0 ? (uint32_t)window->min_size.y : 0u;
  uint32_t max_width = window->max_size.x > 0 ? (uint32_t)window->max_size.x : UINT32_MAX;
  uint32_t max_height = window->max_size.y > 0 ? (uint32_t)window->max_size.y : UINT32_MAX;

  if (max_width < min_width) max_width = min_width;
  if (max_height < min_height) max_height = min_height;

  const uint64_t ratio_num = (uint32_t)window->aspect_ratio.numerator;
  const uint64_t ratio_den = (uint32_t)window->aspect_ratio.denominator;

  for (int pass = 0; pass < 2; ++pass) {
    uint64_t lhs = (uint64_t)(*width) * ratio_den;
    uint64_t rhs = (uint64_t)(*height) * ratio_num;

    if (lhs > rhs) {
      uint64_t adjusted = ((uint64_t)(*height) * ratio_num + (ratio_den / 2u)) / ratio_den;
      *width = (uint32_t)(adjusted > 0 ? adjusted : 1u);
    }
    else if (lhs < rhs) {
      uint64_t adjusted = ((uint64_t)(*width) * ratio_den + (ratio_num / 2u)) / ratio_num;
      *height = (uint32_t)(adjusted > 0 ? adjusted : 1u);
    }

    *width = _lvkw_wayland_clamp_size(*width, min_width, max_width);
    *height = _lvkw_wayland_clamp_size(*height, min_height, max_height);
  }
}

static void _xdg_surface_handle_configure(void *userData, struct xdg_surface *surface,
                                          uint32_t serial) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;
  LVKW_WINDOW_ASSUME(userData, window != NULL,
                  "Window handle must not be NULL in xdg surface configure handler");
  LVKW_WINDOW_ASSUME(userData, surface != NULL, "XDG surface must not be NULL in configure handler");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  lvkw_xdg_surface_ack_configure(ctx, surface, serial);

  if (!(window->base.pub.flags & LVKW_WINDOW_STATE_READY)) {
    window->base.pub.flags |= LVKW_WINDOW_STATE_READY;
    LVKW_Event evt = {0};
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_READY,
                          (LVKW_Window *)window, &evt);
  }
}

const struct xdg_surface_listener _lvkw_wayland_xdg_surface_listener = {
    .configure = _xdg_surface_handle_configure};

static void _xdg_toplevel_handle_configure(void *userData, struct xdg_toplevel *toplevel,
                                           int32_t width, int32_t height, struct wl_array *states) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;
  LVKW_WINDOW_ASSUME(userData, window != NULL,
                  "Window handle must not be NULL in xdg toplevel configure handler");
  LVKW_WINDOW_ASSUME(userData, toplevel != NULL, "XDG toplevel must not be NULL in configure handler");

  bool maximized = false;
  bool fullscreen = false;
  bool focused = false;
  uint32_t *state;
  wl_array_for_each(state, states) {
    if (*state == XDG_TOPLEVEL_STATE_MAXIMIZED) maximized = true;
    if (*state == XDG_TOPLEVEL_STATE_FULLSCREEN) fullscreen = true;
    if (*state == XDG_TOPLEVEL_STATE_ACTIVATED) focused = true;
  }

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  if (window->is_maximized != maximized) {
    window->is_maximized = maximized;
    if (maximized)
      window->base.pub.flags |= LVKW_WINDOW_STATE_MAXIMIZED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_MAXIMIZED;

    LVKW_Event evt = {0};
    evt.maximized.maximized = maximized;
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_MAXIMIZED,
                          (LVKW_Window *)window, &evt);
  }

  if (window->is_fullscreen != fullscreen) window->is_fullscreen = fullscreen;
  if (fullscreen)
    window->base.pub.flags |= LVKW_WINDOW_STATE_FULLSCREEN;
  else
    window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_FULLSCREEN;

  bool old_focused = (window->base.pub.flags & LVKW_WINDOW_STATE_FOCUSED) != 0;
  if (old_focused != focused) {
    if (focused)
      window->base.pub.flags |= LVKW_WINDOW_STATE_FOCUSED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_FOCUSED;

    LVKW_Event evt = {0};
    evt.focus.focused = focused;
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_FOCUS,
                          (LVKW_Window *)window, &evt);
  }

  bool size_changed = false;
  if (width != 0 || height != 0) {
    uint32_t pending_width = width > 0 ? (uint32_t)width : (uint32_t)window->size.x;
    uint32_t pending_height = height > 0 ? (uint32_t)height : (uint32_t)window->size.y;

    if (ctx->enforce_client_side_constraints && !maximized && !fullscreen) {
      _lvkw_wayland_enforce_aspect_ratio(&pending_width, &pending_height, window);
    }

    if (pending_width != window->size.x || pending_height != window->size.y) {
      window->size.x = pending_width;
      window->size.y = pending_height;
      _lvkw_wayland_update_opaque_region(window);
      size_changed = true;
    }
  }

  if (size_changed || !(window->base.pub.flags & LVKW_WINDOW_STATE_READY)) {
    LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
    lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                        LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window,
                                        &evt);
  }
}

static void _xdg_toplevel_handle_close(void *userData, struct xdg_toplevel *toplevel) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)userData;
  LVKW_WINDOW_ASSUME(userData, window != NULL,
                  "Window handle must not be NULL in xdg toplevel close handler");
  LVKW_WINDOW_ASSUME(userData, toplevel != NULL, "XDG toplevel must not be NULL in close handler");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  LVKW_Event evt = {0};
  lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_CLOSE_REQUESTED,
                        (LVKW_Window *)window, &evt);
}

const struct xdg_toplevel_listener _lvkw_wayland_xdg_toplevel_listener = {
    .configure = _xdg_toplevel_handle_configure,
    .close = _xdg_toplevel_handle_close,
    .configure_bounds = NULL,
    .wm_capabilities = NULL};

static void _xdg_decoration_handle_configure(void *data,
                                             struct zxdg_toplevel_decoration_v1 *decoration,
                                             uint32_t mode) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)data;
  if (!window) return;

  switch (mode) {
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
      window->is_decorated = true;
      break;
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
      window->is_decorated = false;
      break;
    default:
      break;
  }
  (void)decoration;
}

const struct zxdg_toplevel_decoration_v1_listener _lvkw_wayland_xdg_decoration_listener = {
    .configure = _xdg_decoration_handle_configure,
};

static void _libdecor_frame_handle_configure(struct libdecor_frame *frame,
                                             struct libdecor_configuration *configuration,
                                             void *user_data) {
  LVKW_Window_WL *window = (LVKW_Window_WL *)user_data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  int width, height;

  enum libdecor_window_state state_flags;
  bool maximized = false;
  bool fullscreen = false;
  bool focused = false;

  if (lvkw_libdecor_configuration_get_window_state(ctx, configuration, &state_flags)) {
    if (state_flags & LIBDECOR_WINDOW_STATE_MAXIMIZED) maximized = true;
    if (state_flags & LIBDECOR_WINDOW_STATE_FULLSCREEN) fullscreen = true;
    if (state_flags & LIBDECOR_WINDOW_STATE_ACTIVE) focused = true;
  }

  if (window->is_maximized != maximized) {
    window->is_maximized = maximized;
    if (maximized)
      window->base.pub.flags |= LVKW_WINDOW_STATE_MAXIMIZED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_MAXIMIZED;

    LVKW_Event evt = {0};
    evt.maximized.maximized = maximized;
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_MAXIMIZED,
                          (LVKW_Window *)window, &evt);
  }

  if (window->is_fullscreen != fullscreen) window->is_fullscreen = fullscreen;
  if (fullscreen)
    window->base.pub.flags |= LVKW_WINDOW_STATE_FULLSCREEN;
  else
    window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_FULLSCREEN;

  bool old_focused = (window->base.pub.flags & LVKW_WINDOW_STATE_FOCUSED) != 0;
  if (old_focused != focused) {
    if (focused)
      window->base.pub.flags |= LVKW_WINDOW_STATE_FOCUSED;
    else
      window->base.pub.flags &= (uint32_t)~LVKW_WINDOW_STATE_FOCUSED;

    LVKW_Event evt = {0};
    evt.focus.focused = focused;
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_FOCUS,
                          (LVKW_Window *)window, &evt);
  }

  if (!lvkw_libdecor_configuration_get_content_size(ctx, configuration, frame, &width, &height)) {
    width = (int)window->size.x;
    height = (int)window->size.y;
  }
  window->libdecor.last_configuration = configuration;

  uint32_t content_width = width > 0 ? (uint32_t)width : (uint32_t)window->size.x;
  uint32_t content_height = height > 0 ? (uint32_t)height : (uint32_t)window->size.y;

  if (ctx->enforce_client_side_constraints && !maximized && !fullscreen) {
    _lvkw_wayland_enforce_aspect_ratio(&content_width, &content_height, window);
  }

  window->size.x = content_width;
  window->size.y = content_height;

  struct libdecor_state *state =
      lvkw_libdecor_state_new(ctx, (int)content_width, (int)content_height);
  lvkw_libdecor_frame_commit(ctx, frame, state, configuration);
  lvkw_libdecor_state_free(ctx, state);

  _lvkw_wayland_update_opaque_region(window);

  if (!(window->base.pub.flags & LVKW_WINDOW_STATE_READY)) {
    window->base.pub.flags |= LVKW_WINDOW_STATE_READY;
    LVKW_Event evt = {0};
    lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_READY,
                          (LVKW_Window *)window, &evt);
  }

  LVKW_Event evt = _lvkw_wayland_make_window_resized_event(window);
  lvkw_event_queue_push_compressible(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue,
                                      LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)window,
                                      &evt);
}

static void _libdecor_frame_handle_close(struct libdecor_frame *frame, void *user_data) {
  (void)frame;
  LVKW_Window_WL *window = (LVKW_Window_WL *)user_data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_Event evt = {0};
  lvkw_event_queue_push(&ctx->linux_base.base, &ctx->linux_base.base.prv.event_queue, LVKW_EVENT_TYPE_CLOSE_REQUESTED,
                        (LVKW_Window *)window, &evt);
}

static void _libdecor_frame_handle_commit(struct libdecor_frame *frame, void *user_data) {
  (void)frame;
  LVKW_Window_WL *window = (LVKW_Window_WL *)user_data;
  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;
  lvkw_wl_surface_commit(ctx, window->wl.surface);
}

static struct libdecor_frame_interface _libdecor_frame_interface = {
    .configure = _libdecor_frame_handle_configure,
    .close = _libdecor_frame_handle_close,
    .commit = _libdecor_frame_handle_commit,
};

static uint32_t _lvkw_wayland_map_content_type(LVKW_ContentType content_type) {
  switch (content_type) {
    case LVKW_CONTENT_TYPE_PHOTO:
      return WP_CONTENT_TYPE_V1_TYPE_PHOTO;
    case LVKW_CONTENT_TYPE_VIDEO:
      return WP_CONTENT_TYPE_V1_TYPE_VIDEO;
    case LVKW_CONTENT_TYPE_GAME:
      return WP_CONTENT_TYPE_V1_TYPE_GAME;
    case LVKW_CONTENT_TYPE_NONE:
    default:
      return WP_CONTENT_TYPE_V1_TYPE_NONE;
  }
}

bool _lvkw_wayland_create_xdg_shell_objects(LVKW_Window_WL *window,
                                            const LVKW_WindowCreateInfo *create_info) {
  LVKW_WINDOW_ASSUME(window, window != NULL,
                  "Window handle must not be NULL in create_xdg_shell_objects");

  LVKW_Context_WL *ctx = (LVKW_Context_WL *)window->base.prv.ctx_base;

  LVKW_WINDOW_ASSUME(window, ctx != NULL, "Context must not be NULL in create_xdg_shell_objects");
  LVKW_WINDOW_ASSUME(window, window->wl.surface != NULL,
                  "Wayland surface must not be NULL in create_xdg_shell_objects");

  LVKW_WaylandDecorationMode mode = ctx->decoration_mode;

  bool try_ssd =
      (mode == LVKW_WAYLAND_DECORATION_MODE_AUTO || mode == LVKW_WAYLAND_DECORATION_MODE_SSD);
  bool try_csd =
      (mode == LVKW_WAYLAND_DECORATION_MODE_AUTO || mode == LVKW_WAYLAND_DECORATION_MODE_CSD);

  if (try_ssd && ctx->protocols.opt.zxdg_decoration_manager_v1) {
    window->xdg.surface = lvkw_xdg_wm_base_get_xdg_surface(
        ctx, ctx->protocols.xdg_wm_base, window->wl.surface);
    if (!window->xdg.surface) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN,
                                  "xdg_wm_base_get_xdg_surface() failed");
      return false;
    }

    lvkw_xdg_surface_add_listener(ctx, window->xdg.surface,
                                  &_lvkw_wayland_xdg_surface_listener, window);

    window->xdg.toplevel = lvkw_xdg_surface_get_toplevel(ctx, window->xdg.surface);
    if (!window->xdg.toplevel) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN,
                                  "xdg_surface_get_toplevel() failed");
      return false;
    }

    lvkw_xdg_toplevel_add_listener(ctx, window->xdg.toplevel,
                                   &_lvkw_wayland_xdg_toplevel_listener, window);

    if (create_info->attributes.title) {
      lvkw_xdg_toplevel_set_title(ctx, window->xdg.toplevel,
                                  create_info->attributes.title);
    }
    else {
      lvkw_xdg_toplevel_set_title(ctx, window->xdg.toplevel, "Lvkw");
    }

    if (create_info->app_id) {
      lvkw_xdg_toplevel_set_app_id(ctx, window->xdg.toplevel, create_info->app_id);
    }

    window->xdg.decoration = lvkw_zxdg_decoration_manager_v1_get_toplevel_decoration(
        ctx, ctx->protocols.opt.zxdg_decoration_manager_v1, window->xdg.toplevel);
    lvkw_zxdg_toplevel_decoration_v1_add_listener(ctx, window->xdg.decoration,
                                                  &_lvkw_wayland_xdg_decoration_listener, window);
    lvkw_zxdg_toplevel_decoration_v1_set_mode(
        ctx, window->xdg.decoration,
        window->is_decorated ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                             : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);

    if (create_info->attributes.fullscreen) {
      struct wl_output *target_output =
          _lvkw_wayland_find_monitor(ctx, create_info->attributes.monitor);
      lvkw_xdg_toplevel_set_fullscreen(ctx, window->xdg.toplevel, target_output);
    }
    else if (create_info->attributes.maximized) {
      lvkw_xdg_toplevel_set_maximized(ctx, window->xdg.toplevel);
    }

    window->decor_mode = LVKW_WAYLAND_DECORATION_MODE_SSD;
  }

  if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_AUTO && try_csd && ctx->libdecor.ctx) {
    window->libdecor.frame =
        lvkw_libdecor_decorate(ctx, ctx->libdecor.ctx, window->wl.surface,
                               &_libdecor_frame_interface, window);

    if (window->libdecor.frame) {
      if (create_info->attributes.title) {
        lvkw_libdecor_frame_set_title(ctx, window->libdecor.frame,
                                      create_info->attributes.title);
      }
      else {
        lvkw_libdecor_frame_set_title(ctx, window->libdecor.frame, "Lvkw");
      }

      if (create_info->app_id) {
        lvkw_libdecor_frame_set_app_id(ctx, window->libdecor.frame,
                                       create_info->app_id);
      }

      if (create_info->attributes.fullscreen) {
        struct wl_output *target_output =
            _lvkw_wayland_find_monitor(ctx, create_info->attributes.monitor);
        lvkw_libdecor_frame_set_fullscreen(ctx, window->libdecor.frame,
                                           target_output);
      }
      else if (create_info->attributes.maximized) {
        lvkw_libdecor_frame_set_maximized(ctx, window->libdecor.frame);
      }

      enum libdecor_capabilities caps =
          LIBDECOR_ACTION_CLOSE | LIBDECOR_ACTION_MOVE | LIBDECOR_ACTION_MINIMIZE;
      if (window->is_resizable) {
        caps |= LIBDECOR_ACTION_RESIZE | LIBDECOR_ACTION_FULLSCREEN;
      }
      lvkw_libdecor_frame_set_capabilities(ctx, window->libdecor.frame, caps);
      lvkw_libdecor_frame_set_visibility(ctx, window->libdecor.frame, window->is_decorated);

      lvkw_libdecor_frame_map(ctx, window->libdecor.frame);
      window->decor_mode = LVKW_WAYLAND_DECORATION_MODE_CSD;
    }
  }

  if (window->decor_mode == LVKW_WAYLAND_DECORATION_MODE_AUTO) {
    // Fallback to no decorations (raw xdg_shell)
    window->xdg.surface = lvkw_xdg_wm_base_get_xdg_surface(
        ctx, ctx->protocols.xdg_wm_base, window->wl.surface);
    if (!window->xdg.surface) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN,
                                  "xdg_wm_base_get_xdg_surface() failed");
      return false;
    }

    lvkw_xdg_surface_add_listener(ctx, window->xdg.surface,
                                  &_lvkw_wayland_xdg_surface_listener, window);

    window->xdg.toplevel = lvkw_xdg_surface_get_toplevel(ctx, window->xdg.surface);
    if (!window->xdg.toplevel) {
      LVKW_REPORT_WIND_DIAGNOSTIC(window, LVKW_DIAGNOSTIC_UNKNOWN,
                                  "xdg_surface_get_toplevel() failed");
      return false;
    }

    lvkw_xdg_toplevel_add_listener(ctx, window->xdg.toplevel,
                                   &_lvkw_wayland_xdg_toplevel_listener, window);

    if (create_info->attributes.title) {
      lvkw_xdg_toplevel_set_title(ctx, window->xdg.toplevel,
                                  create_info->attributes.title);
    }
    else {
      lvkw_xdg_toplevel_set_title(ctx, window->xdg.toplevel, "Lvkw");
    }

    if (create_info->app_id) {
      lvkw_xdg_toplevel_set_app_id(ctx, window->xdg.toplevel, create_info->app_id);
    }

    if (create_info->attributes.fullscreen) {
      struct wl_output *target_output =
          _lvkw_wayland_find_monitor(ctx, create_info->attributes.monitor);
      lvkw_xdg_toplevel_set_fullscreen(ctx, window->xdg.toplevel, target_output);
    }
    else if (create_info->attributes.maximized) {
      lvkw_xdg_toplevel_set_maximized(ctx, window->xdg.toplevel);
    }

    window->decor_mode = LVKW_WAYLAND_DECORATION_MODE_NONE;
  }

  _lvkw_wayland_apply_size_constraints(window);

  if (ctx->protocols.opt.wp_viewporter) {
    window->ext.viewport = lvkw_wp_viewporter_get_viewport(
        ctx, ctx->protocols.opt.wp_viewporter, window->wl.surface);
  }

  if (ctx->protocols.opt.wp_fractional_scale_manager_v1) {
    window->ext.fractional_scale = lvkw_wp_fractional_scale_manager_v1_get_fractional_scale(
        ctx, ctx->protocols.opt.wp_fractional_scale_manager_v1, window->wl.surface);
    lvkw_wp_fractional_scale_v1_add_listener(ctx, window->ext.fractional_scale,
                                             &_lvkw_wayland_fractional_scale_listener, window);
  }

  if (ctx->protocols.opt.wp_content_type_manager_v1) {
    window->ext.content_type = lvkw_wp_content_type_manager_v1_get_surface_content_type(
        ctx, ctx->protocols.opt.wp_content_type_manager_v1, window->wl.surface);
    if (window->ext.content_type) {
      lvkw_wp_content_type_v1_set_content_type(
          ctx, window->ext.content_type, _lvkw_wayland_map_content_type(create_info->content_type));
    }
  }

  _lvkw_wayland_update_opaque_region(window);

  if (ctx->linux_base.inhibit_idle && ctx->protocols.opt.zwp_idle_inhibit_manager_v1) {
    window->ext.idle_inhibitor = lvkw_zwp_idle_inhibit_manager_v1_create_inhibitor(
        ctx, ctx->protocols.opt.zwp_idle_inhibit_manager_v1, window->wl.surface);
  }

  return true;
}
