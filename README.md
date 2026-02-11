# LVKW

LVKW is a modern, lightweight library for cross-platform window and input management, built with a single objective: **to be as fast and ergonomic as a custom, handwritten backend.**

Specifically designed for Vulkan applications, LVKW provides a high-performance alternative to libraries like GLFW, stripping away legacy baggage and general-purpose bloat to focus solely on what matters for modern rendering engines.

All it does is platform-agnostic Window and input management for Vulkan. That's it. No timers, no threads, no networking. Just raw, direct, and minimal-overhead control.

## Integration

### Within a CMake project
To use LVKW in your project, add it as a subdirectory in your `CMakeLists.txt` and link against one of the following targets:

- `lvkw::lvkw`: The recommended target. On Linux, this enables **indirect dispatching**, allowing the library to automatically pick between Wayland and X11 at runtime for the cost of marginal overhead. On platforms with a single backend (e.g win32), that ovehead is never present.
- `lvkw::wayland` / `lvkw::x11` (Linux only): Link directly against a specific backend to eliminate indirect dispatching overhead. This enables direct, inlineable calls to the backend logic.

```cmake
add_subdirectory(path/to/lvkw)

find_package(Vulkan Required)

target_link_libraries(your_target PRIVATE lvkw::lvkw Vulkan::Vulkan)

# OR: For zero-overhead direct backend linking (only relevant on linux)
# target_link_libraries(your_target PRIVATE lvkw::wayland)
```

Those targets are all static libraries.

### Safety & Validation

LVKW provides intensive validation of parameters and preconditions when built with `LVKW_ENABLE_DEBUG_DIAGNOSTICS=ON`. In this mode, violations are reported through your diagnostic callback and will trigger an abort to prevent undefined behavior. 

For higher-level language bindings or custom validation needs, the library also provides `lvkw/lvkw_checked.h`, which wraps the core API with runtime checks that return `LVKW_ERROR` instead of aborting.

## Usage Examples

### C++
```cpp
#include <vulkan/vulkan.h>
#include "lvkw/lvkw.hpp"

int main() {
  LVKW_ContextCreateInfo ctx_info = {};
  ctx_info.attributes.diagnostic_cb = [](const LVKW_DiagnosticInfo *info, void *) {
    std::cerr << "Diagnostic: " << info->message << " (Code: " << (int)info->diagnostic << ")" << std::endl;
  };
  lvkw::Context ctx(ctx_info);

  auto extensions = ctx.getVkExtensions();
  VkInstance vk_instance = /* ... */;

  LVKW_WindowCreateInfo window_info = {
      .attributes = {.title = "LVKW Example", .logicalSize = {800, 600}},
      .app_id = "example.lvkw",
      .content_type = LVKW_CONTENT_TYPE_GAME,
  };

  lvkw::Window window = ctx.createWindow(window_info);
  MyRenderEngine engine;

  bool keep_going = true;

  while (keep_going) {
    ctx.pollEvents(
      [&](lvkw::WindowReadyEvent) {
        auto surface = window.createVkSurface(vk_instance);
        engine.init(surface);
      },
      [&](lvkw::WindowCloseEvent) { keep_going = false; },
      [&](lvkw::KeyboardEvent evt) { /*...*/ },
      [&](lvkw::MouseMotionEvent evt) { /*...*/ }
    );

    if (engine.ready()) {
      // draw stuff
    }
  }

  return 0;
}
```

