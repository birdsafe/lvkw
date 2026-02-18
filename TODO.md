**N.B.** This is not a full list of what's left to be done (or maybe it is, I just don't want to mislead). Just a running tally so that I don't forget ideas that I have.

# API

- Consider introducing lvkw_wnd_setPosition()

- Should tunables be discoverable?
  - If they are ID-Driven, then we will be able to add/remove them without breaking the API (or at least the ABI)
  - We can still provide a header-only easy struct wrapper around it, to avoid users having to trudge
  - Counter-argument: 
    - Marking a tunable as deprecated in a minor release and removing them in the next major release sounds just fine 

- Should metrics be discoverable?
  - Same deal as tunables

- Revise how tunables/metrics should be organized for extensions 

- Investigate a shared model between tunables and Metrics

- Investigate the likelyhood of running out of event bits eventually.
  - Would introducing non-maskable events helps? (Don't have to do it now, just chck if it's our get-out-of-jail card)
  - Investigate full implications of 64-bit event masks

# Backends
- Align backend IME/text-input implementation with the [Text Input Model Rework proposal](docs/dev_guide/text_input_model.md).

## Linux

- Finish Hoisting common functionality

### WAYLAND

- Follow-up:
  - [x] 2026-02-18: Made post-drop DND payload timeout tunable (`wayland.dnd_post_drop_timeout_ms`).
  - Evaluate async refactor for clipboard/selection pull operations to match non-blocking DND behavior.
- Support `wp_primary_selection_v1` (middle-click paste) via consolidated `push/pull` API (See [Implementation Plan](docs/dev_guide/primary_selection_plan.md)).
- Implement `LVKW_WINDOW_ATTR_ASPECT_RATIO` via client-side enforcement (See [Implementation Plan](docs/dev_guide/wayland_constraints_plan.md)).
- Do a tunable-exposition pass
- Do a Metrics exposition pass

### X11

- Finish implementation
- Audit feature gaps
- Defer full XDND action-feedback implementation until we do a small protocol-state design pass (event callback feedback handoff vs. XdndStatus timing).

## MacOS

- Finish implementation
- Audit feature gaps

## Win32

- Implement it
- N.B. No need to audit for a feature gap, everythign

# Common Core

- Do a "Does this need hoisting" pass.
- Can we go lock-less for the [Sync|Scan]Events relationship? I think we might be able to...

# Extensions

- Formalize extensions internal interface surface

## Controller

- 

# Documentation

- Completely redo the dev-guide
  - We need a Handle hierarchy section
  - We could use a "How is a backend structured" section
  - We could use a "How is an extension structured" section


# Build systtem

- Just implement CMake distributions already
- Investigate a .so/.dll version of lvkw

# Testing

- Broadcast a call for help with testing
- Eventually: Investigate fuzzing
- Eventually: Investigate covergae testing
- Comprehensively review the mock backend, and reorganize it so that it's, in effect, an actual backend.
