# LVKW

LVKW is a small but (eventually) feature-complete library for cross-platform window and input management specifically designed for Vulkan applications. It provides a lightweight alternative to libraries like GLFW, focusing solely on Vulkan support without legacy baggage.

All it does is platform-agnostic Window and input management for Vulkan. That's it. No timers, no threads, no networking.

## Integration

To use LVKW in your project, add it as a subdirectory in your `CMakeLists.txt` and link against the `lvkw::lvkw` target.

```cmake
add_subdirectory(path/to/lvkw)
target_link_libraries(your_target PRIVATE lvkw::lvkw)
```

## Usage Examples

Frankly, it's fundamentally difficult to write a simple and quick example making use of the vulkan API.

So I'll invite you to go see `examples/basic_c/main.c` and `examples/basic_cpp/main.cpp`, which are fully
functional (if simple) vulkan applications but are designed to show how to use lvkw with as few 
distractions as possible.

But here's a taste:

```cpp

#include "lvkw/lvkw.hpp"

int main() {
  lvkw::Context ctx([](const LVKW_DiagnosisInfo *info, void * /* userdata */) { 
    std::cerr << "lvkw Diagnosis: " << info->message << " (Code: " << (int)info->diagnosis << ")" << std::endl; 
  });

  auto extensions = ctx.getVulkanInstanceExtensions();
  VkInstance vk_instance = /* ... */;

  LVKW_WindowCreateInfo window_info = {
        .title = "LVKW Example",
        .app_id = "example.lvkw",
        .size = {800, 600},
        .content_type = LVKW_CONTENT_TYPE_GAME,
        .user_data = nullptr,
  };
  
  lvkw::Window window(ctx, window_info);
  MyRenderEngine engine;

  bool keep_going = true;
  
  while (keep_going) {
    ctx.pollEvents(
      [&](const LVKW_WindowReadyEvent &) {
        auto surface = window.createVkSurface(instance);
        engine.init(surface);
      },
      [&](const LVKW_WindowCloseEvent &) { state.keep_going = false; },
      [&](const LVKW_KeyboardEvent &evt) { /*...*/ },
      [&](const LVKW_MouseMotionEvent &evt) { /*...*/}
      // etc...
    );

    if(engine.ready()) {
      // draw stuff
    }
  }

  return 0;
}
```

The C API is effectively the equivalent.

## Documentation

For detailed API documentation, please consult the header files directly:
- C API: [`include/lvkw/lvkw.h`](include/lvkw/lvkw.h)
- C++ API: [`include/lvkw/lvkw.hpp`](include/lvkw/lvkw.hpp)

The root [`CMakeLists.txt`](CMakeLists.txt) is also kept as legible as possible, so it can serve as 
documention for CMake integration.

These headers are as self-documenting as possible. Once the library is more complete, more 
formal documentation will be produced.

## Library status

Ready for early adopters! 

While the API is still very much in flux. I wouldn't use it yet in a *product* yet, but the library 
is perfectly usable for Vulkan sandboxes and experiments.

There are also a number of core features missing. Notably:
- Monitor handling
- Text input
- Gamepad handling
- Blocking event polling.
- Touch support
- OS Icon support

But we want to really lock down the core API before expanding on it.

### Backends

- **Wayland:** Feature Complete. Pretty happy with diagnosis reporting.
- **X11:** Feature Complete. Needs cleanup.
- **Win32:** Feature Complete. Needs cleanup.
- **Cocoa:** Not implemented. Contributions welcome.



## Acknowledgements

A lot of the design and code is directly inspired, if not at times lifted from the outstanding [GLFW](glfw.org). At the time of writing, credit goes to Marcus Geelnard (2002-2006) and Camilla Löwy (2006-2019)

The examples' code are derived from [Vulkan Tutorial](https://github.com/Overv/VulkanTutorial), but with modifications.