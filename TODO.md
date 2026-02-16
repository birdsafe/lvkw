**N.B.** This is not a full list of what's left to be done (or maybe it is, I just don't want to mislead). Just a running tally so that I don't forget ideas that I have.

# API

**N.B.** For any library users stumbling upon this and might be put off by the API still being in flux: None of these are massive changes. Refactoring your project around them will be easy.

- Inestigate Monitor VS Controller inconsistencies 
  - They have a similar lifetime model, so they should probably be more aligned so that they share a mental model.

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
## Linux

- Finish Hoisting common functionality

### WAYLAND

- Introduce a "Pretend it's missing" flag to the protocols table for testing
- Do a tunable-exposition pass
- Do a Metrics exposition pass

### X11

- Finish implementation
- Audit feature gaps

## MacOS

- Finish implementation
- Audit feature gaps

## Win32

- Implement it
- N.B. No need to audit for a feature gap, everythign

# Common Core

- Do a "Does this need hoisting" pass.
- Formalize a

# Extensions

- Formalize extensions internal interface surface
- Formalize more atomic directory structure

## Controller

- 

# Documentation

- Formalize the module breakdown and enforce it everywhere, including backend implementation file names
- Completely redo the dev-guide
  - We need a Handle hierarchy section
  - We could use a "How is a backend structured" section
  - We could use a "How is an extension structured" section


# Build systtem

- Just implement CMake distributions already
- Investigate a dlib version of lvkw

# Testing

- Broadcast a call for help with testing
- Eventually: Investigate fuzzing
- Eventually: Investigate covergae testing
- Comprehensively review the mock backend, and reorganize it so that it's, in effect, an actual backend.