# X11 Completion Plan

## Goal
Bring the X11 backend to practical feature parity with the current LVKW public API contracts for Linux use-cases, with explicit test coverage for each completed area.

## Progress Checkpoints
- [x] 2026-02-17: Workstream 1 baseline implementation landed for X11 mouse motion contract.
- [x] 2026-02-17: Implemented `mouse_motion.delta` using per-window logical cursor history.
- [x] 2026-02-17: Implemented XI2 `XI_RawMotion` cookie processing and lock-mode `raw_delta` propagation.
- [x] 2026-02-17: Added cursor history resets on focus/enter/leave and lock/unlock transitions to avoid bogus deltas.
- [x] 2026-02-17: Workstream 2 slice: implemented maximize request path (`LVKW_WINDOW_ATTR_MAXIMIZED`) and `_NET_WM_STATE` property tracking to emit `LVKW_EVENT_TYPE_WINDOW_MAXIMIZED`.
- [x] 2026-02-17: Workstream 2 slice: applied create-time `cursor_mode`/`fullscreen`/`maximized` attributes and enabled X11 property/focus event masks.
- [x] 2026-02-17: Workstream 2 slice: initialized `lvkw_display_getWindowGeometry(...).origin` deterministically.
- [x] 2026-02-17: Workstream 2 slice: implemented X11 handlers for `min/max/aspect/resizable/decorated` window attributes via WM normal hints and Motif hints.
- [x] 2026-02-17: Workstream 2 slice: wired storage/updates for `monitor`, `accept_dnd`, `text_input_type`, `text_input_rect`, and `mouse_passthrough` (with unsupported diagnostic for passthrough enable).
- [x] 2026-02-17: Workstream 5 slice: implemented X11 idle timeout transition detection and `LVKW_EVENT_TYPE_IDLE_STATE_CHANGED` emission from pump loop.
- [x] 2026-02-17: Workstream 3 slice: implemented X11 clipboard ownership + SelectionRequest serving for text/MIME payloads.
- [x] 2026-02-17: Workstream 3 slice: implemented `set/getClipboardText`, `set/getClipboardData`, and `getClipboardMimeTypes` with context-managed cache lifetimes.
- [x] 2026-02-17: Workstream 3 slice: added X11 dynamic symbol coverage for selection/property APIs needed by clipboard flow.
- [x] 2026-02-17: Workstream 5 slice: added X11 wake pipe so `lvkw_events_post` unblocks `lvkw_events_pump` promptly.
- [x] 2026-02-17: Workstream 5 slice: added X11 monitor mode-change event emission (`LVKW_EVENT_TYPE_MONITOR_MODE`) on detected RandR mode/scale changes.
- [x] 2026-02-17: Workstream 5 slice: reconciled X11 `WINDOW_READY` emission to map-time (`MapNotify`) and added context-lost checks for post/scan event paths.
- [x] 2026-02-17: Workstream 4 prep slice: implemented `XdndAware` window property wiring through `LVKW_WINDOW_ATTR_ACCEPT_DND`.
- [x] 2026-02-17: Workstream 1/feature-parity slice: added X11 `LVKW_EVENT_TYPE_TEXT_INPUT` emission on key press via xkb UTF-8 extraction.
- [x] 2026-02-17: Workstream 3 polish slice: improved external clipboard interop by aliasing text targets (`UTF8_STRING`/`STRING`) and deduplicating MIME enumeration output.
- [x] 2026-02-17: Workstream 2 polish slice: `LVKW_WINDOW_ATTR_MONITOR` now emits explicit unsupported diagnostic when non-null monitor targeting is requested on X11.
- [x] 2026-02-17: Workstream 7 slice: synced docs/feature matrix to current X11 status and explicitly documented XDND action-feedback deferral.
- [ ] 2026-02-17: Workstream 6 blocked in current environment (`Xvfb`/`xvfb-run` unavailable; no deterministic finite-run X11 smoke binary yet), so X11 integration tests were not added this pass.
- [x] 2026-02-17: Checkpoint validation passed (`cmake --build build -j`, `ctest --test-dir build --output-on-failure`).
- [x] Workstream 2: Window attribute/state parity.
- [x] Workstream 3: Clipboard (text + MIME).
- [ ] Workstream 4: XDND drag-and-drop.
- [x] Workstream 5: Idle/monitor/event robustness.
- [ ] Workstream 6: Expanded X11-focused testing.
- [x] Workstream 7: Documentation and feature matrix sync.

## Scope
In scope:
- X11 backend behavior and API parity work under `src/lvkw/linux/x11/`.
- Supporting common/core updates needed by X11 completion.
- Unit/integration/manual test coverage for newly implemented X11 behavior.
- User/dev documentation updates reflecting true X11 support status.

Out of scope for this branch:
- Win32 backend implementation.
- Full IME composition parity with Wayland protocol richness.
- Refactors not required for X11 feature completion.

