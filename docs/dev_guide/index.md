# LVKW Developer Guide

This guide provides a concise overview of the LVKW architecture and internal conventions for library developers.

## 1. Architecture Overview

LVKW uses a layered architecture to support multiple backends (Wayland, X11, Win32) with a unified C API.

- **Public API (`include/lvkw/`)**: Defines handles and the core interface.
- **Internal Base (`src/lvkw/base/`)**: Provides common structures, event queueing, string caching, and base implementations.
- **Backends (`src/lvkw/linux/`, `src/lvkw/win32/`, etc.)**: Implement platform-specific logic.

## 2. Core Handles & Types

LVKW handles (`LVKW_Context*`, `LVKW_Window*`) are pointers to structures that are embedded as the first member of their corresponding internal "Base" structures.

**Note on Opacity:** Handles in LVKW are **semi-opaque by design**. Public structures (like `struct LVKW_Context`) are defined in public headers to allow direct access to common status flags (e.g., `LVKW_WND_STATE_LOST`) and `userdata`, which improves ergonomics and reduces API surface area.

### `LVKW_Context`
- Public handle: `LVKW_Context*`
- Internal structure: `LVKW_Context_Base` (defined in `lvkw_types_internal.h`).
- The `LVKW_Context` struct MUST be the first member of `LVKW_Context_Base`.

### `LVKW_Window`
- Public handle: `LVKW_Window*`
- Internal structure: `LVKW_Window_Base`.
- The `LVKW_Window` struct MUST be the first member of `LVKW_Window_Base`.

**Mandate:** Always use `(LVKW_Context_Base*)ctx` and `(LVKW_Window_Base*)win` to access internal fields.

## 3. Backend Selection & Dispatching (Linux)

Linux supports multiple backends in a single binary using **Indirect Dispatching**.

- **Macro**: `LVKW_INDIRECT_BACKEND` is defined for the Linux build.
- **Selection**: `lvkw_createContext` probes for Wayland first, then X11 (see `src/lvkw/linux/lvkw_linux.c`).
- **Dispatching**: Once a backend is selected, its function table (`LVKW_Backend`, defined in `lvkw_backend.h`) is stored in `LVKW_Context_Base->prv.backend`. API calls are then routed through this table.

## 4. Error Handling & Diagnostics

LVKW uses a "Diagnostics" system instead of just return codes for detailed error reporting.

- **Macros (`lvkw_diagnostic_internal.h`)**:
    - `LVKW_REPORT_CTX_DIAGNOSTIC(ctx_base, diagnostic, msg)`
    - `LVKW_REPORT_WIND_DIAGNOSTIC(window_base, diagnostic, msg)`
- **Assertions & Assumptions (`lvkw_assume.h`)**:
    - `LVKW_CTX_ASSUME`: Internal consistency checks (enabled by `LVKW_ENABLE_INTERNAL_CHECKS`).

## 5. Memory Management

Never use `malloc`/`free` directly. Use the internal wrappers that respect the user-provided allocator.

- **Macros (`lvkw_mem_internal.h`)**:
    - `lvkw_context_alloc(ctx_base, size)`
    - `lvkw_context_free(ctx_base, ptr)`
    - `lvkw_context_alloc_aligned(ctx_base, size)` (for 64-byte alignment).

## 6. Event Management

Events are stored in an `LVKW_EventQueue` (ring buffer) within the context base.

- **Enqueuing**: Backends call `lvkw_event_queue_push`.
- **Merging**: Consecutive small-motion mouse events are automatically merged to prevent queue saturation.
- **Dispatching**: `lvkw_ctx_pollEvents` and `lvkw_ctx_waitEvents` pop events and trigger user callbacks.

## 7. Thread Affinity

LVKW follows a strict thread-affinity model.

- All calls to a context and its windows MUST originate from the thread that created the context.
- **Hybrid Model**: If `LVKW_CTX_FLAG_PERMIT_CROSS_THREAD_API` is set, certain functions (geometry queries, attribute updates) are allowed from other threads, but still require **external synchronization**.
- **Validation**: When `LVKW_VALIDATE_API_CALLS` is enabled, affinity is strictly enforced and will `abort()` on violation.

## 8. API Validation

API validation is performed by `static inline` functions in `src/lvkw/base/lvkw_api_constraints.h`.

