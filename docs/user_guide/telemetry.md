# Telemetry & Monitoring

## Enabling Telemetry

Telemetry collection is optional. To use it, LVKW must be compiled with the following CMake option:

- `LVKW_GATHER_TELEMETRY`: Set to `ON`

If disabled, the telemetry API functions will return `LVKW_SUCCESS` but with zeroed metrics (or only basic structural info like `current_capacity`). This ensures that your instrumentation code can remain active across all build types without extra error handling.

## Retrieving Metrics

Telemetry is retrieved by "Category". Currently, LVKW supports the following categories:

- `LVKW_TELEMETRY_CATEGORY_EVENTS`: Metrics related to the internal event queue.

### C API

Metrics are retrieved into a typed structure via `lvkw_ctx_getTelemetry`.

```c
LVKW_EventTelemetry tel;
// This will succeed even if telemetry gathering is compiled out.
if (lvkw_ctx_getTelemetry(ctx, LVKW_TELEMETRY_CATEGORY_EVENTS, &tel, true) == LVKW_SUCCESS) {
    printf("Peak Queue Usage: %u\n", tel.peak_count);
    printf("Dropped Events: %u\n", tel.drop_count);
}
```

- **`reset` parameter**: If set to `true`, watermarks (like `peak_count`) are reset to current values, and counters (like `drop_count`) are reset to zero.

### C++ API

The C++ wrapper provides a type-safe template method `getTelemetry<T>()`.

```cpp
auto tel = ctx.getTelemetry<LVKW_EventTelemetry>(true);
std::cout << "Peak Usage: " << tel.peak_count << std::endl;
```

## Event Queue Metrics (`LVKW_EventTelemetry`)

| Metric | Description |
| :--- | :--- |
| `peak_count` | The maximum number of events that were held in the queue simultaneously since the last reset (High Watermark). |
| `current_capacity` | The current size of the allocated event queue. |
| `drop_count` | The total number of events discarded because the queue hit its `max_capacity` limits. |

### Using Telemetry for Tuning

These metrics are essential for optimizing the [Event Queue Tuning](tuning.md#event-queue-tuning):

1.  **Monitor `peak_count`**: if it consistently nears your `initial_capacity`, consider increasing the initial size to avoid runtime reallocations.
2.  **Monitor `drop_count`**: If this is non-zero, your `max_capacity` is too low, or your application is not gathering or polling events frequently enough (causing a backlog).
3.  **Check `current_capacity`**: If this is much higher than your `peak_count`, the queue may have grown during a rare burst and is now wasting memory.
