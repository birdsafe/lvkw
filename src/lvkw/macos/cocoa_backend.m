// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "api_constraints.h"
#include "macos_internal.h"

#include <vulkan/vulkan.h>

@interface LVKWWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) LVKW_Window_Cocoa *window;
@end

@implementation LVKWWindowDelegate

- (BOOL)windowShouldClose:(NSWindow *)sender {
  (void)sender;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)self.window->base.prv.ctx_base;
  LVKW_Event event = {0};
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_CLOSE_REQUESTED, (LVKW_Window *)self.window, &event);
  return NO; // We handle closing manually via lvkw_display_destroyWindow
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
  (void)notification;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)self.window->base.prv.ctx_base;
  self.window->base.pub.flags |= LVKW_WINDOW_STATE_FOCUSED;
  LVKW_Event event = {.focus = {.focused = true}};
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_FOCUS, (LVKW_Window *)self.window, &event);
}

- (void)windowDidResignKey:(NSNotification *)notification {
  (void)notification;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)self.window->base.prv.ctx_base;
  self.window->base.pub.flags &= ~(uint32_t)LVKW_WINDOW_STATE_FOCUSED;
  LVKW_Event event = {.focus = {.focused = false}};
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_FOCUS, (LVKW_Window *)self.window, &event);
}