## Current Gap Baseline
Public API and docs expose behavior not currently implemented in X11:
- Mouse motion contract includes `delta` and `raw_delta`, but X11 currently leaves both unset (`src/lvkw/linux/x11/x11_events.c:98`).
- Clipboard APIs are hard-failing as unsupported in X11 (`src/lvkw/linux/x11/x11_window.c:452`).
- Many window attributes exist in API but are ignored in X11 update path (`include/lvkw/c/display.h:132`, `src/lvkw/linux/x11/x11_window.c:274`).
- X11 selects XI RawMotion at startup but does not process GenericEvent cookies (`src/lvkw/linux/x11/x11_context.c:303`, `src/lvkw/linux/x11/x11_events.c:50`).
- X11 does not currently emit text input/composition events or DND events, while API/docs define them (`include/lvkw/c/events.h:91`).
- Context idle timeout state is stored but idle state change event generation is missing (`src/lvkw/linux/x11/x11_context.c:497`).

## Execution Order (Priority)
1. Input/event correctness and event contracts.
2. Window attribute/state parity.
3. Clipboard implementation.
4. DND implementation.
5. Idle/monitor mode polish and backend robustness.
6. Test matrix hardening and documentation sync.

## Workstream 1: Input and Event Contract Completion
### Deliverables
- Populate `mouse_motion.delta` and `mouse_motion.raw_delta` correctly.
- Handle XInput2 GenericEvent cookies for `XI_RawMotion`.
- Ensure `raw_delta` contract: raw hardware movement only, otherwise `{0,0}`.
- Prevent bogus deltas from synthetic warps while cursor is locked.
- Keep compressible behavior unchanged for motion events.

### Implementation Tasks
- Extend X11 event loop to process `GenericEvent` and call `XGetEventData/XFreeEventData`.
- Track per-window last logical pointer position for accelerated delta.
- Track last raw values for locked pointer mode and emit raw deltas only when available.
- Reset delta baselines on focus changes, Enter/Leave, lock/unlock transitions.
- Validate modifier propagation consistency between key/button/motion paths.

### Files
- `src/lvkw/linux/x11/x11_events.c`
- `src/lvkw/linux/x11/x11_internal.h`

### Acceptance Criteria
- `LVKW_EVENT_TYPE_MOUSE_MOTION` always has deterministic `delta`.
- `raw_delta` is non-zero only with true raw input availability.
- No cursor-warp artifacts in deltas when in `LVKW_CURSOR_LOCKED`.

## Workstream 2: Window Attribute and State Parity
### Deliverables
- Implement remaining `LVKW_WindowAttributesField` handling for X11.
- Maintain `LVKW_WINDOW_STATE_*` flags and emit matching events where contract requires.
- Apply create-time attributes consistently (fullscreen/maximized/resizable/decorated/etc.).

### Implementation Tasks
- Add handlers in `lvkw_wnd_update_X11` for `LVKW_WINDOW_ATTR_MONITOR`, `LVKW_WINDOW_ATTR_MAXIMIZED`, `LVKW_WINDOW_ATTR_MIN_SIZE`, `LVKW_WINDOW_ATTR_MAX_SIZE`, `LVKW_WINDOW_ATTR_ASPECT_RATIO`, `LVKW_WINDOW_ATTR_RESIZABLE`, `LVKW_WINDOW_ATTR_DECORATED`, `LVKW_WINDOW_ATTR_MOUSE_PASSTHROUGH`, `LVKW_WINDOW_ATTR_ACCEPT_DND`, `LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE`, and `LVKW_WINDOW_ATTR_TEXT_INPUT_RECT`.
- Implement EWMH interactions (`_NET_WM_STATE_MAXIMIZED_*`, Motif/NET hints as needed).
- Add WM_NORMAL_HINTS support for min/max/aspect constraints.
- Implement creation-time application of attributes before mapping where possible.
- Emit `LVKW_EVENT_TYPE_WINDOW_MAXIMIZED` on transitions.
- Verify `lvkw_wnd_getGeometry_X11` fills `origin` deterministically.

### Files
- `src/lvkw/linux/x11/x11_window.c`
- `src/lvkw/linux/x11/x11_events.c`
- `src/lvkw/linux/x11/x11_internal.h`
- `src/lvkw/linux/x11/dlib/X11.h` (if extra symbols required)

### Acceptance Criteria
- Each declared window attribute bit produces real backend effect or documented deterministic no-op with diagnostics.
- Window flag transitions and maximization events align with observed WM state.

## Workstream 3: Clipboard (Text + MIME)
### Deliverables
- Implement `set/getClipboardText`, `set/getClipboardData`, `getClipboardMimeTypes` for X11.
- Match documented ownership/lifetime semantics for returned pointers.

### Implementation Tasks
- Add X11 selection ownership implementation for `CLIPBOARD`.
- Implement `SelectionRequest`, `SelectionNotify`, `SelectionClear` handling.
- Support targets negotiation (`TARGETS`, `UTF8_STRING`, fallback `STRING`, MIME atoms).
- Add context-level clipboard caches for owned MIME payloads, read-back data, and MIME query results.
- Add timeout/error handling for selection conversion flow.
- Ensure clipboard access remains primary-thread constrained by existing API model.

