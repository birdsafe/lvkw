# Advanced Configuration & Tuning

The `LVKW_ContextTuning` struct exposes internal hooks for advanced users. Most applications should stick to `LVKW_CONTEXT_TUNING_DEFAULT`, but understanding these options can solve niche problems.

## 1. Core Tuning

These options affect the fundamental behavior of the library and are relevant across all platforms.

## 4. Performance: The "Hot path"

The library internally distinguishes between API methods that are in the "hot" vs "cold" paths when weighting space vs time tradeoffs.

*   **Hot Path:** Methods (like `pumpEvents`, `getGeometry`) have a very strong bias in favor of execution time.
*   **Cold Path:** Methods (like `createContext`, `updateAttributes`) are weighted, within reason, in favor of binary space.

That does not mean that invoking a cold path method will result in a performance hitch. Any given cold-path method remains safe to invoke occasionally. However, if you find yourself invoking a cold-path method every single frame, you are likely not using the library as intended, and it should be treated as a code smell.

This is tagged in the headers using the `LVKW_HOT` and `LVKW_COLD` macros.

## 5. Extension Tuning

*This section is reserved for tuning parameters related to optional extensions (e.g., Controller deadzones, Audio buffers).*

*Currently, no extensions expose tuning parameters.*