- Every public API implementation should start with a call to `LVKW_API_VALIDATE(function_name, ...);`.
- This ensures consistent behavior between backends and provides detailed diagnostic feedback.
- Depending on `LVKW_RECOVERABLE_API_CALLS`, validation failures will either `abort()` (default for development) or return an error code.

## 9. String Caching

The library maintains a small, fixed-size string cache (`LVKW_StringCache`) in the context base.

- Use `_lvkw_string_cache_intern` to store strings that need to remain valid for the lifetime of the context (e.g., clipboard MIME types or text).
- This avoids frequent allocations and ensures stable pointers for the user.

## 10. Controller Support (Optional)

Controller/Gamepad support is guarded by the `LVKW_ENABLE_CONTROLLER` macro.

- Backends implementing controller support should populate the `ctrl` section of the `LVKW_Backend` function table.
- Controller events are pushed to the same event queue as window events.

## 11. Coding Conventions

- **Naming Conventions**:
    - `lvkw_`: Public API prefix.
    - `snake_case` for grouping (e.g., `lvkw_ctx_`, `lvkw_wnd_`).
    - `camelCase` for actions (e.g., `pollEvents`, `getGeometry`).
    - `_lvkw_`: Internal shared helpers (across files).
    - `_WL`, `_X11`, `_Win32`: Backend-specific implementation suffixes.
- **Optimization Hints**:
    - `LVKW_HOT`: Functions in the performance-critical path.
    - `LVKW_COLD`: Initialization or infrequent functions.
- **Vulkan Surface Creation**:
    - Backends must respect the dynamic loading architecture (see [Vulkan Loading](vulkan_loading.md)).
    - Use `vk_loader` if provided, or fallback to weak symbol/module handle lookup.

## 12. Public Interface Cleanliness

To maintain a clean library experience:
- Public headers (`lvkw.h`, `lvkw.hpp`) must only contain user-relevant information.
- Internal details must reside in `details/` (for public-facing but internal logic like attribute shorthands) or in `src/`.

## 13. Licensing & Third-Party Code

When adding new third-party headers or code (vendoring) into `src/lvkw/*/dlib/vendor/`, you **MUST** ensure:
1.  **License Compatibility**: The code must use a permissive license compatible with LVKW's license (e.g., MIT, BSD, Zlib).
    - **Strictly Prohibited**: GPL, LGPL (unless dynamically linked, which defeats the purpose of vendoring headers), or any license requiring source disclosure of the *entire* application.
2.  **Attribution**: If the license requires attribution (like MIT), ensure the license text is preserved in the header or added to `LICENSE.md`.
3.  **Minimalism**: Only vendor what is absolutely necessary to build the library without external system dependencies (headers only).


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
| Custom Cursors | 100% | 0% | 0% | 0% | 0% |
| Window Constraints & State | 100% | 0% | 0% | 0% | 0% |
| Drag and Drop | 100% | 0% | 0% | 0% | 0% |
| **Input Management** | | | | | |
| Event Polling/Waiting | 100% | 95% | 95% | 5% | 5% |
| Keyboard (State & Events) | 100% | 90% | 90% | 5% | 0% |
| Mouse Motion (Accelerated) | 100% | 95% | 95% | 0% | 0% |
| Mouse Motion (Raw/Dedicated) | 100% | 90% | 85% | 0% | 0% |
| Mouse Buttons & Scroll | 100% | 95% | 95% | 0% | 0% |
| Cursor Shapes & Modes | 100% | 90% | 80% | 0% | 0% |
| Controller / Gamepad | 100% | 80% | 80% | 0% | 0% |
| TextInput (UTF-8) | 100% | 85% | 75% | 0% | 0% |
| IME Support (Composition) | 100% | 0% | 0% | 0% | 0% |
| Controller Haptics/Rumble | 0% | 0% | 0% | 0% | 0% |
| **System & Environment** | | | | | |
| Monitor & Video Modes | 100% | 90% | 85% | 5% | 5% |
| HiDPI / Scaling Support | 100% | 95% | 80% | 0% | 0% |
| Clipboard (UTF-8 Text) | 100% | 85% | 80% | 0% | 0% |
| Idle Notification/Inhibition| 100% | 90% | 80% | 0% | 0% |