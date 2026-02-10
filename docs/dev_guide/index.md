# LVKW Developer Guide

This guide provides a concise overview of the LVKW architecture and internal conventions for library developers.

## 1. Architecture Overview

LVKW uses a layered architecture to support multiple backends (Wayland, X11, Win32) with a unified C API.

- **Public API (`include/lvkw/lvkw.h`)**: Defines opaque handles and the core interface.
- **Internal Base (`src/lvkw/base/`)**: Provides common structures, event queueing, and base implementations.
- **Backends (`src/lvkw/wayland/`, `src/lvkw/x11/`, etc.)**: Implement platform-specific logic.

## 2. Core Handles & Types

All public handles are pointers to structures that start with a "Base" header. 

**Note on Opacity:** Handles in LVKW are **semi-opaque by design**. While the internal implementation details are hidden in `_Base` structures, the public structures in `lvkw.h` are intentionally visible to improve user-facing ergonomics (e.g., direct access to common status flags like `is_lost`) and to reduce the overall API surface area by avoiding unnecessary getter functions. This balance MUST be maintained; do not move these public structure definitions into private headers.

**Mutable Tokens:** Public handles (`LVKW_Context*` and `LVKW_Window*`) are treated as **mutable tokens**. They are rarely passed as `const` in the public API to allow users direct access to their fields (like `userdata`) without needing to cast, and to acknowledge that many "read-only" operations may still involve internal state changes (diagnostics, caching, etc.).

### `LVKW_Context`
- Public handle: `LVKW_Context*`
- Internal structure: `LVKW_Context_Base` (defined in `lvkw_types_internal.h`).
- Contexts store the backend function table, allocator, and event queue.

### `LVKW_Window`
- Public handle: `LVKW_Window*`
- Internal structure: `LVKW_Window_Base`.
- Windows belong to a context and are linked in a list within `LVKW_Context_Base`.

**Mandate:** Always use `(LVKW_Context_Base*)ctx` and `(LVKW_Window_Base*)win` to access internal fields.

## 3. Backend Selection & Dispatching (Linux)

Linux supports multiple backends in a single binary using **Indirect Dispatching**.

- **Macro**: `LVKW_INDIRECT_BACKEND` is defined for the Linux build.
- **Selection**: `lvkw_context_create` in `src/lvkw/linux/lvkw_linux.c` probes for Wayland first, then X11.
- **Dispatching**: Once a backend is selected, its function table (`LVKW_Backend`) is stored in `LVKW_Context_Base->prv.backend`. Subsequent API calls are routed through this table.

## 4. Error Handling & Diagnosis

LVKW uses a "Diagnosis" system instead of just return codes for detailed error reporting.

- **Macros (`lvkw_diag_internal.h`)**:
    - `LVKW_REPORT_CTX_DIAGNOSIS(ctx_base, diagnosis, msg)`
    - `LVKW_REPORT_WIND_DIAGNOSIS(window_base, diagnosis, msg)`
- **Assertions**:
    - `LVKW_CTX_ASSERT_ARG`: Validates arguments (aborts in debug).
    - `LVKW_CTX_ASSERT_PRECONDITION`: Validates state (aborts in debug).
    - `LVKW_CTX_ASSUME`: Internal consistency checks.

**Note**: Debug diagnosis (`LVKW_ENABLE_DEBUG_DIAGNOSIS`) enables thread-affinity checks. LVKW's is at the context level; all calls to a context must come from the thread that created it.

## 5. Memory Management

Never use `malloc`/`free` directly. Use the internal wrappers that respect the user-provided allocator.

- **Macros (`lvkw_mem_internal.h`)**:
    - `lvkw_context_alloc(ctx_base, size)`
    - `lvkw_context_free(ctx_base, ptr)`

## 6. Event Management

Events are stored in an `LVKW_EventQueue` (ring buffer) within the context base.

- **Enqueuing**: Backends call `lvkw_event_queue_push`.
- **Merging**: Small-motion mouse events are automatically merged to prevent queue saturation.
- **Dispatching**: `lvkw_context_pollEvents` and `lvkw_context_waitEvents` pop events and trigger user callbacks.

## 7. Thread Affinity

LVKW is **NOT thread-safe**. 

- All calls to a context and its windows MUST originate from the thread that created the context.
- In debug builds (`LVKW_ENABLE_DEBUG_DIAGNOSIS`), thread affinity is strictly enforced via `thrd_current()` and will `abort()` on violation.

## 8. Checked API

The `lvkw_checked.h` header provides `lvkw_chk_...` wrappers for most API functions.

- These wrappers perform extra validation (e.g., NULL checks, state validation) before calling the core API.
- They use the same diagnosis mechanism as the core library.
- It is meant to be used by wrappers for higher-level frameworks that need recoverability. C/C++ users of the library should not use it.


## 9. Coding Conventions

- **Prefixes**:
    - `lvkw_`: Public API.
    - `_lvkw_`: Internal shared helpers (across files).
    - `lvkw_..._WL/X11/Win32`: Backend-specific implementations of API functions.
- **Headers**:
    - `lvkw_internal.h`: Umbrella header for all internal definitions.
    - `lvkw_types_internal.h`: Shared structure definitions.
- **Boilerplate**:
    - Backend-specific creation functions (e.g., `_lvkw_context_create_WL`) are responsible for initializing both the base structure and their specific extensions.

## 10. Public Interface & Root Cleanliness

To maintain a clean and professional library experience, the following files MUST only contain user-relevant information and configuration:

- **`include/lvkw/lvkw.h` & `include/lvkw/lvkw.hpp`**: These should only contain public API definitions, documentation, and types. Internal implementation details, private macros, or build-system templates (like `.h.in` files) must reside in the `details/` subdirectory or within the `src/` tree.
- **Root `CMakeLists.txt`**: This file should focus on project-wide metadata (`project()` versioning), high-level options, and subdirectory orchestration. Library-specific implementation details (like `configure_file` for internal headers or complex compiler flag logic) should be pushed down into `src/CMakeLists.txt` or other relevant subdirectories.

The goal is to ensure that a user looking at the root of the repository or the public headers only sees what they need to use the library, not how the library is built.

## 11. API Constraints & Validation

LVKW employs a unified approach to API validation that serves two distinct purposes:

1.  **Checked API (`lvkw_checked.h`)**: Provides runtime-recoverable validation for language wrappers or high-level frameworks.
2.  **Debug Diagnosis**: Provides strict validation (aborting on failure) for C/C++ developers during development.

### Implementation Pattern

All API constraints are defined in `include/lvkw/details/lvkw_api_constraints.h` as `static inline` functions prefixed with `_lvkw_api_constraints_`.

- **Macros**: These functions use `_LVKW_..._CONSTRAINT` and `_LVKW_..._PRECONDITION` macros.
- **Public Use**: When included via `lvkw_checked.h`, these macros default to reporting a diagnosis and returning `LVKW_ERROR_NOOP`.
- **Internal Use**: When included via `src/lvkw/base/lvkw_api_checks.h`, these macros are redefined to map to internal assertions (`LVKW_CTX_ASSERT_ARG`, etc.), which typically `abort()` in debug builds to catch programmer errors early.

### Mandate: Public Header Placement

Because these constraints must be visible to the `lvkw_checked.h` header (which is public), **all API validation logic MUST reside in the public `include/lvkw/details/lvkw_api_constraints.h` header.** This ensures consistency between the core library's debug checks and the checked API's runtime validation.
