# Wayland Backend API Completeness Tasklist

## Coverage Baseline
- [x] Audit public C API declarations in `include/lvkw/*.h`
- [x] Map Linux dispatch to Wayland vtable in `src/lvkw/linux/wayland/wayland_core.c`
- [x] Classify each API item as implemented, partial, or unimplemented

## Fully Implemented (No action required)
- [x] `lvkw_ctx_destroy`
- [x] `lvkw_ctx_getVkExtensions`
- [x] `lvkw_ctx_syncEvents`
- [x] `lvkw_ctx_postEvent`
- [x] `lvkw_ctx_scanEvents`
- [x] `lvkw_ctx_getMonitors`
- [x] `lvkw_ctx_getMonitorModes`
- [x] `lvkw_ctx_getTelemetry` (`LVKW_TELEMETRY_CATEGORY_EVENTS`)
- [x] `lvkw_ctx_createWindow`
- [x] `lvkw_wnd_destroy`
- [x] `lvkw_wnd_createVkSurface`
- [x] `lvkw_wnd_getGeometry`
- [x] `lvkw_ctx_getStandardCursor`
- [x] `lvkw_ctx_createCursor`
- [x] `lvkw_cursor_destroy`
- [x] `lvkw_getVersion` (base)
- [x] `lvkw_wnd_getContext` (base)
- [x] `lvkw_ctrl_create` (Linux controller path)
- [x] `lvkw_ctrl_destroy` (Linux controller path)
- [x] `lvkw_ctrl_getInfo` (Linux controller path)

## Partial / Conditional APIs
- [x] Handle failure from initial `lvkw_ctx_update_WL(...)` during context creation
  - [x] Propagate failure from `src/lvkw/linux/wayland/wayland_context.c` instead of ignoring return value
- [x] Harden `lvkw_ctx_update` behavior for optional protocols
  - [x] Define expected behavior when `ext_idle_notifier_v1` is unavailable
  - [x] Define expected behavior when idle inhibition protocol is unavailable
- [x] Improve `lvkw_wnd_requestFocus` fallback behavior when `xdg_activation_v1` is missing
  - [x] Decide whether to keep error or degrade gracefully
- [x] Implement actual haptics in `lvkw_ctrl_setHapticLevels`
  - [x] Replace TODO no-op in `src/lvkw/linux/controller/lvkw_controller_linux.c`
- [x] Implement true `LVKW_CURSOR_HIDDEN` semantics
  - [x] Hide cursor while not locked in Wayland cursor update path

## Unimplemented Public APIs (Wayland)
- [x] Implement `lvkw_wnd_setClipboardText`
- [x] Implement `lvkw_wnd_getClipboardText`
- [x] Implement `lvkw_wnd_setClipboardData`
- [x] Implement `lvkw_wnd_getClipboardData`
- [x] Implement `lvkw_wnd_getClipboardMimeTypes`

## Missing Window Attribute Handling (`lvkw_wnd_update_WL`)
- [x] Implement `LVKW_WND_ATTR_DECORATED`
- [ ] Implement `LVKW_WND_ATTR_ACCEPT_DND`
- [x] Implement `LVKW_WND_ATTR_TEXT_INPUT_TYPE`
- [ ] Implement `LVKW_WND_ATTR_TEXT_INPUT_RECT`
- [x] Confirm and document behavior for `LVKW_WND_ATTR_ASPECT_RATIO` (currently stored but not enforced)

## Missing Event Types on Wayland
- [x] Emit `LVKW_EVENT_TYPE_DND_HOVER`
- [x] Emit `LVKW_EVENT_TYPE_DND_LEAVE`
- [x] Emit `LVKW_EVENT_TYPE_DND_DROP`
- [x] Emit `LVKW_EVENT_TYPE_TEXT_COMPOSITION`

## State/Contract Mismatches
- [x] Keep `LVKW_WND_STATE_FULLSCREEN` synchronized with backend fullscreen state
- [x] Apply `LVKW_ContentType` from `LVKW_WindowCreateInfo` on Wayland

## Verification Tasks
- [ ] Add backend-level tests for clipboard API behavior on Wayland
- [ ] Add tests for `LVKW_WND_ATTR_*` fields newly implemented in Wayland
- [ ] Add tests for DnD event lifecycle (hover/leave/drop)
- [ ] Add tests for text composition events
- [ ] Add tests for fullscreen flag synchronization
- [ ] Add tests for `LVKW_CURSOR_HIDDEN`

## Suggested Execution Order
- [x] Phase 1: Contract correctness (fullscreen flag, context update error propagation, cursor hidden)
- [ ] Phase 2: API parity (remaining missing `lvkw_wnd_update` attributes)
- [x] Phase 3: Event parity (DnD + text composition)
- [x] Phase 4: Controller haptics
- [ ] Phase 5: Test coverage for all completed items
