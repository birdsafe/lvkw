# Text Input Model Rework (Backend-Neutral)

## Goal

Define a single text-input/composition event contract that is backend-neutral across Wayland, X11, Win32, and macOS.

This document proposes a **replacement** for the current composition semantics (pre-user stage, API break allowed).

## Why Rework

Current `LVKW_EVENT_TYPE_TEXT_COMPOSITION` is underspecified for cross-platform behavior:

- Wayland, XIM/XIC, TSF/IMM, and Cocoa expose different composition lifecycles.
- Cursor/selection semantics differ (byte index vs codepoint/UTF-16 style).
- Ordering with physical key events is not guaranteed tightly enough.
- `text_input_type` / `text_input_rect` cannot be implemented consistently without a strict lifecycle model.

## Proposed Event Model

### Keep

- `LVKW_EVENT_TYPE_KEY` (physical key semantics unchanged)

### Replace

Replace current composition usage with explicit phases:

- `LVKW_EVENT_TYPE_TEXT_EDIT_START`
- `LVKW_EVENT_TYPE_TEXT_EDIT_UPDATE`
- `LVKW_EVENT_TYPE_TEXT_EDIT_COMMIT`
- `LVKW_EVENT_TYPE_TEXT_EDIT_END`

`LVKW_EVENT_TYPE_TEXT_INPUT` becomes optional compatibility sugar and should be emitted only when it exactly mirrors `TEXT_EDIT_COMMIT`.

## Proposed Payloads (C API shape)

```c
typedef struct LVKW_TextRangeUtf8 {
  uint32_t start_byte;   // UTF-8 byte offset
  uint32_t length_byte;  // UTF-8 byte length
} LVKW_TextRangeUtf8;

typedef struct LVKW_TextEditStartEvent {
  uint64_t session_id;
} LVKW_TextEditStartEvent;

typedef struct LVKW_TextEditUpdateEvent {
  uint64_t session_id;
  LVKW_TRANSIENT const char* preedit_utf8;
  uint32_t preedit_length;
  LVKW_TextRangeUtf8 caret;      // insertion point as range length 0
  LVKW_TextRangeUtf8 selection;  // selected sub-range inside preedit
} LVKW_TextEditUpdateEvent;

typedef struct LVKW_TextEditCommitEvent {
  uint64_t session_id;
  uint32_t delete_before_bytes;
  uint32_t delete_after_bytes;
  LVKW_TRANSIENT const char* committed_utf8;
  uint32_t committed_length;
} LVKW_TextEditCommitEvent;

typedef struct LVKW_TextEditEndEvent {
  uint64_t session_id;
  bool canceled;
} LVKW_TextEditEndEvent;
```

## Normative Semantics

1. Session lifecycle
- Each composition session starts with `TEXT_EDIT_START` and ends with `TEXT_EDIT_END`.
- `session_id` is unique per window while the window lives.

2. Ordering
- `START` happens before any `UPDATE`/`COMMIT` in that session.
- `END` happens after final `UPDATE`/`COMMIT`.
- `COMMIT` may appear zero or more times during a session (backend-dependent), but all committed text must be emitted exactly once overall.
- `COMMIT` must not be emitted outside an active session.
- Each `COMMIT` is an atomic edit operation: delete `delete_before_bytes` before and `delete_after_bytes` after the current insertion point, then insert `committed_utf8`.
- Pure deletion commits are valid (`committed_length == 0`).
- Backends that cannot provide surrounding-delete data must emit `delete_before_bytes = 0` and `delete_after_bytes = 0`.

3. UTF-8 indexing
- All indices/ranges in public events use UTF-8 byte offsets.
- `delete_before_bytes` and `delete_after_bytes` are UTF-8 byte counts in application text space.
- Backends using UTF-16/UTF-32 must convert before event emission.

4. Focus rules
- Focus loss must terminate active session with `TEXT_EDIT_END{canceled=true}`.
- Destroying a window with an active session must emit the same cancel-end before teardown-visible event completion.

5. Relationship to `KEY`
- Physical key events remain independent.
- Backends must not synthesize fake key events from IME internals.

6. Direct text entry (non-IME)
- Direct key-driven text entry (for example, standard US keyboard typing with no active composition) must still be surfaced via `TEXT_EDIT_COMMIT`.
- For direct entry, backends should use a minimal session shape: `TEXT_EDIT_START` -> `TEXT_EDIT_COMMIT` -> `TEXT_EDIT_END`, with no `TEXT_EDIT_UPDATE`.
- For direct-entry commits, `delete_before_bytes` and `delete_after_bytes` are typically zero unless platform text services report replacement semantics.

