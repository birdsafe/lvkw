# Wayland Client-Side Constraints & "Hacks" Plan

## Objective
Implement client-side enforcement for window constraints (like Aspect Ratio) that are missing from the Wayland protocol, and provide a tunable to control these non-standard behaviors.

## 1. The "Hacks" Tunable
We will introduce a master switch in the context tuning to enable or disable behaviors that involve the client "fighting" or "filtering" compositor suggestions.

**`include/lvkw/c/context.h`**
```c
typedef struct LVKW_ContextTuning {
  // ... existing fields ...
  struct {
    LVKW_WaylandDecorationMode decoration_mode;
    /**
     * @brief If true, LVKW will intercept compositor resize suggestions to 
     * enforce constraints like aspect ratio. 
     * 
     * Note: This may cause "jitter" on some compositors that expect strict 
     * adherence to their size suggestions.
     */
    bool enforce_client_side_constraints;
  } wayland;
} LVKW_ContextTuning;
```

## 2. Aspect Ratio Enforcement Logic

Since `xdg_toplevel` only understands min/max size, LVKW must intercept the `configure` event and "fix" the dimensions before acknowledging them.

### The Algorithm
Inside `_xdg_toplevel_handle_configure` and `_libdecor_handle_configure`:
1. If `tuning.wayland.enforce_client_side_constraints` is `false` or no aspect ratio is set, proceed as normal.
2. If the window is in compositor-owned sizing states (fullscreen/maximized), skip client-side aspect enforcement.
3. If the compositor provides a specific width/height (`> 0`):
    - Calculate the target ratio: `R = numerator / denominator`.
    - Compare requested `W/H` to `R`.
    - If `W/H > R`: The window is too wide. Set `W = H * R`.
    - If `W/H < R`: The window is too tall. Set `H = W / R`.
4. Apply `min_size` and `max_size` constraints to the results.
5. Re-apply the ratio if min/max bounds broke it.
6. Store the adjusted size as pending configured size and use it for resize event emission and subsequent commit.

### Configure/Ack Sequencing Constraint
`ack_configure` must remain in the `xdg_surface.configure` path.
- Do not move ack logic into `xdg_toplevel.configure`.
- `xdg_toplevel.configure` computes/stores pending adjusted size.
- `xdg_surface.configure` acknowledges serial, then the committed surface dimensions must match that adjusted pending size.

This avoids protocol-ordering bugs and keeps sizing math decoupled from serial acknowledgment.

### Libdecor Note
For libdecor, apply constraints to content size (not full frame size), since aspect ratio usually targets app render area.

### Previous version (replaced)
The old flow described using adjusted values directly for acking in toplevel configure; that sequencing is incorrect and is superseded by the rules above.

## 3. Implementation Challenges

### A. The "Ping-Pong" Effect
Some compositors might see the client's adjusted size and immediately send another configure event to "correct" it back to their original suggestion.
- **Solution**: Ensure commit dimensions always match our latest adjusted pending size.
- Provide a tunable escape hatch (`enforce_client_side_constraints = false`) for compositor-specific issues.

### B. Libdecor Content vs. Frame
`libdecor` manages the frame. Constrain content surface dimensions, not frame extents.

## 4. Tasks
1. **Tuning**: Add `enforce_client_side_constraints` to `LVKW_ContextTuning`.
2. **Logic**: Implement `_lvkw_apply_aspect_ratio(width, height, ratio, min, max)`.
3. **Integration**:
   - Update `_xdg_toplevel_handle_configure` in `wayland_window.c` to compute/store adjusted pending size.
   - Keep `ack_configure` in `_xdg_surface_handle_configure`; apply pending size on commit path.
   - Update `_libdecor_frame_handle_configure` similarly using content dimensions.
4. **State Guards**: Skip enforcement when fullscreen/maximized.
5. **Verification**: Test under GNOME (Mutter) and KDE (KWin), including drag-resize, maximize, and fullscreen transitions.