### C
```c
#include <vulkan/vulkan.h>
#include "lvkw/lvkw.h"

bool keep_going = true;

void on_diagnostic(const LVKW_DiagnosticInfo *info, void *userdata) {
  fprintf(stderr, "Diagnostic: %s (Code: %d)\n", info->message, info->diagnostic);
}

void on_event(const LVKW_Event *event, void *userdata) {
  switch (event->type) {
    case LVKW_EVENT_TYPE_WINDOW_READY:
      // Create Vulkan surface, initialize renderer, etc.
      break;
    case LVKW_EVENT_TYPE_CLOSE_REQUESTED:
      keep_going = false;
      break;
    case LVKW_EVENT_TYPE_KEY:
      if (event->key.key == LVKW_KEY_ESCAPE && event->key.state == LVKW_BUTTON_STATE_PRESSED)
        keep_going = false;
      break;
    default: break;
  }
}

int main() {
  LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  ctx_info.attributes.diagnostic_cb = on_diagnostic;

  LVKW_Context *ctx = NULL;
  if (lvkw_createContext(&ctx_info, &ctx) != LVKW_SUCCESS) return 1;

  LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  window_info.attributes.title = "LVKW Example";
  window_info.attributes.logicalSize = (LVKW_LogicalVec){800, 600};
  window_info.content_type = LVKW_CONTENT_TYPE_GAME;

  LVKW_Window *window = NULL;
  if (lvkw_ctx_createWindow(ctx, &window_info, &window) != LVKW_SUCCESS) {
    lvkw_ctx_destroy(ctx);
    return 1;
  }

  while (keep_going) {
    lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, on_event, NULL);
    // draw stuff
  }

  lvkw_wnd_destroy(window);
  lvkw_ctx_destroy(ctx);
  return 0;
}
```

Consult the examples/ directory for more.

## Feature Matrix

The following table tracks the implementation progress and release-readiness (including robustness and testing) of various features across the supported backends.

| Feature | API | Wayland | X11 | Win32 | Cocoa |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **Window & Surface** | | | | | |
| Window Lifecycle | 100% | 95% | 90% | 5% | 5% |
| Vulkan Surface Creation | 100% | 95% | 95% | 5% | 5% |
| Window Focus Tracking | 100% | 90% | 80% | 0% | 0% |
| Fullscreen Toggling | 100% | 90% | 80% | 0% | 0% |
| Transparency Support | 100% | 90% | 85% | 0% | 0% |
| **Input Management** | | | | | |
| Event Polling/Waiting | 100% | 95% | 95% | 5% | 5% |
| Keyboard (State & Events) | 100% | 90% | 90% | 5% | 0% |
| Mouse Motion (Accelerated) | 100% | 95% | 95% | 0% | 0% |
| Mouse Motion (Raw/Dedicated) | 100% | 90% | 85% | 0% | 0% |
| Mouse Buttons & Scroll | 100% | 95% | 95% | 0% | 0% |
| Cursor Shapes & Modes | 100% | 90% | 80% | 0% | 0% |
| Controller / Gamepad | 100% | 80% | 80% | 0% | 0% |
| TextInput (UTF-8) | 100% | 85% | 75% | 0% | 0% |
| **System & Environment** | | | | | |
| Monitor & Video Modes | 100% | 90% | 85% | 5% | 5% |
| HiDPI / Scaling Support | 100% | 95% | 80% | 0% | 0% |
| Clipboard (UTF-8 Text) | 100% | 85% | 80% | 0% | 0% |
| Idle Notification/Inhibition| 100% | 90% | 80% | 0% | 0% |
| **Planned (Roadmap)** | | | | | |
| Window Icons & Custom Cursors | 0% | 0% | 0% | 0% | 0% |
| Window Constraints & State | 0% | 0% | 0% | 0% | 0% |
| Drag and Drop | 0% | 0% | 0% | 0% | 0% |
| IME Support (Composition) | 0% | 0% | 0% | 0% | 0% |
| Controller Haptics/Rumble | 0% | 0% | 0% | 0% | 0% |

## Library status

Ready for early adopters! 

The API is still prone to changing a bit. I wouldn't use it yet in a *product* yet, but the library 
is perfectly usable for Vulkan sandboxes and experiments.

## Documentation

The public headers and root CMakeLists.txt are meant to contain nothing but user-relevant information. As such, they
can serve as reference guides in of themselves.

- C API: [`include/lvkw/lvkw.h`](include/lvkw/lvkw.h)
- C++ API: [`include/lvkw/lvkw.hpp`](include/lvkw/lvkw.hpp)
- Root [`CMakeLists.txt`](CMakeLists.txt)


### Threading Model

- **Thread Affinity:** Each context and its associated windows are **thread-bound**. All API calls involving a context or its windows must be made from the thread that created that context. In debug builds, this is strictly enforced.
- **Multi-Context Parallelism:** You can safely run multiple LVKW contexts in separate threads. Each context is independent and manages its own event loop and resources.
- **Future Thread-Safe Backends:** The current backends are non-thread-safe by design to minimize overhead. However, the architecture allows for the implementation of thread-safe backends (e.g. `wayland_mt`) should the need arise. 

