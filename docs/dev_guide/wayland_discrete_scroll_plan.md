# Wayland Discrete Mouse Scroll Implementation Plan

## Objective
Support discrete scroll increments (mouse wheel "clicks") in the Wayland backend. Currently, LVKW only processes continuous axis movements, which can make UI elements that expect "steps" (like zoom levels or item selection) feel imprecise or overly sensitive.

## 1. The Problem
The standard `wl_pointer.axis` event provides a continuous value. While high-precision touchpads use this well, physical mouse wheels send "clicks" that are better represented by the `wl_pointer.axis_discrete` event (or `axis_v120` in newer versions). Without handling these, a single wheel click might be interpreted as a small continuous movement rather than one clear "unit" of scrolling.

We also want a cross-backend contract that does not depend on backend-specific normalization constants.

## 2. Event Model Change (No Boolean Flag)
Extend `LVKW_MouseScrollEvent` to carry both:
- `delta` (continuous movement, existing field)
- `steps` (signed integer step counts per axis)

`has_steps` is unnecessary: `steps == 0` means no discrete component.

## 3. Wayland Protocol Details
We need to handle the `axis_discrete` event within the `wl_pointer` listener.

### `wl_pointer` version 5+
- **`axis_discrete`**: This event is sent alongside `axis`. It provides an integer representing the number of discrete steps (e.g., `1` for one notch forward).
- **`axis_v120` (Version 8+)**: Newer compositors use this for even higher precision (increments of 120, similar to Windows). 

## 4. Implementation Plan

### A. Data Structure Updates (`wayland_internal.h`)
Update the `pending_pointer` structure to track discrete steps:
```c
struct {
  uint32_t mask;
  LVKW_Event motion;
  LVKW_Event button;
  LVKW_Event scroll;
  struct { int32_t x, y; } discrete_steps_accum; // Track integer steps before emitting frame
  struct { int32_t x, y; } v120_accum;           // Optional high-res step accumulator
} pending_pointer;
```

### B. Event Handling (`wayland_input.c`)
Modify the `wl_pointer_listener`:
1. **`_pointer_handle_axis_discrete`**:
   - Accumulate integer steps in `discrete_steps_accum`.
2. **`_pointer_handle_axis`**:
   - Continue to calculate continuous delta as before.
3. **`_pointer_handle_axis_v120`** (where protocol version supports it):
   - Accumulate v120 values and convert to steps using integer division by 120.
   - Preserve remainder in `v120_accum` for subsequent frames.
4. **`_pointer_handle_frame`**:
   - When pushing the `LVKW_Event`:
     - Fill `delta` from continuous axis movement.
     - Fill `steps` from discrete/v120 accumulation.

### C. Normalization Policy
- Do not apply arbitrary constants (no `10.0`/`15.0` scaling heuristics).
- Preserve protocol-native semantics:
  - `delta` remains continuous.
  - `steps` remains discrete.

## 5. Cross-Backend Consistency
Implement the same event contract on other backends where possible:
- X11 wheel buttons map to `steps = +/-1` (with matching sign in `delta` policy).
- macOS non-precise wheel input maps to integer `steps`; precise trackpad input typically leaves `steps == 0`.
- Backends lacking discrete info keep `steps == 0`.

## 6. Tasks
1. **API**: Extend `LVKW_MouseScrollEvent` with `steps`.
2. **Internal**: Add step accumulators to `LVKW_Context_WL`.
3. **Listener**: Implement `_pointer_handle_axis_discrete` in `wayland_input.c`.
4. **Logic**: Implement v120 handling where supported and feed both `delta` and `steps` in frame emission.
5. **Backend Pass**: Align X11/macOS behavior to populate `steps` where possible.
6. **Verification**: Test with both a physical scroll wheel (discrete) and a touchpad (continuous) on GNOME and KDE.