7. Relationship to `TEXT_INPUT`
- Preferred long-term path: consume `TEXT_EDIT_COMMIT`.
- If `TEXT_INPUT` is retained during migration, each `TEXT_INPUT` must correspond 1:1 to one `TEXT_EDIT_COMMIT`.

## IME Hint + Capability Model

`LVKW_WINDOW_ATTR_TEXT_INPUT_TYPE` and `LVKW_WINDOW_ATTR_TEXT_INPUT_RECT` remain attributes, but behavior is capability-gated.
Add surrounding-text attributes so backends can provide IME context-aware edit operations:

```c
typedef struct LVKW_SurroundingTextUtf8 {
  LVKW_TRANSIENT const char* utf8;
  uint32_t length_bytes;
  uint32_t cursor_offset_bytes;  // insertion point within utf8
  uint32_t anchor_offset_bytes;  // selection anchor within utf8 (equals cursor for no selection)
} LVKW_SurroundingTextUtf8;
```

New window attributes:
- `LVKW_WINDOW_ATTR_SURROUNDING_TEXT` (value type: `LVKW_SurroundingTextUtf8`)
- `LVKW_WINDOW_ATTR_SURROUNDING_CURSOR_OFFSET` (value type: `uint32_t`, UTF-8 byte offset within surrounding text)

Add context/window capability query:

```c
typedef struct LVKW_TextInputCapabilities {
  bool supports_edit_sessions;
  bool supports_text_input_type;
  bool supports_text_input_rect;
  bool supports_surrounding_text;
  bool supports_candidate_window_control;
} LVKW_TextInputCapabilities;
```

Rules:
- If capability is false, setting related attribute is deterministic no-op with diagnostic.
- `supports_edit_sessions` must be true for backends claiming IME/composition support.
- If `supports_surrounding_text` is true, apps should keep `LVKW_WINDOW_ATTR_SURROUNDING_TEXT` updated whenever focused editable state changes (text, cursor, or selection).
- All surrounding text offsets are UTF-8 byte offsets within the provided `utf8` snapshot.
- `LVKW_WINDOW_ATTR_SURROUNDING_CURSOR_OFFSET` is an override for the cursor offset in the current surrounding-text snapshot and may be updated independently for cheap cursor moves.
- If both are set, the most recent attribute write wins for cursor position.
- Backends may use surrounding text for context-aware correction and surrounding delete/replace behavior.

## Backend Mapping Notes

### Wayland
- `zwp_text_input_v3` preedit/commit maps naturally to UPDATE/COMMIT.
- `delete_surrounding_text` + commit string map to one `COMMIT` with delete counts + `committed_utf8`.
- Enter/leave/focus transitions drive START/END.

### X11 (XIM/XIC)
- `XFilterEvent` + `Xutf8LookupString` + preedit callbacks feed UPDATE/COMMIT.
- XIC focus in/out and window focus loss drive START/END.
- Spot location updates map from `text_input_rect` when supported by input style.

### Win32 (TSF/IMM)
- TSF preferred; IMM fallback.
- Composition string/result string map to UPDATE/COMMIT.
- Replacement ranges from TSF/IMM context map to commit delete counts when available.
- IME end/cancel messages map to END.

### macOS (NSTextInputClient)
- `setMarkedText` maps to UPDATE.
- `insertText` maps to COMMIT.
- Replacement ranges map to commit delete counts when available.
- Unmark/cancel/focus transitions map to END.

## Migration Plan

1. Add new event types/payloads in headers.
2. Implement backend adapters:
- Wayland first (reference behavior)
- X11 (XIM/XIC)
- Win32
- macOS
3. Temporarily dual-emit `TEXT_INPUT` from `TEXT_EDIT_COMMIT`.
4. Remove old `TEXT_COMPOSITION` references from docs/tests.
5. Add contract tests that assert session ordering and UTF-8 range invariants.

## Test Contract (Must-Have)

- Session ordering: START -> (UPDATE|COMMIT)* -> END
- Focus-loss cancel end
- Byte-offset validity against `preedit_utf8` length
- Atomic replace correctness: delete-before/delete-after + committed text applied as one edit
- Pure deletion commit handling (`committed_length == 0`)
- Surrounding-text sync correctness: cursor/selection offsets remain valid UTF-8 byte offsets into the latest snapshot
- 1:1 commit mirroring to `TEXT_INPUT` (while compatibility is enabled)
- Cross-backend parity suite for identical scripted input sequences