If your project requires a thread-safe backend, please feel free to open an issue and share your use case!

### The "Hot path"

The library internally distinguishes between API methods that are in the hot vs cold paths
when weighting space vs time tradeoffs. Hot path methods have a very strong bias in favor of time, wheras cold-path methods are weighted, within reason, in favor of space.

That does not mean that invoking a cold path method will result in a performance hitch. Any given cold-path method will remain safe to invoke occasionally. But if you find yourself invoking a cold-path method every single frame, you are likely not using the library how it was intended, and that should be treated as a code smell.

This is tagged in the headers using `@lvkw_path cold|hot`

## FAQs

### Having to wait for LVKW_WindowReadyEvent is a #!&%^& pain

Sure, but it's necessary to get the absolute smoothest experience in multi-window applications. And synchronous window creation is just too sticky of an API, so the library corrals users into the "right" way by default.

In any case, window creation can easily be made synchronous on your side of the fence. If you don't mind dropping a few frames on window creation, nothing's stopping you from writing a dedicated `pollEvents()` loop just to wait for the window to be ready.

```cpp
LVKW_WindowCreateInfo wci = {};
wci.attributes.title = "C++ Test Window";
wci.attributes.logicalSize = {1024, 768};

auto window = ctx.createWindow(wci);

// Manual blocking wait for the windowReadyEvent
bool ready = false;
while(!ready) {
  ctx.pollEvents([&](const lvkw::WindowReadyEvent&) {ready = true;});
}

auto surface = window.createVkSurface(vk_instance)
```

### What's up with the attribute substructs in the createInfos, what goes in them seems arbitrary.

Some properties of context/windows must be set at creation, and others can be changed on the fly later. Attributes represent the later, and the same struct type is used when populating the create infos and when invoking `lvkw_[ctx|wnd]_update()`. That makes things nice and consistent.

By and large, only things that we know we can change on the fly on every backend gets to be an attribute. So transparency is out, for example, because there's at least one backend that requires re-creating the window to change it.

### can I store event pointers for later?

**No.** The event data passed to your callback is transient and you should assume it will become invalid after your callback returns. Under `LVKW_ENABLE_DEBUG_DIAGNOSTICS`, the event will be overwritten at that time for safety. However, in release builds, the data may **appear** to remain valid for longer under certain backends, so don't be fooled. If you need to process an event later, you **must** copy it into your own structures.

### how does scaling (HIDPI) work?

All coordinate variables have either `logical` or `pixel` in their name to make crystal clear if they are referring to logical OS dimensions or to the actual pixel count. In short, use `logical` units for your UI and `pixel` for rendering.

### why is there no `setWindowPosition()`?

Modern display servers (like Wayland) increasingly forbid clients from positioning themselves. By omitting this, we avoid implementing heavy, platform-specific hacks that are often ignored by the OS anyway.

### does LVKW drop mouse events?

LVKW performs **tail-compression** on mouse motion and scroll events. If multiple motion events arrive before you call `pollEvents()`, they are merged into a single event containing the latest absolute position and the accumulated delta. This prevents high-polling-rate mice from saturating your event loop. However, this only applies to consecutive events. So a mouse_move -> button -> mouse_move. will remain three separate events, no matter how close-together they are.

Additionally, LVKW has a hard-cap per context (defaults to **4096 events**). If this limit is reached, the library will aggressively evict the oldest compressible motion events to make room for newer ones. Reaching this limit would require stalling your application without polling events for thousands of frames, or extreme system-wide event floods. 

This limit can be tuned via `LVKW_ContextAdvancedOptions::max_event_capacity` during context creation if your specific application requires more (or less) buffer.

## Acknowledgements

A lot of the design and code is directly inspired, if not at times lifted from the outstanding [GLFW](glfw.org). At the time of writing, credit goes to Marcus Geelnard (2002-2006) and Camilla Löwy (2006-2019)

The examples' code are derived from [Vulkan Tutorial](https://github.com/Overv/VulkanTutorial), but with modifications.