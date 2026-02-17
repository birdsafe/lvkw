# Primary Selection (Middle-Click) Implementation Plan

## Objective
Support the "Primary Selection" (highlight-to-copy, middle-click-to-paste) on Linux/Wayland via a consolidated IPC-style data transfer API.

## 1. API Changes (IPC-style)

The API is generalized to support multiple system-managed data exchange targets, moving away from legacy "Clipboard" naming to reflect the IPC nature of the operations.

### A. New Types (`include/lvkw/c/data.h`)
```c
typedef enum LVKW_DataExchangeTarget {
    LVKW_DATA_EXCHANGE_TARGET_CLIPBOARD = 0,
    LVKW_DATA_EXCHANGE_TARGET_PRIMARY = 1,
} LVKW_DataExchangeTarget;
```

### B. New Functions (`include/lvkw/c/data.h`)
New functions use `push/pull` terminology to emphasize cross-process data transfer:
- `lvkw_data_pushText(window, target, text)`
- `lvkw_data_pullText(window, target, out_text)`
- `lvkw_data_pushData(window, target, data_array, count)`
- `lvkw_data_pullData(window, target, mime, out_ptr, out_size)`
- `lvkw_data_listBufferMimeTypes(window, target, out_mimes, out_count)`

### C. Window Attribute (`include/lvkw/c/display.h`)
- `LVKW_WINDOW_ATTR_PRIMARY_SELECTION` (bool): Controls window participation in the primary selection protocol. Default: `true`.

## 2. Wayland Backend Strategy

### Generic Selection Handler
The logic for `wl_data_device` (Clipboard) and `wp_primary_selection_device` (Primary) is virtually identical (pipes, async reads, source/offer tracking).

**Plan:** Refactor `wayland_misc.c` to use an internal `LVKW_WaylandSelectionHandler` struct that can be instantiated for both targets. This reduces code duplication and ensures that bug fixes in the transfer logic (like the async DND read fix) apply to all selections.

### Protocol Binding
1.  Bind `zwp_primary_selection_device_manager_v1` (optional).
2.  In `_lvkw_wayland_seat_listener`, create a `zwp_primary_selection_device_v1` if the manager is available.
3.  When `lvkw_data_pushText` is called with `LVKW_DATA_EXCHANGE_TARGET_PRIMARY`:
    - Create a `zwp_primary_selection_source_v1`.
    - Claim ownership via `zwp_primary_selection_device_v1_set_selection`.
4.  Track selection changes via `zwp_primary_selection_device_v1_listener`.

## 3. Compatibility Policy
This is an intentional API break:
- Remove legacy clipboard-only entry points (no transitional wrappers).
- Rename/replace C and C++ API surface in one pass.
- Update backend dispatch tables and backend implementations to match the new signatures.

Because source compatibility is not required, this plan assumes downstream users will migrate to `push/pull` APIs directly.

## 4. Tasks
1.  **ABI**: Add new enum and `push/pull` functions to headers.
2.  **Internal**: Refactor existing Wayland clipboard code into a generic selection handler.
3.  **Protocol**: Bind `primary-selection-unstable-v1` in `wl_protocols.h`.
4.  **Backend**: Implement the mapping in `wayland_entry.c` and `wayland_misc.c`.
5.  **Attributes**: Implement `LVKW_WINDOW_ATTR_PRIMARY_SELECTION` toggle logic in `wayland_window.c`.
6.  **Dispatch Layer**: Update backend interfaces (`backend.h`), backend entry points, and cross-platform stubs (X11/Win32/macOS/mock) to the new data API.
7.  **Bindings/Tests**: Update C++ wrappers, validation tests, and mock backend tests to the new naming and semantics.
