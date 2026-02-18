# Advanced Configuration & Tuning

The `LVKW_ContextTuning` struct exposes internal hooks for advanced users. Most applications should stick to `LVKW_CONTEXT_TUNING_DEFAULT`, but understanding these options can solve niche problems.

## 1. Core Tuning

These options affect the fundamental behavior of the library and are relevant across all platforms.

### Event Queue (`events`)

The event queue's size determines how many events can be buffered between `commitEvents` calls.

**Parameters (`LVKW_EventTuning`):**

*   **`initial_capacity`:** Starting size (default 64). Lower values reduce memory usage for simple apps.
*   **`max_capacity`:** Hard limit (default 4096). Events are dropped (after compression attempts) if this is reached. Increase for high-frequency input (e.g., 8000Hz mice) or infrequent polling.
*   **`growth_factor`:** Multiplier when resizing (default 2.0).

**Usage:**

```c
create_info.tuning.events.initial_capacity = 128;
create_info.tuning.events.max_capacity = 8192;
```

**Optimization Tip:**
You can use the [Metrics system](metrics.md) to monitor your actual `peak_count` and see if the queue is growing or dropping events. For competitive gaming or low-latency applications, pre-allocate a large capacity (`initial_capacity = max_capacity`) to avoid any runtime allocation during gameplay.

## 2. Integration Overrides

These settings allow you to override how LVKW interacts with the system environment.

### Custom Vulkan Loading (`vk_loader`)

By default, LVKW assumes your application is linked against the system Vulkan library (`libvulkan.so`, `vulkan-1.lib`) to resolve the one single vulkan function pointer it needs: `vkGetInstanceProcAddr`. If you set a `vk_loader` pointer, this will be bypassed entirely and it will be used instead.

**Why Change This:**
*   You use a custom loader like `volk` or `glad`.
*   You bundle a local Vulkan runtime and don't want to depend on system libraries.

**How To:**
Pass your loader function pointer in `LVKW_ContextCreateInfo`:

```c
LVKW_ContextCreateInfo create_info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;

// Assuming you have a function matching the signature:
// void (*LVKW_VkGetInstanceProcAddrFunc)(VkInstance instance, const char *pName);
create_info.tuning.vk_loader = my_get_instance_proc_addr;

lvkw_context_create(&create_info, &ctx);
```

Note that if you do not link against a library providing `vkGetInstanceProcAddr` AND do not provide a vk_loader, you will get a runtime error that will be reported as a `LVKW_DIAGNOSTIC_VULKAN_FAILURE`.

## 3. Backend-Specific Tuning

These options only affect specific platforms. They are ignored on other platforms.

### Wayland (`wayland`)

Wayland handles window decorations (title bars, minimize/close buttons) differently than X11 or Windows.

**Modes (`decoration_mode`):**

*   **`AUTO` (Default):** Tries to use Server-Side Decorations (SSD - drawn by the compositor/desktop environment). If not supported (e.g., GNOME), falls back to Client-Side Decorations (CSD).
*   **`SSD`:** Force Server-Side Decorations. If the compositor doesn't support the `xdg-decoration` protocol, your window might have *no* decorations.
*   **`CSD`:** Force Client-Side Decorations. Use this if you want consistent, toolkit-styled decorations regardless of the compositor, or if SSD support is buggy.
*   **`NONE`:** Disable all decorations. Useful for full-screen games or custom UIs where you handle window controls entirely.

**Fallback Behavior:**
Explicit modes (`SSD`, `CSD`) are **strict**.
*   If you request `SSD` but the compositor doesn't support it, the result is `NONE` (no decorations).
*   If you request `CSD` but `libdecor` fails to initialize, the result is `NONE`.
*   Only `AUTO` will attempt to switch strategies (SSD -> CSD) to ensure decorations are present.

**Usage:**

```c
create_info.tuning.wayland.decoration_mode = LVKW_WAYLAND_DECORATION_MODE_CSD;
```

**DND post-drop timeout (`dnd_post_drop_timeout_ms`):**

Controls how long LVKW keeps waiting for an asynchronous `text/uri-list` payload after a Wayland drop event.

*   Default is `1000` (via `LVKW_CONTEXT_TUNING_DEFAULT`).
*   The value is used as-is by the backend.
*   `0` means immediate timeout (no grace period).

```c
create_info.tuning.wayland.dnd_post_drop_timeout_ms = 1500;
```

## 4. Performance: The "Hot path"

The library internally distinguishes between API methods that are in the "hot" vs "cold" paths when weighting space vs time tradeoffs.

*   **Hot Path:** Methods (like `pumpEvents`, `commitEvents`, `scanEvents`, `getGeometry`) have a very strong bias in favor of execution time.
*   **Cold Path:** Methods (like `createContext`, `updateAttributes`) are weighted, within reason, in favor of binary space.

That does not mean that invoking a cold path method will result in a performance hitch. Any given cold-path method remains safe to invoke occasionally. However, if you find yourself invoking a cold-path method every single frame, you are likely not using the library as intended, and it should be treated as a code smell.

This is tagged in the headers using the `LVKW_HOT` and `LVKW_COLD` macros.

## 5. Extension Tuning

*This section is reserved for tuning parameters related to optional extensions (e.g., Controller deadzones, Audio buffers).*

*Currently, no extensions expose tuning parameters.*
