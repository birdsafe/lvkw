# Integration Guide

## CMake Integration

The recommended way to use LVKW is via `FetchContent` or `add_subdirectory`. If, for some reason, you need a CMake-friendly system-managed installation of LVKW, feel free to file an issue about it. We simply do not have a use case for it at the moment.

### FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(lvkw
  GIT_REPOSITORY https://github.com/birdsafe/lvkw.git
  GIT_TAG <insert_lvkw_version_tag_here>
)

# Set LVKW options before making it available
set(LVKW_VALIDATE_API_CALLS ON CACHE BOOL "" FORCE)
set(LVKW_ENABLE_DIAGNOSTICS ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(lvkw)

find_package(Vulkan REQUIRED)
target_link_libraries(your_target PRIVATE lvkw::lvkw Vulkan::Vulkan)
```

**Available Targets:**
- `lvkw::lvkw`: Almost certainly what you want. On platforms with uncertain environments (e.g Linux), this adds a level of indirection for runtime selection.
- `lvkw::wayland` / `lvkw::x11`: (Linux only) Direct backend linking for zero-overhead.

### Prebuilt Binaries

To create a local distribution of LVKW (libraries and headers), use the `install` target:

```bash
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../dist -DLVKW_VALIDATE_API_CALLS=OFF
cmake --build . --target install
```

This creates a `dist/` directory with `include/` and `lib/`. You can then manually add these paths to your project.

## System Dependencies

When linking against static LVKW libraries manually, ensure you link the following system dependencies:

- **Linux:** `-lpthread -ldl` (and potentially Wayland/X11 client libraries if not using pkg-config).
- **Windows:** Standard system libraries.
- **macOS:** `-framework Cocoa -framework QuartzCore`

## Vulkan Integration

LVKW requires access to `vkGetInstanceProcAddr` to load platform-specific surface creation functions.

### 1. Standard Linking (Recommended)
Link your application against the Vulkan loader (`libvulkan.so`, `vulkan-1.lib`). LVKW will automatically detect and use the global `vkGetInstanceProcAddr` symbol.

### 2. Dynamic / Custom Loading
If your application loads Vulkan dynamically (e.g., via `dlopen`/`LoadLibrary`) or uses a custom loader, you do not need to link against the Vulkan library.

Instead, pass your loaded function pointer to LVKW during context creation:

```c
LVKW_ContextCreateInfo create_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;

// Setup tuning with your loader function
LVKW_ContextTuning tuning = LVKW_CONTEXT_TUNING_DEFAULT;
tuning.vk_loader = (LVKW_VkGetInstanceProcAddrFunc)my_get_instance_proc_addr;

create_info.tuning = &tuning;

lvkw_context_create(&create_info, &ctx);
```

If `vk_loader` is provided, LVKW will use it exclusively and will not attempt to resolve the system symbol.
