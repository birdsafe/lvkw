# Benchmarking

This page describes how to build and run LVKW's internal benchmarks.

**Note:** Event queue eviction policy benchmarks are currently disabled as the core library no longer includes a mandatory queue. They are being moved to a separate utility library.

## Path Convention

Use repository-root-relative paths in docs and examples.

- Good: `cmake -S . -B build-bench ...`
- Avoid: machine-local absolute paths such as `/home/.../repos/lvkw/...`

This keeps commands portable across contributors and CI.

## Configure For Benchmarks

From the repository root:

```bash
cmake -S . -B build-bench -DCMAKE_BUILD_TYPE=Release -DLVKW_BUILD_BENCHMARKS=ON -DLVKW_BUILD_TESTS=OFF -DLVKW_BUILD_EXAMPLES=OFF -DLVKW_VALIDATE_API_CALLS=OFF -DLVKW_ENABLE_INTERNAL_CHECKS=OFF -DLVKW_RECOVERABLE_API_CALLS=OFF -DLVKW_ENABLE_DIAGNOSTICS=ON -DLVKW_GATHER_METRICS=ON
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
  -DLVKW_ENABLE_DIAGNOSTICS=ON \
  -DLVKW_GATHER_METRICS=ON

cmake --build build-bench -j
```
