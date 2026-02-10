# Diagnosis and Error Handling

LVKW uses a two-tiered system for reporting issues: **Status Codes** for immediate control flow and a **Diagnosis Callback** for detailed, human-readable information.

## Tier 1: LVKW_Status

Most LVKW functions return an `LVKW_Status`. This is a simple enum that indicates whether the immediate operation succeeded or failed.

*   **`LVKW_SUCCESS`**: Everything is fine.
*   **`LVKW_ERROR`**: The operation failed.

### Lost States
In previous versions, LVKW used bitmasks to indicate if a window or context was "lost" (unrecoverable). This is now handled by checking the `is_lost` field on the handle itself:

```c
if (lvkw_ctx_pollEvents(...) == LVKW_ERROR) {
    if (ctx->is_lost) {
        // Abandon ship! The connection to the display server is gone.
    }
}
```

## Tier 2: The Diagnosis Callback

While `LVKW_Status` tells you *if* a failure occurred, the diagnosis callback tells you *why* it happened. This is a diagnostic tool meant for logging and debugging.

### Setting up a Callback

```c
void my_diagnosis_handler(const LVKW_DiagnosisInfo *info, void *userdata) {
    fprintf(stderr, "LVKW [%s]: %s\n", 
            info->window ? "Window" : "Context", 
            info->message);
}

LVKW_ContextCreateInfo ci = lvkw_ctx_defaultCreateInfo();
ci.diagnosis_cb = my_diagnosis_handler;
lvkw_createContext(&ci, &ctx);
```

### Diagnostic Categories
*   `LVKW_DIAGNOSIS_OUT_OF_MEMORY`
*   `LVKW_DIAGNOSIS_VULKAN_FAILURE`
*   `LVKW_DIAGNOSIS_INVALID_ARGUMENT` (Checked API or Debug builds)
*   `LVKW_DIAGNOSIS_PRECONDITION_FAILURE` (e.g., using a window before it is READY)

## C++ Exception Handling

The C++ wrapper (`lvkw.hpp`) automatically converts failures into `lvkw::Exception` objects. If a function returns `LVKW_ERROR`, an exception is thrown.

```cpp
try {
    lvkw::Window window(ctx, wci);
} catch (const lvkw::Exception& e) {
    std::cerr << "Caught LVKW error: " << e.what() << std::endl;
}
```

Note that the exception carries the `LVKW_Status`. For the detailed diagnostic message, you still need to check your logs (from the diagnosis callback).