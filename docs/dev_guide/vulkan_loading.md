# Vulkan Loading Architecture

LVKW is designed to be flexible regarding how the Vulkan runtime is loaded. It supports both standard linking against the Vulkan loader (SDK) and completely dynamic loading (e.g., for custom engines or when `libvulkan` is not present at link time).

## The Challenge

The library needs to call `vkGetInstanceProcAddr` to load the platform-specific surface creation functions (e.g., `vkCreateWaylandSurfaceKHR`, `vkCreateXlibSurfaceKHR`). However, we want to avoid:
1.  Forcing users to link against `libvulkan` if they don't want to.
2.  Requiring complex compile-time flags to toggle between "linked" and "dynamic" modes.

## The Solution

LVKW uses a hybrid approach combining **Weak Symbols** (on Unix-like systems) and **Runtime Injection**.

### 1. `vk_loader` Tuning Option

The `LVKW_ContextTuning` structure allows users to provide an explicit function pointer for `vkGetInstanceProcAddr`.

```c
LVKW_ContextCreateInfo info = LVKW_CONTEXT_CREATE_INFO_DEFAULT;
// For users with custom loaders:
info.tuning = &(LVKW_ContextTuning){
    .vk_loader = (LVKW_VkGetInstanceProcAddrFunc)my_custom_loader_func
};
```

If this pointer is provided, LVKW uses it exclusively and does not attempt to use the system symbol.

### 2. Weak Symbol Fallback (Linux & macOS)

If `vk_loader` is `NULL` (default), the backend checks for the presence of the `vkGetInstanceProcAddr` symbol in the global namespace. This is achieved using `__attribute__((weak))`.

```c
extern __attribute__((weak)) PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance, const char*);

// In implementation:
if (!vk_loader && vkGetInstanceProcAddr) {
    vk_loader = vkGetInstanceProcAddr;
}
```

- **Linked:** If the user links against `libvulkan`, the symbol is resolved, and `vk_loader` becomes valid.
- **Unlinked:** If the user does *not* link `libvulkan`, the symbol is `NULL`. LVKW detects this and reports a runtime error (unless a manual loader was provided).

### 3. Module Handle Fallback (Windows)

On Windows, weak symbols are not supported in the same way. Instead, the backend relies on `GetModuleHandle`:

1.  Check `vk_loader`.
2.  If `NULL`, call `GetModuleHandle("vulkan-1.dll")`.
3.  If the handle is valid (meaning Vulkan is loaded in the process), use `GetProcAddress` to find `vkGetInstanceProcAddr`.

This covers both implicit linking (where the DLL is loaded at startup) and manual `LoadLibrary` scenarios.

## Summary for Contributors

When implementing surface creation for a new backend:
1.  **Always** check `ctx->base.prv.vk_loader` first.
2.  **Never** assume `vkGetInstanceProcAddr` is non-NULL or valid without checking.
3.  **Do not** add hard compile-time dependencies or `#ifdef` guards for `libvulkan` unless absolutely necessary.
