# Event Queue Eviction Policy Benchmarks (Legacy / Disabled)

**Note:** These benchmarks are currently disabled as the core LVKW library no longer uses a mandatory event queue. They are preserved here for reference and will be moved to a separate utility library in a future update.

This benchmark suite compares the 3 compile-time eviction policies used by
the old `lvkw_event_queue_push` when the queue was full.

Policies:
- `oldest_only`
- `half_by_type`
- `half_by_type_window`

Scenarios:
- `all_noncompressible` (key events only)
- `all_compressible` (motion/scroll/resize only)
- `mixed` (70% key, 20% motion, 10% scroll)

Grid:
- Queue capacities: `64, 256, 1024, 4096`
- Insertions: `Q/2, Q, 2Q, 8Q`
- Queue is fixed-size (`initial_capacity == max_capacity`, `growth_factor == 1.0`)

## Build

```bash
cmake -S . -B build -DLVKW_BUILD_BENCHMARKS=ON -DLVKW_BUILD_TESTS=OFF -DLVKW_BUILD_EXAMPLES=OFF
cmake --build build -j
```

## Run and export JSON

```bash
./build/benchmarks/lvkw_bench_event_queue_oldest \
  --benchmark_format=json \
  --benchmark_out=oldest.json

./build/benchmarks/lvkw_bench_event_queue_half_by_type \
  --benchmark_format=json \
  --benchmark_out=half_by_type.json

./build/benchmarks/lvkw_bench_event_queue_half_by_type_window \
  --benchmark_format=json \
  --benchmark_out=half_by_type_window.json
```

## Compare outputs

```bash
python3 benchmarks/event_queue/compare_event_queue_policies.py \
  oldest.json \
  half_by_type.json \
  half_by_type_window.json
```
