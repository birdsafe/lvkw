// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "lvkw_api_constraints.h"
#include "lvkw_macos_internal.h"

#include <vulkan/vulkan.h>

@interface LVKWWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) LVKW_Window_Cocoa *window;
@end

@implementation LVKWWindowDelegate

- (BOOL)windowShouldClose:(NSWindow *)sender {
  (void)sender;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)self.window->base.prv.ctx_base;
  LVKW_Event event = {0};
  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, LVKW_EVENT_TYPE_CLOSE_REQUESTED, (LVKW_Window *)self.window, &event);
  return NO; // We handle closing manually via lvkw_wnd_destroy
}

@end

@interface LVKWContentView : NSView
@end

@implementation LVKWContentView
- (BOOL)wantsUpdateLayer { return YES; }
+ (Class)layerClass { return [CAMetalLayer class]; }
- (CALayer *)makeBackingLayer { return [CAMetalLayer layer]; }
@end

static void *_lvkw_default_alloc(size_t size, void *userdata) {
  (void)userdata;
  return malloc(size);
}

static void _lvkw_default_free(void *ptr, void *userdata) {
  (void)userdata;
  free(ptr);
}

LVKW_Status lvkw_ctx_create_Cocoa(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;

  LVKW_Allocator allocator = {.alloc_cb = _lvkw_default_alloc, .free_cb = _lvkw_default_free};
  if (create_info->allocator.alloc_cb) {
    allocator = create_info->allocator;
  }

  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)lvkw_alloc(&allocator, create_info->userdata, sizeof(LVKW_Context_Cocoa));
  if (!ctx) {
    return LVKW_ERROR;
  }
  memset(ctx, 0, sizeof(LVKW_Context_Cocoa));

  _lvkw_context_init_base(&ctx->base, create_info);
  ctx->base.prv.alloc_cb = allocator;

  LVKW_EventTuning event_tuning;
  if (create_info->tuning) {
    event_tuning = create_info->tuning->events;
  } else {
    LVKW_ContextTuning default_tuning = LVKW_CONTEXT_TUNING_DEFAULT;
    event_tuning = default_tuning.events;
  }
  lvkw_event_queue_init(&ctx->base, &ctx->event_queue, event_tuning);

  ctx->app = [NSApplication sharedApplication];
  [ctx->app setActivationPolicy:NSApplicationActivationPolicyRegular];

  *out_ctx_handle = (LVKW_Context *)ctx;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_destroy_Cocoa(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;

  lvkw_event_queue_cleanup(&ctx->base, &ctx->event_queue);
  _lvkw_context_cleanup_base(&ctx->base);
  lvkw_context_free(&ctx->base, ctx);

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getVkExtensions_Cocoa(LVKW_Context *ctx_handle, uint32_t *count,
                                           const char *const **out_extensions) {
  LVKW_API_VALIDATE(ctx_getVkExtensions, ctx_handle, count, out_extensions);
  static const char *extensions[] = {
    "VK_KHR_surface",
    "VK_EXT_metal_surface"
  };

  *count = 2;
  *out_extensions = extensions;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_pollEvents_Cocoa(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata) {
  return lvkw_ctx_waitEvents_Cocoa(ctx_handle, 0, event_mask, callback, userdata);
}

LVKW_Status lvkw_ctx_waitEvents_Cocoa(LVKW_Context *ctx_handle, uint32_t timeout_ms,
                                      LVKW_EventType event_mask, LVKW_EventCallback callback,
                                      void *userdata) {
  LVKW_API_VALIDATE(ctx_waitEvents, ctx_handle, timeout_ms, event_mask, callback, userdata);
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;

  uint64_t start_time = _lvkw_get_timestamp_ms();

  while (true) {
    @autoreleasepool {
      NSEvent *event = nil;
      NSDate *untilDate = nil;

      if (timeout_ms == 0) {
        untilDate = [NSDate distantPast];
      } else if (timeout_ms == LVKW_NEVER) {
        untilDate = [NSDate distantFuture];
      } else {
        uint64_t elapsed = _lvkw_get_timestamp_ms() - start_time;
        if (elapsed >= timeout_ms) {
          untilDate = [NSDate distantPast];
        } else {
          untilDate = [NSDate dateWithTimeIntervalSinceNow:(timeout_ms - elapsed) / 1000.0];
        }
      }

      while ((event = [ctx->app nextEventMatchingMask:NSEventMaskAny
                                             untilDate:untilDate
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES])) {
        [ctx->app sendEvent:event];
        untilDate = [NSDate distantPast]; // Only wait for the first event
      }
    }

    // Dispatch queued events
    LVKW_EventType type;
    LVKW_Window *window;
    LVKW_Event ev;
    bool matched = false;
    while (lvkw_event_queue_pop(&ctx->event_queue, LVKW_EVENT_TYPE_ALL, &type, &window, &ev)) {
      if (event_mask & type) {
        callback(type, window, &ev, userdata);
        matched = true;
      }
    }

    if (matched || timeout_ms == 0) break;

    uint64_t elapsed = _lvkw_get_timestamp_ms() - start_time;
    if (timeout_ms != LVKW_NEVER && elapsed >= timeout_ms) break;
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_update_Cocoa(LVKW_Context *ctx_handle, uint32_t field_mask,
                                  const LVKW_ContextAttributes *attributes) {
  LVKW_API_VALIDATE(ctx_update, ctx_handle, field_mask, attributes);
  (void)ctx_handle;
  (void)field_mask;
  (void)attributes;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitors_Cocoa(LVKW_Context *ctx_handle, LVKW_Monitor **out_monitors, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_monitors, count);
  (void)ctx_handle;
  *count = 0; // TODO: Implement monitor detection
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getMonitorModes_Cocoa(LVKW_Context *ctx_handle, const LVKW_Monitor *monitor,
                                           LVKW_VideoMode *out_modes, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitorModes, ctx_handle, monitor, out_modes, count);
  (void)ctx_handle;
  *count = 0;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_createWindow_Cocoa(LVKW_Context *ctx_handle, const LVKW_WindowCreateInfo *create_info,
                                        LVKW_Window **out_window_handle) {
  LVKW_API_VALIDATE(ctx_createWindow, ctx_handle, create_info, out_window_handle);
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;

  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)lvkw_context_alloc(&ctx->base, sizeof(LVKW_Window_Cocoa));
  if (!window) return LVKW_ERROR;
  memset(window, 0, sizeof(LVKW_Window_Cocoa));

  window->base.prv.ctx_base = &ctx->base;
  window->base.pub.userdata = create_info->userdata;

  NSRect contentRect = NSMakeRect(0, 0, create_info->attributes.logicalSize.x, create_info->attributes.logicalSize.y);
  NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

  window->window = [[NSWindow alloc] initWithContentRect:contentRect
                                               styleMask:styleMask
                                                 backing:NSBackingStoreBuffered
                                                   defer:NO];

  if (!window->window) {
    lvkw_context_free(&ctx->base, window);
    return LVKW_ERROR;
  }

  [window->window setTitle:[NSString stringWithUTF8String:create_info->attributes.title]];
  [window->window setReleasedWhenClosed:NO];

  LVKWWindowDelegate *delegate = [[LVKWWindowDelegate alloc] init];
  delegate.window = window;
  [window->window setDelegate:delegate];

  window->view = [[LVKWContentView alloc] initWithFrame:contentRect];
  [window->view setWantsLayer:YES];
  window->layer = (CAMetalLayer *)[window->view layer];
  
  [window->window setContentView:window->view];
  [window->window makeKeyAndOrderFront:nil];

  _lvkw_window_list_add(&ctx->base, &window->base);

  window->base.pub.flags |= LVKW_WND_STATE_READY;
  // Push Window Ready event
  LVKW_Event ready_evt = {0};
  lvkw_event_queue_push(&ctx->base, &ctx->event_queue, LVKW_EVENT_TYPE_WINDOW_READY, (LVKW_Window *)window, &ready_evt);

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_destroy_Cocoa(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)window->base.prv.ctx_base;

  lvkw_event_queue_remove_window_events(&ctx->event_queue, window_handle);
  _lvkw_window_list_remove(&ctx->base, &window->base);

  id delegate = [window->window delegate];
  [window->window setDelegate:nil];
  [delegate release];

  [window->window close];
  [window->window release];

  lvkw_context_free(&ctx->base, window);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_createVkSurface_Cocoa(LVKW_Window *window_handle, VkInstance instance, VkSurfaceKHR *out_surface) {
  LVKW_API_VALIDATE(wnd_createVkSurface, window_handle, instance, out_surface);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)window->base.prv.ctx_base;

  *out_surface = VK_NULL_HANDLE;

  LVKW_VkGetInstanceProcAddrFunc vk_loader = ctx->base.prv.vk_loader;
  if (!vk_loader) {
    extern __attribute__((weak)) LVKW_VulkanVoidFunction vkGetInstanceProcAddr(VkInstance, const char *);
    vk_loader = (LVKW_VkGetInstanceProcAddrFunc)vkGetInstanceProcAddr;
  }

  if (!vk_loader) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE, "Vulkan loader not found");
    return LVKW_ERROR;
  }

  typedef VkResult (*PFN_vkCreateMetalSurfaceEXT)(VkInstance, const void *, const void *, VkSurfaceKHR *);
  PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)vk_loader(instance, "vkCreateMetalSurfaceEXT");

  if (!vkCreateMetalSurfaceEXT) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE, "vkCreateMetalSurfaceEXT not found");
    return LVKW_ERROR;
  }

  struct {
    int sType;
    const void *pNext;
    uint32_t flags;
    const void *pLayer;
  } create_info = {
    .sType = 1000217000, // VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT
    .pNext = NULL,
    .flags = 0,
    .pLayer = window->layer
  };

  if (vkCreateMetalSurfaceEXT(instance, &create_info, NULL, out_surface) != VK_SUCCESS) {
    LVKW_REPORT_WIND_DIAGNOSTIC(&window->base, LVKW_DIAGNOSTIC_VULKAN_FAILURE, "vkCreateMetalSurfaceEXT failed");
    return LVKW_ERROR;
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_getGeometry_Cocoa(LVKW_Window *window_handle, LVKW_WindowGeometry *out_geometry) {
  LVKW_API_VALIDATE(wnd_getGeometry, window_handle, out_geometry);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;

  NSRect frame = [window->window contentRectForFrameRect:[window->window frame]];
  NSRect backing = [window->window convertRectToBacking:frame];

  out_geometry->logicalSize.x = (LVKW_real_t)frame.size.width;
  out_geometry->logicalSize.y = (LVKW_real_t)frame.size.height;
  out_geometry->pixelSize.x = (int32_t)backing.size.width;
  out_geometry->pixelSize.y = (int32_t)backing.size.height;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_update_Cocoa(LVKW_Window *window_handle, uint32_t field_mask,
                                  const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;

  if (field_mask & LVKW_WND_ATTR_TITLE) {
    [window->window setTitle:[NSString stringWithUTF8String:attributes->title]];
  }

  if (field_mask & LVKW_WND_ATTR_LOGICAL_SIZE) {
    NSRect frame = [window->window frame];
    frame.size.width = attributes->logicalSize.x;
    frame.size.height = attributes->logicalSize.y;
    [window->window setFrame:frame display:YES];
  }

  // TODO: Implement other attributes

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_requestFocus_Cocoa(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;
  [window->window makeKeyAndOrderFront:nil];
  return LVKW_SUCCESS;
}
