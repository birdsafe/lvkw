# Metrics & Monitoring

## Enabling Metrics

Metrics collection is optional. To use it, LVKW must be compiled with the following CMake option:

- `LVKW_GATHER_METRICS`: Set to `ON`

If disabled, the metrics API functions will return `LVKW_SUCCESS` but with zeroed metrics (or only basic structural info like `current_capacity`). This ensures that your instrumentation code can remain active across all build types without extra error handling.

## Retrieving Metrics

Metrics is retrieved by "Category". Currently, LVKW supports the following categories:

- `LVKW_METRICS_CATEGORY_EVENTS`: Metrics related to the internal notification ring (used for cross-thread event posting).

### C API

Metrics are retrieved into a typed structure via `lvkw_instrumentation_getMetrics`.

```c
LVKW_EventMetrics tel;
// This will succeed even if metrics gathering is compiled out.
if (lvkw_instrumentation_getMetrics(ctx, LVKW_METRICS_CATEGORY_EVENTS, &tel, true) == LVKW_SUCCESS) {
    printf("Peak Ring Usage: %u\n", tel.peak_count);
    printf("Dropped Notifications: %u\n", tel.drop_count);
}
```

- **`reset` parameter**: If set to `true`, watermarks (like `peak_count`) are reset to current values, and counters (like `drop_count`) are reset to zero.

### C++ API

The C++ wrapper provides a type-safe template method `getMetrics<T>()`.

```cpp
auto tel = ctx.getMetrics<LVKW_EventMetrics>(true);
std::cout << "Peak Usage: " << tel.peak_count << std::endl;
```

## Notification Ring Metrics (`LVKW_EventMetrics`)

| Metric | Description |
| :--- | :--- |
| `peak_count` | The maximum number of events that were held in the notification ring simultaneously since the last reset (High Watermark). |
| `current_capacity` | The current size of the allocated notification ring. |
| `drop_count` | The total number of events discarded because the ring was full. |

### Using Metrics for Monitoring

These metrics help monitor the health of cross-thread event posting:

1.  **Monitor `peak_count`**: if it consistently nears your `current_capacity`, you might be posting events faster than the primary thread can process them.
2.  **Monitor `drop_count`**: If this is non-zero, your notification ring is too small for your burst patterns. Note that for standard backends, this capacity is currently fixed.
