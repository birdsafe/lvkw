# LVKW

LVKW is a modern, lightweight library that provides cross-platform OS support for Vulkan-centric applications and games. It is built obsessively to be **as efficient as a custom, handwritten backend** while providing everything needed to build a complete App or game with a pleasant to use API.

**Language Standards:** 
- For **using** LVKW: C11 / C++20 or later.
- For **compiling** LVKW: C23 or later.

**Supported Platforms:**
- **Linux:** Wayland & X11 (Runtime selection)
- **Windows:** Win32
- **macOS:** Cocoa (Work in Progress)

## Integration

The easiest way to integrate LVKW is via CMake (`FetchContent` or `add_subdirectory`).

```cmake
include(FetchContent)
FetchContent_Declare(lvkw 
  GIT_REPOSITORY https://github.com/birdsafe/lvkw.git 
  GIT_TAG <target_version>
)
FetchContent_MakeAvailable(lvkw)
target_link_libraries(your_target PRIVATE lvkw::lvkw)
```

For detailed instructions, including **Prebuilding Binaries**, **System Dependencies**, and **Advanced Vulkan Loading** (e.g. dynamic/custom loaders), please consult the [Integration Guide](docs/user_guide/integration.md).

## Examples

### C++
```cpp
// N.B. lvkw does not include vulkan headers for you. And it does not care if you include them before or after it.
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
      .attributes = {
        .title = "LVKW Example", 
        .logicalSize = {800, 600}
      },
      .app_id = "example.lvkw",
      .content_type = LVKW_CONTENT_TYPE_GAME,
  };

  lvkw::Window window = ctx.createWindow(window_info);
  MyRenderEngine engine;

  bool keep_going = true;

  while (keep_going) {
    // N.B. Events will be implicitly masked by which overloads are present.
    // i.e. If you don't pass in a lvkw::MouseMotionEvent callback, the event 
    // won't be polled for.
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
// N.B. lvkw does not include vulkan headers for you. And it does not care if you include them before or after it.
#include <vulkan/vulkan.h>
#include "lvkw/lvkw.h"

// Globals for brevity in this example, everything can be passed around via userdata
bool keep_going = true;
bool ready = false;
VkInstance vk_instance = NULL;
VkSurfaceKHR vk_surface = NULL;

void on_event(LVKW_EventType type, LVKW_Window *w, const LVKW_Event *e, void *u) {
  switch (type) {
    case LVKW_EVENT_TYPE_WINDOW_READY: {
       lvkw_wnd_createVkSurface(w, vk_instance, &vk_surface);
       // Init renderer with surface...
       ready = true;
       break;
    }
    case LVKW_EVENT_TYPE_CLOSE_REQUESTED: keep_going = false; break;
    // ... handle input ...
  }
}

int main() {
  LVKW_ContextCreateInfo ctx_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
  LVKW_Context *ctx;
  if (lvkw_createContext(&ctx_info, &ctx) != LVKW_SUCCESS) return 1;

  // Get required Vulkan extensions
  uint32_t ext_count;
  const char** extensions;
  lvkw_ctx_getVkExtensions(ctx, &ext_count, &extensions);

  // Initialize Vulkan (create instance using extensions)
  // vk_instance = ...

  LVKW_WindowCreateInfo w_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
  w_info.attributes.title = "LVKW C Example";
  w_info.attributes.logicalSize = (LVKW_LogicalVec){800, 600};
  
  LVKW_Window *window;
  if (lvkw_ctx_createWindow(ctx, &w_info, &window) != LVKW_SUCCESS) return 1;

  while (keep_going) {
    lvkw_ctx_pollEvents(ctx, LVKW_EVENT_TYPE_ALL, on_event, NULL);
    if(ready) {
       // draw stuff
    }
  }

  vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);
  lvkw_wnd_destroy(window);
  lvkw_ctx_destroy(ctx);
  return 0;
}
```

Consult the examples/ directory for more.

### Safety & Validation

LVKW provides a few different options to control the validation behavior. These are all fixed at the time of compiling the library. The complete list can be found in the root CMakeLists, but here are the most relevant ones for the majority of cases:

