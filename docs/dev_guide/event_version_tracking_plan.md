# Event Version Tracking Implementation Plan

This document outlines the plan for adding optional event version tracking to LVKW's event system.

## Goal
Allow decoupled consumers to avoid rescanning unchanged snapshots while preserving existing event queue semantics and API ergonomics.

## Finalized Decisions

- `scanTracked` should **silently succeed** when called repeatedly for the same committed snapshot.
- `scanTracked` should continue to return `LVKW_SUCCESS` on the "already seen" path (no new status code).
- Public `getCommitId` is **not required for v1**.
- Existing external synchronization contract remains in effect: treat `commitEvents` as writer and `scanEvents`/`scanTracked` as readers of the same snapshot.
- Optional diagnostics may be added for detected skipped commits (`current_commit_id > last_seen_id + 1`), but this must not change API return behavior.

## Proposed Changes

### 1. Data Structure Updates (`src/lvkw/common/core/types_internal.h`)
Add a monotonic queue commit counter to `LVKW_EventQueue` with thread-safe access.

```c
typedef struct LVKW_EventQueue {
  // ... existing fields ...
  LVKW_ATOMIC(uint64_t) commit_id; // Increments on successful snapshot commit.
} LVKW_EventQueue;
```

### 2. Queue API Updates (`src/lvkw/common/context/event_queue.h/.c`)
Add internal helpers to read/increment commit state in one place.

```c
uint64_t lvkw_event_queue_get_commit_id(const LVKW_EventQueue *q);
void lvkw_event_queue_note_commit_success(LVKW_EventQueue *q);
```

Implementation notes:
- `lvkw_event_queue_get_commit_id` uses atomic relaxed load.
- `lvkw_event_queue_note_commit_success` uses atomic relaxed fetch-add.
- Increment must happen only after commit path is known successful for the backend call path.

### 3. Public C API (`include/lvkw/c/events.h`)
Add tracked scanning without introducing a new status value.

```c
/**
 * @brief Scans only when a newer committed snapshot exists.
 *
 * If no newer snapshot exists, returns LVKW_SUCCESS and does not invoke callback.
 * If a scan occurs, *last_seen_id is updated to the scanned commit ID.
 */
LVKW_HOT LVKW_Status lvkw_events_scanTracked(LVKW_Context *context,
                                             LVKW_EventType event_mask,
                                             uint64_t *last_seen_id,
                                             LVKW_EventCallback callback,
                                             void *userdata);
```

Behavior contract:
- If `last_seen_id == NULL`: `LVKW_ERROR_INVALID_USAGE`.
- Load current commit id, compare to `*last_seen_id`.
- If unchanged/older: return `LVKW_SUCCESS` with no callback calls.
- If newer: perform normal scan, then write back the scanned commit id.
- Optional diagnostic hook: report possible skipped frame when current id is more than one ahead.

### 4. Public C++ API (`include/lvkw/lvkw.hpp`, `include/lvkw/details/lvkw_hpp_impl.hpp`)
Add tracked overloads that preserve existing exception behavior.

```cpp
namespace lvkw {
  template <typename F>
  bool scanEvents(Context &ctx, LVKW_EventType event_mask, uint64_t &last_seen_id, F &&callback);

  template <typename F>
  bool pollEvents(Context &ctx, uint64_t &last_seen_id, F &&callback);
}
```

Notes:
- Return `true` when callback scan executed, `false` when snapshot unchanged.
- Underlying C status remains `LVKW_SUCCESS` in both paths; exceptions only for actual errors.

## Implementation Steps

1. **Phase 1: Internal Plumbing**
   - Add atomic `commit_id` to `LVKW_EventQueue`.
   - Initialize it to `0` in queue init.
   - Add internal getters/increment helpers in queue module.

2. **Phase 2: Correct Commit Boundary**
   - Update backend commit paths to call `lvkw_event_queue_note_commit_success(...)` only after successful gather/swap completion and context-lost checks.
   - Do not increment on failed commit returns.

3. **Phase 3: Tracked Scan (C)**
   - Implement `lvkw_events_scanTracked`.
   - Keep return code `LVKW_SUCCESS` for both "scanned" and "unchanged" paths.
   - Optional diagnostic for commit gaps.

4. **Phase 4: C++ Wrappers**
   - Add tracked overloads returning `bool` (did_scan).
   - Ensure wrappers throw only on real error statuses.

5. **Phase 5: Validation & Testing**
   - Add/extend tests in `tests/unit/test_event_queue.cpp` and API-level unit tests to verify:
   - `commit_id` increases once per successful commit.
   - Repeated `scanTracked` for same commit performs zero callbacks and returns success.
   - `last_seen_id` updates only when a scan occurs.
   - Null `last_seen_id` returns invalid usage.
   - Commit failure path does not increment `commit_id`.
   - C++ tracked overload returns `false` on unchanged snapshot and does not throw.
   - Optional skipped-commit diagnostic path (if enabled).

## Optionality
This feature is opt-in. Existing `lvkw_events_scan` and C++ polling/waiting paths continue to work unchanged. Overhead is a single atomic increment per successful commit and a lightweight compare in tracked scan.
