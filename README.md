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

LVKW provides intensive validation of parameters and preconditions when built with `LVKW_ENABLE_DEBUG_DIAGNOSIS=ON`. In this mode, violations are reported through your diagnosis callback and will trigger an abort to prevent undefined behavior. 

For higher-level language bindings or custom validation needs, the library also provides `lvkw/lvkw_checked.h`, which wraps the core API with runtime checks that return `LVKW_ERROR` instead of aborting.

## Usage Examples

### C++
```cpp
#include <vulkan/vulkan.h>
#include "lvkw/lvkw.hpp"

int main() {
  LVKW_ContextCreateInfo ctx_info = {};
  ctx_info.attributes.diagnosis_cb = [](const LVKW_DiagnosisInfo *info, void *) {
    std::cerr << "Diagnosis: " << info->message << " (Code: " << (int)info->diagnosis << ")" << std::endl;
  };
  lvkw::Context ctx(ctx_info);

  auto extensions = ctx.getVulkanInstanceExtensions();
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

void on_diagnosis(const LVKW_DiagnosisInfo *info, void *userdata) {
  fprintf(stderr, "Diagnosis: %s (Code: %d)\n", info->message, info->diagnosis);
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
  ctx_info.attributes.diagnosis_cb = on_diagnosis;

  LVKW_Context *ctx = NULL;
  if (lvkw_createContext(&ctx_info, &ctx) != LVKW_SUCCESS) return 1;

  LVKW_WindowCreateInfo window_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  window_info.attributes.title = "LVKW Example";
  window_info.attributes.logicalSize = (LVKW_Size){800, 600};
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

## Threading Model

LVKW follows a strict but clear threading model:

- **Thread Affinity:** Each context and its associated windows are **thread-bound**. All API calls involving a context or its windows must be made from the thread that created that context. In debug builds, this is strictly enforced.
- **Multi-Context Parallelism:** You can safely run multiple LVKW contexts in separate threads. Each context is independent and manages its own event loop and resources.
- **Future Thread-Safe Backends:** The current backends are non-thread-safe by design to minimize overhead. However, the architecture allows for the implementation of thread-safe backends (e.g. `wayland_mt`) should the need arise. 

If your project requires a thread-safe backend, please feel free to open an issue and share your use case!

## Documentation

The public headers and root CMakeLists.txt are meant to contain nothing but user-relevant information. As such, they
can serve as reference guides in of themselves.

- C API: [`include/lvkw/lvkw.h`](include/lvkw/lvkw.h)
- C++ API: [`include/lvkw/lvkw.hpp`](include/lvkw/lvkw.hpp)
- Root [`CMakeLists.txt`](CMakeLists.txt)

## Library status

Ready for early adopters! 

The API is still prone to changing a bit. I wouldn't use it yet in a *product* yet, but the library 
is perfectly usable for Vulkan sandboxes and experiments.

There are also a number of core features still missing. Notably:
- Monitor handling: In development right now.
- Text input
- Gamepad handling
- Touch support
- OS Icon support

But we want to really lock down the core API before expanding on it.

### Backends

- **Wayland:** Feature Complete.
- **X11:** Feature Complete. Needs cleanup.
- **Win32:** Stubbed-in for CI validation, but not implemented yet.
- **Cocoa:** Stubbed-in for CI validation, but not implemented yet.

## FAQs

### Having to wait for LVKW_WindowReadyEvent is a #!&%^& pain

Sure, but it's necessary to get the absolute smoothest experience in multi-window applications. And synchronous window creation is just too sticky of an API.

If you are building a single-window application that you don't mind dropping a few frames on startup, nothing's stopping yo from writing a dedicated `pollEvents()` loop just to wait for the window.

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

Do note that this WILL consume events from other windows as well, so it's only suited for single-window applications.

### What's up with the attribute substructs in the createInfos, what goes in them seems a bit arbitrary.

Some properties of context/windows must be set at creation, and others can be changed on the fly later. Attributes represent the later, and the same struct type is used when populating the create infos and when invoking `lvkw_[ctx|wnd]_update()`. That mkaes things nice and consistent.

By and large, only things that we know we can change on the fly on every backend gets to be an attribute. So transparency is out, for example, because there's at least one backend that requires re-creating the window to change it.

### can I store event pointers for later?

**No.** The event data passed to your callback is transient and often lives on the stack or in a reusable ring buffer. If you need to process an event later (e.g., in a deferred command buffer), you must copy the data into your own structures.

### how does scaling (HIDPI) work?

LVKW uses a **Logical by Default** strategy. All window attributes and input events (mouse positions, scroll deltas) use logical units. All rendering-related queries (Vulkan surface size, swapchain extents) use pixel counts. This ensures your UI logic stays DPI-independent while your rendering remains pixel-perfect.

### why is there no `setWindowPosition()`?

SModern display servers (like Wayland) increasingly forbid clients from positioning themselves. By omitting this, we avoid implementing heavy, platform-specific hacks that are often ignored by the OS anyway.

### does LVKW drop mouse events?

LVKW performs **tail-compression** on mouse motion and scroll events. If multiple motion events arrive before you call `pollEvents()`, they are merged into a single event containing the latest absolute position and the accumulated delta. This prevents high-polling-rate mice from saturating your event loop. However, this only applies to consecutive events. So a mouse_move -> button -> mouse_move. will remain three separate events, no matter how close-together they are.

Additionally, LVKW has a hard-cap per context (defaults to **4096 events**). If this limit is reached, the library will aggressively evict the oldest compressible motion events to make room for newer ones. Reaching this limit would require stalling your application without polling events for thousands of frames, or extreme system-wide event floods. 

This limit can be tuned via `LVKW_ContextAdvancedOptions::max_event_capacity` during context creation if your specific application requires more (or less) buffer.

## Acknowledgements

A lot of the design and code is directly inspired, if not at times lifted from the outstanding [GLFW](glfw.org). At the time of writing, credit goes to Marcus Geelnard (2002-2006) and Camilla Löwy (2006-2019)

The examples' code are derived from [Vulkan Tutorial](https://github.com/Overv/VulkanTutorial), but with modifications.