- (void)windowDidResize:(NSNotification *)notification {
  (void)notification;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)self.window->base.prv.ctx_base;
  
  if ([self.window->window isZoomed]) {
      self.window->base.pub.flags |= LVKW_WINDOW_STATE_MAXIMIZED;
  } else {
      self.window->base.pub.flags &= ~(uint32_t)LVKW_WINDOW_STATE_MAXIMIZED;
  }

  LVKW_Event event = {0};
  lvkw_wnd_getGeometry_Cocoa((LVKW_Window *)self.window, &event.resized.geometry);
  lvkw_event_queue_push_compressible(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_RESIZED, (LVKW_Window *)self.window, &event);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification {
  (void)notification;
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification {
  (void)notification;
  self.window->base.pub.flags |= LVKW_WINDOW_STATE_FULLSCREEN;
}

- (void)windowDidExitFullScreen:(NSNotification *)notification {
  (void)notification;
  self.window->base.pub.flags &= ~(uint32_t)LVKW_WINDOW_STATE_FULLSCREEN;
}

@end

@interface LVKWContentView : NSView
@end

@implementation LVKWContentView
- (BOOL)wantsUpdateLayer { return YES; }
+ (Class)layerClass { return [CAMetalLayer class]; }
- (CALayer *)makeBackingLayer { return [CAMetalLayer layer]; }
- (BOOL)acceptsFirstResponder { return YES; }

// Overriding these prevents the OS from "bleeping" when keys/buttons are pressed,
// as it signals the event has been handled by the responder chain.
- (void)keyDown:(NSEvent *)event { (void)event; }
- (void)keyUp:(NSEvent *)event { (void)event; }
- (void)flagsChanged:(NSEvent *)event { (void)event; }
- (void)mouseDown:(NSEvent *)event { (void)event; }
- (void)mouseUp:(NSEvent *)event { (void)event; }
- (void)rightMouseDown:(NSEvent *)event { (void)event; }
- (void)rightMouseUp:(NSEvent *)event { (void)event; }
- (void)otherMouseDown:(NSEvent *)event { (void)event; }
- (void)otherMouseUp:(NSEvent *)event { (void)event; }
- (void)scrollWheel:(NSEvent *)event { (void)event; }
@end

static LVKW_Key _lvkw_translate_key(unsigned short scancode) {
  static const LVKW_Key table[128] = {
    [0x00] = LVKW_KEY_A, [0x01] = LVKW_KEY_S, [0x02] = LVKW_KEY_D, [0x03] = LVKW_KEY_F,
    [0x04] = LVKW_KEY_H, [0x05] = LVKW_KEY_G, [0x06] = LVKW_KEY_Z, [0x07] = LVKW_KEY_X,
    [0x08] = LVKW_KEY_C, [0x09] = LVKW_KEY_V, [0x0B] = LVKW_KEY_B, [0x0C] = LVKW_KEY_Q,
    [0x0D] = LVKW_KEY_W, [0x0E] = LVKW_KEY_E, [0x0F] = LVKW_KEY_R, [0x10] = LVKW_KEY_Y,
    [0x11] = LVKW_KEY_T, [0x12] = LVKW_KEY_1, [0x13] = LVKW_KEY_2, [0x14] = LVKW_KEY_3,
    [0x15] = LVKW_KEY_4, [0x16] = LVKW_KEY_6, [0x17] = LVKW_KEY_5, [0x18] = LVKW_KEY_EQUAL,
    [0x19] = LVKW_KEY_9, [0x1A] = LVKW_KEY_7, [0x1B] = LVKW_KEY_MINUS, [0x1C] = LVKW_KEY_8,
    [0x1D] = LVKW_KEY_0, [0x1E] = LVKW_KEY_RIGHT_BRACKET, [0x1F] = LVKW_KEY_O, [0x20] = LVKW_KEY_U,
    [0x21] = LVKW_KEY_LEFT_BRACKET, [0x22] = LVKW_KEY_I, [0x23] = LVKW_KEY_P, [0x24] = LVKW_KEY_ENTER,
    [0x25] = LVKW_KEY_L, [0x26] = LVKW_KEY_J, [0x27] = LVKW_KEY_APOSTROPHE, [0x28] = LVKW_KEY_K,
    [0x29] = LVKW_KEY_SEMICOLON, [0x2A] = LVKW_KEY_BACKSLASH, [0x2B] = LVKW_KEY_COMMA, [0x2C] = LVKW_KEY_SLASH,
    [0x2D] = LVKW_KEY_N, [0x2E] = LVKW_KEY_M, [0x2F] = LVKW_KEY_PERIOD, [0x30] = LVKW_KEY_TAB,
    [0x31] = LVKW_KEY_SPACE, [0x32] = LVKW_KEY_GRAVE_ACCENT, [0x33] = LVKW_KEY_BACKSPACE, [0x35] = LVKW_KEY_ESCAPE,
    [0x37] = LVKW_KEY_LEFT_META, [0x38] = LVKW_KEY_LEFT_SHIFT, [0x39] = LVKW_KEY_CAPS_LOCK, [0x3A] = LVKW_KEY_LEFT_ALT,
    [0x3B] = LVKW_KEY_LEFT_CONTROL, [0x3C] = LVKW_KEY_RIGHT_SHIFT, [0x3D] = LVKW_KEY_RIGHT_ALT, [0x3E] = LVKW_KEY_RIGHT_CONTROL,
    [0x3F] = LVKW_KEY_RIGHT_META, [0x41] = LVKW_KEY_KP_DECIMAL, [0x43] = LVKW_KEY_KP_MULTIPLY, [0x45] = LVKW_KEY_KP_ADD,
    [0x47] = LVKW_KEY_NUM_LOCK, [0x4B] = LVKW_KEY_KP_DIVIDE, [0x4C] = LVKW_KEY_KP_ENTER, [0x4E] = LVKW_KEY_KP_SUBTRACT,
    [0x51] = LVKW_KEY_KP_EQUAL, [0x52] = LVKW_KEY_KP_0, [0x53] = LVKW_KEY_KP_1, [0x54] = LVKW_KEY_KP_2,
    [0x55] = LVKW_KEY_KP_3, [0x56] = LVKW_KEY_KP_4, [0x57] = LVKW_KEY_KP_5, [0x58] = LVKW_KEY_KP_6,
    [0x59] = LVKW_KEY_KP_7, [0x5B] = LVKW_KEY_KP_8, [0x5C] = LVKW_KEY_KP_9, [0x60] = LVKW_KEY_F5,
    [0x61] = LVKW_KEY_F6, [0x62] = LVKW_KEY_F7, [0x63] = LVKW_KEY_F3, [0x64] = LVKW_KEY_F8,
    [0x65] = LVKW_KEY_F9, [0x67] = LVKW_KEY_F11, [0x6D] = LVKW_KEY_F10, [0x6F] = LVKW_KEY_F12,
    [0x72] = LVKW_KEY_INSERT,
    [0x73] = LVKW_KEY_HOME, [0x74] = LVKW_KEY_PAGE_UP, [0x75] = LVKW_KEY_DELETE, [0x76] = LVKW_KEY_F4,
    [0x77] = LVKW_KEY_END, [0x78] = LVKW_KEY_F2, [0x79] = LVKW_KEY_PAGE_DOWN, [0x7A] = LVKW_KEY_F1,
    [0x7B] = LVKW_KEY_LEFT, [0x7C] = LVKW_KEY_RIGHT, [0x7D] = LVKW_KEY_DOWN, [0x7E] = LVKW_KEY_UP,
  };

  return (scancode < 128) ? table[scancode] : LVKW_KEY_UNKNOWN;
}

static LVKW_ModifierFlags _lvkw_translate_modifiers(NSEventModifierFlags flags) {
  LVKW_ModifierFlags mods = 0;
  if (flags & NSEventModifierFlagShift) mods |= LVKW_MODIFIER_SHIFT;
  if (flags & NSEventModifierFlagControl) mods |= LVKW_MODIFIER_CONTROL;
  if (flags & NSEventModifierFlagOption) mods |= LVKW_MODIFIER_ALT;
  if (flags & NSEventModifierFlagCommand) mods |= LVKW_MODIFIER_META;
  if (flags & NSEventModifierFlagCapsLock) mods |= LVKW_MODIFIER_CAPS_LOCK;
  if (flags & NSEventModifierFlagNumericPad) mods |= LVKW_MODIFIER_NUM_LOCK;
  return mods;
}

static void _lvkw_process_event(LVKW_Context_Cocoa *ctx, NSEvent *nsEvent) {
  NSWindow *nsWindow = [nsEvent window];
  LVKW_Window_Cocoa *window = NULL;

  if (nsWindow) {
    id delegate = [nsWindow delegate];
    if ([delegate isKindOfClass:[LVKWWindowDelegate class]]) {
      window = ((LVKWWindowDelegate *)delegate).window;
    }
  }

  NSEventType type = [nsEvent type];
  switch (type) {
    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp: {
      if (!window) break;
      LVKW_Event event = {
        .key = {
          .key = _lvkw_translate_key([nsEvent keyCode]),
          .state = (type == NSEventTypeKeyDown) ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED,
          .modifiers = _lvkw_translate_modifiers([nsEvent modifierFlags])
        }
      };
      lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_KEY, (LVKW_Window *)window, &event);
      break;
    }
    case NSEventTypeFlagsChanged: {
      if (!window) break;
      NSEventModifierFlags new_mods = [nsEvent modifierFlags];
      NSEventModifierFlags changed = new_mods ^ ctx->last_modifiers;
      ctx->last_modifiers = new_mods;

      unsigned short scancode = [nsEvent keyCode];
      LVKW_Key key = _lvkw_translate_key(scancode);
      if (key == LVKW_KEY_UNKNOWN) break;

      bool pressed = false;
      // We need to know if the bit corresponding to the key is now set
      // This mapping is a bit loose but works for standard modifiers
      if (scancode == 0x38 || scancode == 0x3C) pressed = (new_mods & NSEventModifierFlagShift) != 0;
      else if (scancode == 0x3B || scancode == 0x3E) pressed = (new_mods & NSEventModifierFlagControl) != 0;
      else if (scancode == 0x3A || scancode == 0x3D) pressed = (new_mods & NSEventModifierFlagOption) != 0;
      else if (scancode == 0x37 || scancode == 0x3F) pressed = (new_mods & NSEventModifierFlagCommand) != 0;
      else if (scancode == 0x39) pressed = (new_mods & NSEventModifierFlagCapsLock) != 0;

      LVKW_Event event = {
        .key = {
          .key = key,
          .state = pressed ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED,
          .modifiers = _lvkw_translate_modifiers(new_mods)
        }
      };
      lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_KEY, (LVKW_Window *)window, &event);
      break;
    }
    case NSEventTypeLeftMouseDown:
    case NSEventTypeLeftMouseUp:
    case NSEventTypeRightMouseDown:
    case NSEventTypeRightMouseUp:
    case NSEventTypeOtherMouseDown:
    case NSEventTypeOtherMouseUp: {
      if (!window) break;
      LVKW_MouseButton button;
      switch ([nsEvent buttonNumber]) {
        case 0: button = LVKW_MOUSE_BUTTON_LEFT; break;
        case 1: button = LVKW_MOUSE_BUTTON_RIGHT; break;
        case 2: button = LVKW_MOUSE_BUTTON_MIDDLE; break;
        default: button = (LVKW_MouseButton)([nsEvent buttonNumber]); break;
      }
      LVKW_Event event = {
        .mouse_button = {
          .button = button,
          .state = (type == NSEventTypeLeftMouseDown || type == NSEventTypeRightMouseDown || type == NSEventTypeOtherMouseDown) 
                    ? LVKW_BUTTON_STATE_PRESSED : LVKW_BUTTON_STATE_RELEASED,
          .modifiers = _lvkw_translate_modifiers([nsEvent modifierFlags])
        }
      };
      lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_BUTTON, (LVKW_Window *)window, &event);
      break;
    }
    case NSEventTypeMouseMoved:
    case NSEventTypeLeftMouseDragged:
    case NSEventTypeRightMouseDragged:
    case NSEventTypeOtherMouseDragged: {
      if (!window) break;
      NSPoint p = [nsEvent locationInWindow];
      NSRect contentRect = [nsWindow contentRectForFrameRect:[nsWindow frame]];
      
      LVKW_Event event = {
        .mouse_motion = {
          .position = { (LVKW_Scalar)p.x, (LVKW_Scalar)(contentRect.size.height - p.y) },
          .delta = { (LVKW_Scalar)[nsEvent deltaX], (LVKW_Scalar)[nsEvent deltaY] },
          .raw_delta = { (LVKW_Scalar)[nsEvent deltaX], (LVKW_Scalar)[nsEvent deltaY] } // TODO: raw motion
        }
      };
      lvkw_event_queue_push_compressible(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_MOTION, (LVKW_Window *)window, &event);
      break;
    }
    case NSEventTypeScrollWheel: {
      if (!window) break;
      LVKW_Event event = {
        .mouse_scroll = {
          .delta = { (LVKW_Scalar)[nsEvent scrollingDeltaX], (LVKW_Scalar)[nsEvent scrollingDeltaY] }
        }
      };
      // NSEvent reporting for scroll is complex (pixels vs lines), 
      // LVKW wants logical units.
      if ([nsEvent hasPreciseScrollingDeltas]) {
          // already logical units?
      } else {
          event.mouse_scroll.delta.x *= (LVKW_Scalar)10.0; // rough estimate
          event.mouse_scroll.delta.y *= (LVKW_Scalar)10.0;
      }
      lvkw_event_queue_push_compressible(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_MOUSE_SCROLL, (LVKW_Window *)window, &event);
      break;
    }
    default:
      break;
  }
}