### Files
- `src/lvkw/linux/x11/x11_window.c`
- `src/lvkw/linux/x11/x11_events.c`
- `src/lvkw/linux/x11/x11_internal.h`
- `src/lvkw/linux/x11/dlib/X11.h`
- `src/lvkw/linux/x11/dlib/loader.c`

### Acceptance Criteria
- Text copy/paste succeeds between LVKW app and external X11 apps.
- MIME target list and data retrieval work for at least `text/plain;charset=utf-8` and `text/plain`.
- Lifetime guarantees in `docs/user_guide/clipboard.md` hold for X11 implementation.

## Workstream 4: Drag-and-Drop (XDND)
### Deliverables
- Implement `LVKW_EVENT_TYPE_DND_HOVER`, `LVKW_EVENT_TYPE_DND_DROP`, `LVKW_EVENT_TYPE_DND_LEAVE` on X11.
- Support action feedback loop from app callback to XDND status.

### Implementation Tasks
- Implement XDND message flow (`XdndEnter`, `XdndPosition`, `XdndStatus`, `XdndDrop`, `XdndLeave`, `XdndFinished`).
- Parse `text/uri-list` payload into path list.
- Wire `LVKW_DndFeedback.action` to returned XDND action.
- Respect `LVKW_WINDOW_ATTR_ACCEPT_DND` at runtime.
- Manage per-session userdata persistence across hover/drop/leave.

### Files
- `src/lvkw/linux/x11/x11_events.c`
- `src/lvkw/linux/x11/x11_window.c`
- `src/lvkw/linux/x11/x11_internal.h`
- `src/lvkw/linux/x11/dlib/X11.h`

### Acceptance Criteria
- Dragging files from common Linux file managers emits hover/drop/leave events with correct paths.
- Decline/accept feedback updates cursor/action on source app side.

## Workstream 5: Idle + Monitor/Event Parity + Robustness
### Deliverables
- Idle state changed event generation for X11 idle timeout path.
- Monitor mode-change event parity where detectible.
- Event pump robustness under blocking waits and external events.

### Implementation Tasks
- Periodically query XScreenSaver info during pump/commit and emit `LVKW_EVENT_TYPE_IDLE_STATE_CHANGED` transitions.
- Emit `LVKW_EVENT_TYPE_MONITOR_MODE` when current mode/scale changes after RandR notifications.
- Reconcile `WINDOW_READY` emission to avoid duplicate early ready signaling.
- Add wake mechanism for `lvkw_events_post` so blocked `pumpEvents` can wake promptly (eventfd or X11 self-pipe strategy).
- Harden context/window loss handling and diagnostics around X11 errors.

### Files
- `src/lvkw/linux/x11/x11_context.c`
- `src/lvkw/linux/x11/x11_events.c`
- `src/lvkw/linux/x11/x11_window.c`
- `src/lvkw/linux/x11/x11_internal.h`

### Acceptance Criteria
- Idle event transitions match timeout behavior.
- External posted events can unblock waiting pumps in bounded time.
- Monitor mode/state events are emitted predictably on RandR changes.

## Workstream 6: Testing and Validation
### Deliverables
- Automated tests for implemented X11 behavior.
- Manual validation checklist for WM/compositor variability.

### Implementation Tasks
- Extend mock backend only where API contract tests are backend-neutral.
- Add X11-focused integration tests gated behind Xvfb/CI capability.
- Add targeted tests for motion delta/raw_delta semantics, window attribute updates and resulting flags/events, clipboard round-trip and MIME enumeration, DND parsing/session userdata lifecycle, and idle/monitor mode transition events.
- Add manual test script for real desktop sessions (GNOME/KDE/X11).

### Files
- `tests/unit/` (new and updated tests)
- `tests/mock_backend/` (only if contract-level gaps require)
- `tests/` scripts for optional X11 integration runner

### Acceptance Criteria
- New tests pass locally and in CI target job(s).
- Manual checklist passes on at least two X11 desktop environments.

## Workstream 7: Documentation and Matrix Sync
### Deliverables
- Update docs to reflect actual X11 capabilities and caveats.
- Update feature matrix percentages based on shipped behavior.

### Files
- `docs/dev_guide/index.md`
- `docs/user_guide/events_and_input.md`
- `docs/user_guide/clipboard.md`
- `README.md` (status paragraph if needed)

### Acceptance Criteria
- Docs no longer advertise unsupported X11 behavior.
- Matrix reflects tested implementation, not estimates.

## Suggested Milestones
- Milestone A: Input contract + window state/attribute core.
- Milestone B: Clipboard end-to-end.
- Milestone C: XDND + idle/monitor parity.
- Milestone D: Full test pass + docs sync.

## Risk Register
- WM variance for maximized/decorated/constraints behavior.
- XDND interoperability quirks across file managers.
- Clipboard ownership edge-cases when source exits abruptly.
- Raw motion differences depending on XI2 availability and lock mode.

## Definition of Done
- No known X11 backend hard-fail path for public Linux APIs except explicitly documented unsupported features.
- Public contracts for mouse motion, clipboard, and key window attributes are met.
- Integration tests and manual checklist confirm stable behavior on real X11 sessions.
- Documentation and feature matrix updated in same branch.
