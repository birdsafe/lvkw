# Benchmarking

This page describes how to build and run LVKW's internal benchmarks, with focus on
the event queue eviction policy benchmarks.

## Path Convention

Use repository-root-relative paths in docs and examples.

- Good: `cmake -S . -B build-bench ...`
- Avoid: machine-local absolute paths such as `/home/.../repos/lvkw/...`

This keeps commands portable across contributors and CI.

## Configure For Benchmarks

From the repository root:

```bash
cmake -S . -B build-bench -DCMAKE_BUILD_TYPE=Release -DLVKW_BUILD_BENCHMARKS=ON -DLVKW_BUILD_TESTS=OFF -DLVKW_BUILD_EXAMPLES=OFF -DLVKW_VALIDATE_API_CALLS=OFF -DLVKW_ENABLE_INTERNAL_CHECKS=OFF -DLVKW_RECOVERABLE_API_CALLS=OFF -DLVKW_ENABLE_DIAGNOSTICS=ON -DLVKW_GATHER_TELEMETRY=ON
```

Expanded format:

```bash
cmake -S . -B build-bench \
  -DCMAKE_BUILD_TYPE=Release \
  -DLVKW_BUILD_BENCHMARKS=ON \
  -DLVKW_BUILD_TESTS=OFF \
  -DLVKW_BUILD_EXAMPLES=OFF \
  -DLVKW_VALIDATE_API_CALLS=OFF \
  -DLVKW_ENABLE_INTERNAL_CHECKS=OFF \
  -DLVKW_RECOVERABLE_API_CALLS=OFF \
  -DLVKW_EVENT_QUEUE_EVICT_STRATEGY=oldest_only \
  -DLVKW_ENABLE_DIAGNOSTICS=ON \
  -DLVKW_GATHER_TELEMETRY=ON

cmake --build build-bench -j
```

`LVKW_EVENT_QUEUE_EVICT_STRATEGY` accepts:
- `oldest_only`
- `half_by_type`
- `half_by_type_window`

## Event Queue Policy Benchmarks

The benchmark suite compares the three compile-time eviction policies:

- `LVKW_QUEUE_EVICT_STRATEGY_OLDEST_ONLY`
- `LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE`
- `LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE_WINDOW`

Bench binaries:

- `build-bench/benchmarks/lvkw_bench_event_queue_oldest`
- `build-bench/benchmarks/lvkw_bench_event_queue_half_by_type`
- `build-bench/benchmarks/lvkw_bench_event_queue_half_by_type_window`

Run all three and emit JSON:

```bash
cmake --build build-bench --target lvkw_bench_event_queue_all
```

This target writes:

- `event_queue_oldest.json`
- `event_queue_half_by_type.json`
- `event_queue_half_by_type_window.json`

Compare results:

```bash
python3 benchmarks/event_queue/compare_event_queue_policies.py \
  build-bench/benchmarks/event_queue_oldest.json \
  build-bench/benchmarks/event_queue_half_by_type.json \
  build-bench/benchmarks/event_queue_half_by_type_window.json
```

For benchmark workload details (queue sizes, insertion counts, scenario mix), see:

- `benchmarks/event_queue/README.md`