* **LVKW_VALIDATE_API_CALLS**: Off by default. Enabling it will activate VERY aggressive validation of the API use. This includes thread affinity checks, state validation, the works. Only suitable during development.
* **LVKW_ENABLE_DIAGNOSTICS**: On by default, disabling it will strip virtually all logging and reporting overhead from the library, including the string literals. You should probably only use this for a final release or benchmarking.

#### Recommended configurations

| Build Type | `LVKW_VALIDATE_API_CALLS` | `LVKW_ENABLE_DIAGNOSTICS` | `LVKW_ENABLE_INTERNAL_CHECKS` | Description |
| :--- | :---: | :---: | :---: | :--- |
| **Development** | `ON` | `ON` | `OFF` | Catch API misuse and get detailed error messages. |
| **Release** | `OFF` | `ON` | `OFF` | High performance while maintaining diagnostic capabilities. |
| **Production** | `OFF` | `OFF` | `OFF` | Minimum overhead and binary size (strips all strings). |
| **Debugging lvkw** | `ON` | `ON` | `ON` | Enable internal sanity checks |

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

LVKW provides a flexible threading model designed for high-performance engines:

- **Thread-Bound (Default):** All operations on a context and its windows must occur on the thread that created the context. In debug builds, this is strictly enforced via abort-on-violation.
- **Hybrid Model (Opt-in):** By passing `LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API` during context creation, specific API functions (like attribute updates, geometry queries, and controller haptics) may be called from any thread.
    - **Main-Thread Bound:** Context/Window creation and destruction, and event polling/waiting MUST still occur on the creator thread.
    - **External Synchronization:** When using the hybrid model, the user is **required** to provide external synchronization (e.g., a mutex) to ensure no two threads enter the LVKW context concurrently.
- **Multi-Context Parallelism:** You can safely run multiple independent LVKW contexts in separate threads. Each context manages its own event loop and resources.

### The "Hot path"

The library internally distinguishes between API methods that are in the hot vs cold paths
when weighting space vs time tradeoffs. Hot path methods have a very strong bias in favor of time, wheras cold-path methods are weighted, within reason, in favor of space.

That does not mean that invoking a cold path method will result in a performance hitch. Any given cold-path method will remain safe to invoke occasionally. But if you find yourself invoking a cold-path method every single frame, you are likely not using the library how it was intended, and that should be treated as a code smell.

This is tagged in the headers using the `LVKW_HOT` and `LVKW_COLD` macros.

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

**No.** The event data passed to your callback is transient and you should assume it will become invalid after your callback returns. The data may **appear** to remain valid for longer under certain backends, so don't be fooled. If you need to process an event later, you **must** copy it into your own structures.

### how does scaling (HIDPI) work?

All coordinate variables have either `logical` or `pixel` in their name to make crystal clear if they are referring to logical OS dimensions or to the actual pixel count. In short, use `logical` units for your UI and `pixel` for rendering.

### why is there no `setWindowPosition()`?

Modern display servers (like Wayland) increasingly forbid clients from positioning themselves. By omitting this, we avoid implementing heavy, platform-specific hacks that are often ignored by the OS anyway.

### does LVKW drop mouse events?

LVKW performs **tail-compression** on certain types of events. If multiple motion events arrive before you call `pollEvents()`, they are merged into a single event containing the latest absolute position and the accumulated delta. This prevents high-polling-rate mice from saturating your event loop. However, this only applies to consecutive events. So a mouse_move -> button -> mouse_move. will remain three separate events, no matter how close-together they are.

Additionally, LVKW has a hard-cap per context (defaults to **4096 events**). If this limit is reached, the library will aggressively evict the oldest compressible motion events to make room for newer ones. Reaching this limit would require stalling your application without polling events for thousands of frames, or extreme system-wide event floods. 

This limit can be tuned via `LVKW_ContextAdvancedOptions::max_event_capacity` during context creation if your specific application requires more (or less) buffer.

## Acknowledgements

While there has been a lot of divergence since, a lot of the design and code was originally inspired by, if not at times lifted from the outstanding [GLFW](glfw.org). At the time of writing, credit goes to Marcus Geelnard (2002-2006) and Camilla Löwy (2006-2019)

The examples' code are derived from [Vulkan Tutorial](https://github.com/Overv/VulkanTutorial), but with modifications.

## License

LVKW is licensed under the Zlib license. See [LICENSE.md](LICENSE.md) for details.