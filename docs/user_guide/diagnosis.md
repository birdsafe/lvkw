# Diagnosis and Error Handling in LVKW

LVKW uses a multi-tiered approach to tell you what went wrong and how bad it is.

## 1. API Misuse

Diagnosis reporting is reserved for runtime conditions, NOT API misuses by library users. 

These issues will be reported in the diagnosis callback with verbose details if LVKW_ENABLE_DEBUG_DIAGNOSIS is defined before aborting, but otherwise, the library assumes that it is being used correctly. 

If, for some reason, you want to handle API misuse errors (for a bridge to a higher level programming language), you may include `lvkw/lvkw_checked.h` instead if `lvkw/lvkw.h` to use wrapped API calls. (`lvkw_*` becomes `lvkw_checked_*`).

## 2. Result Codes and Severity (The Primary Mechanism)

Most LVKW functions return an `LVKW_Result`. In principle, this bitmask offers everything strictly needed to handle failures programmatically. It tells you the "blast radius" of a failure so you can decide how to recover.

*   **`LVKW_OK` (0)**: Everything is fine.
*   **`LVKW_ERROR_NOOP`**: The operation failed, but your handles (Window/Context) are still valid. Maybe an allocation failed or a feature is unsupported. You can usually just keep going.
*   **`LVKW_ERROR_WINDOW_LOST`**: The operation failed and the window handle is now dead. You must destroy it. You can try to recreate it if you want to continue.
*   **`LVKW_ERROR_CONTEXT_LOST`**: The entire context (and all its windows) are dead. This usually happens if the connection to the display server is lost. You must destroy the context and restart the engine.

### Specialized Result Types

To make the API more expressive, we use type aliases that hint at the maximum possible severity of a failure for a given function:

*   **`LVKW_Status`**: Guaranteed to only return `LVKW_OK` or `LVKW_ERROR_NOOP`. No handles are at risk.
*   **`LVKW_WindowResult`**: May return `LVKW_ERROR_WINDOW_LOST` (and anything less severe).
*   **`LVKW_ContextResult`**: May return `LVKW_ERROR_CONTEXT_LOST` (and anything less severe).

**Rule of thumb:** Always check the result bitmask. If you see `LVKW_RESULT_CONTEXT_LOST_BIT`, abandon ship on that context.

## 3. The Diagnosis Callback

While `LVKW_Result` tells you *how bad* the failure is, the diagnosis callback tells you *why* it happened. This is a diagnostic tool meant for logging and debugging.

Diagnosis callbacks are associated with a specific `LVKW_Context` and are passed during creation via `LVKW_ContextCreateInfo`.

```c
void my_diagnosis_logger(const LVKW_DiagnosisInfo *info, void *user_data) {
    fprintf(stderr, "LVKW Diagnosis: %s (Code: %d)\n", info->message, info->diagnosis);
}

LVKW_ContextCreateInfo ci = {
    .diagnosis_callback = my_diagnosis_logger,
    .diagnosis_user_data = my_logging_state,
    .user_data = my_engine_state
};

LVKW_Context *ctx;
lvkw_context_create(&ci, &ctx);
```

## 4. C++ Exceptions

The C++ wrapper (`lvkw.hpp`) automatically converts these bitmasks into `lvkw::Exception` objects. If a function returns anything other than `LVKW_OK`, an exception is thrown.

The C++ `lvkw::Context` defaults to a diagnosis callback that prints to `std::cerr`, but you can provide your own in the constructor:

```cpp
// Uses default stderr logger
lvkw::Context ctx;

// OR provide your own
lvkw::Context ctx(my_custom_handler, my_data);
```

Note that the exception carries the `LVKW_Result`. For the detailed diagnostic message, you still need to check your logs (from the diagnosis callback).

## 5. Programmer Errors (Debug Builds)

LVKW distinguishes between "System Failures" (e.g., Wayland disconnected) and "Programmer Errors" (e.g., passing a NULL pointer where it's not allowed).

If you are using a build with `LVKW_ENABLE_DEBUG_DIAGNOSIS` (usually the default in Debug):
*   **Invalid arguments** or **Precondition failures** will report the issue to your diagnostic callback and then **abort the process immediately.**
*   This is intentional. The library assumes that if you're using the API wrong, you want to know immediately during development.

## 6. Build Configuration and Overhead

LVKW is designed to be lean, but diagnosis reporting isn't free. You can toggle the level of safety vs. performance using two CMake variables:

*   **`LVKW_ENABLE_DIAGNOSIS`**: (Default: ON) Enables the internal logic to track diagnostic context and invoke your diagnosis callback. Turning this OFF removes almost all diagnosis-reporting overhead while keeping all control flows the same.
*   **`LVKW_ENABLE_DEBUG_DIAGNOSIS`**: (Default: ON in Debug builds) Enables expensive API validation (null checks, state checks, etc.). Failing a check results in an immediate `abort()`. This implies `LVKW_ENABLE_DIAGNOSIS`.


## 7. Lost States

Internal backends use a "lost" state tracking strategy. If a fatal protocol error occurs during event polling (e.g., the display server disconnects), the context is marked as lost. Subsequent calls to that context will immediately return `LVKW_ERROR_CONTEXT_LOST`. 

This keeps your application from spiraling into undefined behavior after a crash in the display server or a lost connection.