LVKW_Status lvkw_ctx_create_Cocoa(const LVKW_ContextCreateInfo *create_info, LVKW_Context **out_ctx_handle) {
  LVKW_API_VALIDATE(createContext, create_info, out_ctx_handle);
  *out_ctx_handle = NULL;

  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)lvkw_context_alloc_bootstrap(create_info, sizeof(LVKW_Context_Cocoa));
  if (!ctx) {
    return LVKW_ERROR;
  }
  memset(ctx, 0, sizeof(LVKW_Context_Cocoa));

  if (_lvkw_context_init_base(&ctx->base, create_info) != LVKW_SUCCESS) {
    lvkw_context_free(&ctx->base, ctx);
    return LVKW_ERROR;
  }

  ctx->app = [NSApplication sharedApplication];
  [ctx->app setActivationPolicy:NSApplicationActivationPolicyRegular];
  ctx->last_modifiers = [NSEvent modifierFlags];

  *out_ctx_handle = (LVKW_Context *)ctx;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_destroy_Cocoa(LVKW_Context *ctx_handle) {
  LVKW_API_VALIDATE(ctx_destroy, ctx_handle);
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;

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

LVKW_Status lvkw_ctx_pumpEvents_Cocoa(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;

  @autoreleasepool {
    NSDate *untilDate = nil;
    if (timeout_ms == 0) {
      untilDate = [NSDate distantPast];
    } else if (timeout_ms == LVKW_NEVER) {
      untilDate = [NSDate distantFuture];
    } else {
      untilDate = [NSDate dateWithTimeIntervalSinceNow:timeout_ms / 1000.0];
    }

    NSEvent *event = [ctx->app nextEventMatchingMask:NSEventMaskAny
                                           untilDate:untilDate
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES];
    if (event) {
      _lvkw_process_event(ctx, event);
      [ctx->app sendEvent:event];

      // Drain any other immediately available events
      while ((event = [ctx->app nextEventMatchingMask:NSEventMaskAny
                                             untilDate:[NSDate distantPast]
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES])) {
        _lvkw_process_event(ctx, event);
        [ctx->app sendEvent:event];
      }
    }
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_commitEvents_Cocoa(LVKW_Context *ctx_handle) {
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;
  lvkw_event_queue_begin_gather(&ctx->base.prv.event_queue);
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_postEvent_Cocoa(LVKW_Context *ctx_handle, LVKW_EventType type, LVKW_Window *window,
                                     const LVKW_Event *evt) {
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;

  if (!lvkw_event_queue_push_external(&ctx->base.prv.event_queue, type, window, evt)) {
    return LVKW_ERROR;
  }

  // Wake up Cocoa run loop
  [ctx->app postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                         location:NSMakePoint(0, 0)
                                    modifierFlags:0
                                        timestamp:0
                                     windowNumber:0
                                          context:nil
                                          subtype:0
                                            data1:0
                                            data2:0]
              atStart:NO];
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_scanEvents_Cocoa(LVKW_Context *ctx_handle, LVKW_EventType event_mask,
                                      LVKW_EventCallback callback, void *userdata) {
  LVKW_API_VALIDATE(ctx_scanEvents, ctx_handle, event_mask, callback, userdata);
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)ctx_handle;
  lvkw_event_queue_scan(&ctx->base.prv.event_queue, event_mask, callback, userdata);
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

LVKW_Status lvkw_ctx_getMonitors_Cocoa(LVKW_Context *ctx_handle, LVKW_MonitorRef **out_refs, uint32_t *count) {
  LVKW_API_VALIDATE(ctx_getMonitors, ctx_handle, out_refs, count);
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
  window->base.pub.context = &ctx->base.pub;
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

  window->base.pub.flags |= LVKW_WINDOW_STATE_READY;
  // Push Window Ready event
  LVKW_Event ready_evt = {0};
  lvkw_event_queue_push(&ctx->base, &ctx->base.prv.event_queue, LVKW_EVENT_TYPE_WINDOW_READY, (LVKW_Window *)window, &ready_evt);

  *out_window_handle = (LVKW_Window *)window;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_destroy_Cocoa(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_destroy, window_handle);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;
  LVKW_Context_Cocoa *ctx = (LVKW_Context_Cocoa *)window->base.prv.ctx_base;

  lvkw_event_queue_remove_window_events(&ctx->base.prv.event_queue, window_handle);
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

  out_geometry->logicalSize.x = (LVKW_Scalar)frame.size.width;
  out_geometry->logicalSize.y = (LVKW_Scalar)frame.size.height;
  out_geometry->pixelSize.x = (int32_t)backing.size.width;
  out_geometry->pixelSize.y = (int32_t)backing.size.height;

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_update_Cocoa(LVKW_Window *window_handle, uint32_t field_mask,
                                  const LVKW_WindowAttributes *attributes) {
  LVKW_API_VALIDATE(wnd_update, window_handle, field_mask, attributes);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;
  NSWindow *nsWindow = window->window;

  if (field_mask & LVKW_WINDOW_ATTR_TITLE) {
    [nsWindow setTitle:[NSString stringWithUTF8String:attributes->title]];
  }

  if (field_mask & LVKW_WINDOW_ATTR_LOGICAL_SIZE) {
    NSRect frame = [nsWindow contentRectForFrameRect:[nsWindow frame]];
    frame.size.width = attributes->logicalSize.x;
    frame.size.height = attributes->logicalSize.y;
    [nsWindow setFrame:[nsWindow frameRectForContentRect:frame] display:YES];
  }

  if (field_mask & LVKW_WINDOW_ATTR_FULLSCREEN) {
    bool is_fullscreen = ([nsWindow styleMask] & NSWindowStyleMaskFullScreen) != 0;
    if (is_fullscreen != attributes->fullscreen) {
      [nsWindow toggleFullScreen:nil];
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_MAXIMIZED) {
    bool is_maximized = [nsWindow isZoomed];
    if (is_maximized != attributes->maximized) {
      [nsWindow zoom:nil];
    }
  }

  if (field_mask & LVKW_WINDOW_ATTR_MIN_SIZE) {
    NSSize size = NSMakeSize(attributes->minSize.x, attributes->minSize.y);
    [nsWindow setContentMinSize:size];
  }

  if (field_mask & LVKW_WINDOW_ATTR_MAX_SIZE) {
    NSSize size = NSMakeSize(attributes->maxSize.x, attributes->maxSize.y);
    if (size.width <= 0) size.width = CGFLOAT_MAX;
    if (size.height <= 0) size.height = CGFLOAT_MAX;
    [nsWindow setContentMaxSize:size];
  }

  if (field_mask & LVKW_WINDOW_ATTR_ASPECT_RATIO) {
    if (attributes->aspect_ratio.numerator > 0 && attributes->aspect_ratio.denominator > 0) {
      NSSize ratio = NSMakeSize(attributes->aspect_ratio.numerator, attributes->aspect_ratio.denominator);
      [nsWindow setContentAspectRatio:ratio];
    } else {
      [nsWindow setContentAspectRatio:NSMakeSize(0, 0)]; // Unconstrained
    }
  }

  if (field_mask & (LVKW_WINDOW_ATTR_RESIZABLE | LVKW_WINDOW_ATTR_DECORATED)) {
    NSUInteger styleMask = [nsWindow styleMask];
    
    // We need to keep FullScreen if it's there
    NSUInteger preserved = styleMask & NSWindowStyleMaskFullScreen;

    bool resizable = (field_mask & LVKW_WINDOW_ATTR_RESIZABLE) ? attributes->resizable : (styleMask & NSWindowStyleMaskResizable);
    bool decorated = (field_mask & LVKW_WINDOW_ATTR_DECORATED) ? attributes->decorated : (styleMask & NSWindowStyleMaskTitled);

    styleMask = preserved;
    if (decorated) {
        styleMask |= NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    } else {
        styleMask |= NSWindowStyleMaskBorderless;
    }

    if (resizable) {
        styleMask |= NSWindowStyleMaskResizable;
    }

    [nsWindow setStyleMask:styleMask];
    // Re-setting style mask can sometimes cause the title to disappear or the window to need a frame update
    if (decorated && (field_mask & LVKW_WINDOW_ATTR_TITLE)) {
        [nsWindow setTitle:[NSString stringWithUTF8String:attributes->title]];
    }
  }

  return LVKW_SUCCESS;
}

LVKW_Status lvkw_wnd_requestFocus_Cocoa(LVKW_Window *window_handle) {
  LVKW_API_VALIDATE(wnd_requestFocus, window_handle);
  LVKW_Window_Cocoa *window = (LVKW_Window_Cocoa *)window_handle;
  [window->window makeKeyAndOrderFront:nil];
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_createCursor_Cocoa(LVKW_Context *ctx, const LVKW_CursorCreateInfo *create_info,
                                        LVKW_Cursor **out_cursor) {
  (void)ctx;
  (void)create_info;
  *out_cursor = NULL;
  // TODO: Implement cursor creation
  return LVKW_ERROR;
}

LVKW_Status lvkw_cursor_destroy_Cocoa(LVKW_Cursor *cursor) {
  (void)cursor;
  return LVKW_SUCCESS;
}

LVKW_Status lvkw_ctx_getStandardCursor_Cocoa(LVKW_Context *ctx, LVKW_CursorShape shape, LVKW_Cursor **out_cursor) {
  (void)ctx;
  (void)shape;
  *out_cursor = NULL;
  // TODO: Implement standard cursors
  return LVKW_ERROR;
}
