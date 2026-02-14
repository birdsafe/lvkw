# LVKW

LVKW is a work-in-progress library that provides OS support for Vulkan-centric applications and games. It does one thing and attempts to do it as well as it can: Reliably provide you with Windows and as much OS support as possible to create **consistent** Vulkan-based applications with as light of a footprint as I can squeeze it in, both in terms of CPU and Memory.

For the time being, we have solid (but not 100% complete yet) backends for Wayland and MacOS, and X11 is about 50% done. Win32 is still just stubbed in.

It is being built primarily for my personal needs and satisfaction. Maybe others will find it useful.

**Language Standards:** 
- For **compiling** LVKW: 
  - C11

- For **using** LVKW: 
  - C11
  - C++20

The C++20 requirement might seem a bit steep, but it's necessary for the way it deals with event handlers. If you have no choice but to use an older C++ standard, then the C API is still perfectly usable.

## Key Design driving principles

- No. Global. State. None. Nada.
- Pedantic in Debug, no guardrails in Release.
- You don't pay for what you don't use.
- Self-documentation is king.
- 0 means 0. Nothing is free unless it truly is zero-cost.

## Status

It's ready for use by early adopters, as long as you don't need Windows build today. The core windowing + KB/Mouse handling you need for 90% of gaming applications works just fine on the 3 currently supported targets.

## Integration

The easiest way to integrate LVKW is via CMake (`FetchContent` or `add_subdirectory`).

```cmake
add_subdirectory(path/to/lvkw)
target_link_libraries(your_target PRIVATE lvkw::lvkw)
```

or

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

| Build Type | `LVKW_VALIDATE_API_CALLS` | `LVKW_ENABLE_DIAGNOSTICS` | `LVKW_GATHER_TELEMETRY` | `LVKW_ENABLE_INTERNAL_CHECKS` | Description |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **Development** | `ON` | `ON` | `ON` | `OFF` | Catch API misuse and get detailed error messages. |
| **Release** | `OFF` | `ON` | `ON` | `OFF` | High performance while maintaining diagnostic capabilities. |
| **Production** | `OFF` | `OFF` | `OFF` | `OFF` | Minimum overhead and binary size (strips all strings). |
| **Debugging lvkw** | `ON` | `ON` | `ON` | `ON` | Enable internal sanity checks |


## Documentation

The public headers and root CMakeLists.txt are meant to contain nothing but user-relevant information. As such, they can serve as reference guides in of themselves.

- C API: [`include/lvkw/lvkw.h`](include/lvkw/lvkw.h)
- C++ API: [`include/lvkw/lvkw.hpp`](include/lvkw/lvkw.hpp)
- Root [`CMakeLists.txt`](CMakeLists.txt)

The [User Guide](docs/user_guide/index.md) is not meant to be a full guide, but rather a collection of deep-dives on technical nitty-gritty that might be of interest to advanced users. We expect that the headers and examples should be all the documentation you need to get started. And they are formatted to make reading them directly as pleasant as possible.

## FAQs

### Is LVKW Thread-safe?

**NO**.

lvkw expects you to do everything related to a given context object (function calls and dereferencing pointers) from the same thread that created that context. Different contexts can live on separate threads just fine, though.

However, there is also a way to relax this a bit. See the [Threading Deep Dive](docs/user_guide/threading.md) for more details.

### Does LVKW use X11 or Wayland on Linux?

Yes!

You use either or both (selected automatically at runtime). The default is a one-size-fits-all mode that will try wayland first and fallabck to X11 if it can't use it. All the bindings are loaded dynamically. This means the executable will boot on any Linux machine and then probe for the appropriate display server libraries as needed.

If you want to exclusively support X11 or Wayland, that's also possible, and it eliminates every shred of overhead from the dynamic selection machinery (what little there is). See the [Integration guide](docs/user_guide/integration.md) for details.

### Can I use lvkw for other rendering APIs (OpenGL, DX11/12, etc...)?

Not at the moment. Maybe one day, but don't count on it. That would likely become a separate spin-off project. We want this one to stay laser-focused.

### How does LVKW handle high-frequency mouse input?

LVKW Coalesces redundant events as much as it can. This prevents high-polling-rate mice from overwhelming your application with redundant updates while maintaining sub-pixel precision. And before you ask: Yes, it does so without breaking temporal ordering with key and button events.

There's a whole algorithm around this, how it relates to memory use, etc... See the [Event System & Input Deep Dive](docs/user_guide/events_and_input.md) for more details.

### Does lvkw initialize vulkan for me?

Nope. There's too many decisions to make around that. The library has a mandate: Deal with Windows and I/O, and it sticks to it. It does a grand total of 2 Vulkan-specific things:

1 - Get which extensions you need to provide `vkCreateInstance()` via `lvkw_ctx_getVkExtensions()`
2 - Create a `vkSurfaceKHR` for a given window (which you are responsible for deleting) via `lvkw_wnd_createVkSurface()` 

### Is there no synchronous window creation mechanism?

Unfortunately, asynchronous window creation is absolutely necessary to get a smooth experience in multi-window applications. And synchronous window creation is just too sticky of an API, so the library corrals users into the "right" way by default.

### What's up with the attribute substructs in the createInfos, what goes in them seems arbitrary.

Some properties of context/windows must be set at creation, and others can be changed on the fly later. Attributes represent the later, and the same struct type is used when populating the create infos and when invoking `lvkw_[ctx|wnd]_update()`. That makes things nice and consistent.

By and large, only things that we know we can change on the fly on every backend gets to be an attribute. So `transparency` is a member of the createInfo and not the window attributes, for example, because there's at least one backend that requires re-creating the window to change it.

### Can I store event pointers for later?

**No.** The event data passed to your callback is transient and you should assume it will become invalid after your callback returns. The data may **appear** to remain valid for longer under certain backends, so don't be fooled. If you need to process an event later, you **must** copy it into your own structures.

### What's the difference between logical vectors and pixel vectors

The wording distinction is to make dealing with High DPI (retina) displays easier.

All coordinate variables and types have either `logical` or `pixel` in their name to make crystal clear if they are referring to logical OS dimensions or to the actual pixel count. In short, use `logical` units for your UI and `pixel` for rendering.


### What's up with LVKW_HOT and LVKW_COLD?

The library is built with the assumption that certain api functions might be called every frame (HOT), and others will not (COLD). If you are calling a LVKW_COLD method every frame, you are not using the library the way we expect. That's all there is to it. That doesn't mean COLD function are necessarily heavyweight. It's just how we expect them to be used.

See the [Tuning & Performance Guide](docs/user_guide/tuning.md) for more information.

## Acknowledgements

While there has been a lot of divergence since, a lot of the design and code was originally inspired by, if not at times lifted from the outstanding [GLFW](glfw.org). At the time of writing, credit goes to Marcus Geelnard (2002-2006) and Camilla Löwy (2006-2019)

The examples' code are derived from [Vulkan Tutorial](https://github.com/Overv/VulkanTutorial), but with modifications.

## License

LVKW is licensed under the Zlib license. See [LICENSE.md](LICENSE.md) for details.