# API Implementation Flow

This document describes how a public API call flows through LVKW, from the user's call to the platform-specific backend logic.

## 1. Public API Entry Point

All public C API functions are defined in `include/lvkw/*.h` and implemented either in `src/lvkw/base/lvkw_base.c` or in platform-specific `lvkw_<platform>.c` files.

### Validation Layer
The first step of any public API call is validation. LVKW uses a shared validation layer defined in `src/lvkw/base/lvkw_api_constraints.h`.

```c
LVKW_Status lvkw_ctx_someFunction(LVKW_Context *ctx, ...) {
  // 1. Validate handles, thread affinity, and arguments.
  LVKW_API_VALIDATE(ctx_someFunction, ctx, ...);

  // ... implementation ...
}
```

- **Thread Affinity**: Validated here to ensure the call is made from the context's creator thread (unless cross-thread access is permitted).
- **Handle Integrity**: Checks that the context or window is not NULL and not in a "LOST" state.

## 2. Dynamic Dispatch (Linux)

On Linux, LVKW supports multiple display protocols (Wayland and X11) in a single binary. This is handled via a virtual function table (vtable).

### The Backend VTable (`LVKW_Backend`)
The vtable is defined in `src/lvkw/base/lvkw_backend.h`. It contains function pointers for all backend-specific operations.

### Flow on Linux
1. **Selection**: During `lvkw_createContext`, LVKW probes for Wayland. If available, it loads the Wayland vtable; otherwise, it falls back to X11.
2. **Storage**: The selected `LVKW_Backend` pointer is stored in the context's private data: `ctx_base->prv.backend`.
3. **Dispatch**: Public API functions in `src/lvkw/linux/lvkw_linux.c` delegate to the vtable.

```c
// Example from src/lvkw/linux/lvkw_linux.c
LVKW_Status lvkw_ctx_syncEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  const LVKW_Context_Base *ctx_base = (const LVKW_Context_Base *)ctx_handle;
  // Route to the selected backend (Wayland or X11)
  return ctx_base->prv.backend->context.sync_events(ctx_handle, timeout_ms);
}
```

## 3. Static Dispatch (Windows / macOS)

On platforms with a single primary backend, dispatching is static. The public API functions call the platform implementation directly.

```c
// Example from src/lvkw/win32/lvkw_win32.c
LVKW_Status lvkw_ctx_syncEvents(LVKW_Context *ctx_handle, uint32_t timeout_ms) {
  LVKW_VALIDATE(ctx_syncEvents, ctx_handle, timeout_ms);
  return lvkw_ctx_syncEvents_Win32(ctx_handle, timeout_ms);
}
```

## 4. Internal Base Helpers

Functions that are truly cross-platform (like `lvkw_getVersion`) or provide shared logic (like `lvkw_wnd_getContext`) are implemented in `src/lvkw/base/lvkw_base.c`.

Backend-specific implementations often call into `_lvkw_*` private functions in the base layer to perform shared tasks like initializing handles or managing window lists.

## 5. Extensions (e.g., Controller)

Extensions provide optional functionality that may not be available on all platforms or builds.

### Pattern: Guarded VTable
1. **Compile-time Guard**: Logic is wrapped in `#ifdef LVKW_ENABLE_EXTENSION`.
2. **Backend Presence**: The vtable includes an optional section for the extension.
3. **Public API Dispatch**: The public API checks if the backend supports the extension before calling.

```c
// Example from src/lvkw/base/lvkw_backend.h
#ifdef LVKW_ENABLE_CONTROLLER
  struct {
    __typeof__(lvkw_ctrl_create) *create;
    // ...
  } ctrl;
#endif
```

## 6. Checklist for Adding a New API Method

When adding a new function `lvkw_ctx_newMethod`:

1.  **Header**: Declare in `include/lvkw/lvkw-context.h`.
2.  **Validation**: Add `_lvkw_api_constraints_ctx_newMethod` in `src/lvkw/base/lvkw_api_constraints.h`.
3.  **VTable**: Add a function pointer to `LVKW_Backend` in `src/lvkw/base/lvkw_backend.h`.
4.  **Linux Dispatch**: Implement `lvkw_ctx_newMethod` in `src/lvkw/linux/lvkw_linux.c` using the vtable.
5.  **Backend Impls**:
    - Implement `lvkw_ctx_newMethod_WL` in `src/lvkw/linux/wayland/wayland_context.c`.
    - Implement `lvkw_ctx_newMethod_X11` in `src/lvkw/linux/x11/x11_context.c`.
    - Implement `lvkw_ctx_newMethod_Win32` (stub or impl) in `src/lvkw/win32/lvkw_win32.c`.
    - Implement `lvkw_ctx_newMethod_Mock` in `tests/mock_backend/mock_context.c`.
6.  **C++ Wrapper**: Add a corresponding wrapper to `lvkw::Context` or as a free function in `include/lvkw/lvkw.hpp` (and `include/lvkw/details/lvkw_hpp_impl.hpp`). If it involves C++20 features, use `include/lvkw/lvkw_cxx20.hpp`.
