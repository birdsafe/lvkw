// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include <benchmark/benchmark.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {
#include "event_queue.h"

void _lvkw_reportDiagnostic(LVKW_Context* ctx_handle, LVKW_Window* window_handle,
                            LVKW_Diagnostic diagnostic, const char* message) {
  (void)ctx_handle;
  (void)window_handle;
  (void)diagnostic;
  (void)message;
}
}

namespace {

enum class Scenario : int {
  AllNonCompressible = 0,
  AllCompressible = 1,
  Mixed = 2,
};

struct EventOp {
  LVKW_EventType type;
  LVKW_Window* window;
  LVKW_Event evt;
  bool compressible;
};

struct QueueFixture {
  LVKW_Context_Base ctx;
  LVKW_EventQueue queue;

  static void* alloc(size_t size, void*) { return std::malloc(size); }
  static void* realloc_cb(void* ptr, size_t size, void*) { return std::realloc(ptr, size); }
  static void free_cb(void* ptr, void*) { std::free(ptr); }

  void init(uint32_t queue_capacity) {
    std::memset(&ctx, 0, sizeof(ctx));
    std::memset(&queue, 0, sizeof(queue));
    ctx.prv.allocator = {
        .alloc_cb = &QueueFixture::alloc,
        .realloc_cb = &QueueFixture::realloc_cb,
        .free_cb = &QueueFixture::free_cb,
        .userdata = nullptr,
    };
    ctx.prv.event_mask = LVKW_EVENT_TYPE_ALL;

    LVKW_EventTuning tuning = {
        .initial_capacity = queue_capacity,
        .max_capacity = queue_capacity,
        .external_capacity = 64,
        .growth_factor = 1.0,
    };
    if (lvkw_event_queue_init(&ctx, &queue, tuning) != LVKW_SUCCESS) {
      std::abort();
    }
  }

  ~QueueFixture() { lvkw_event_queue_cleanup(&ctx, &queue); }
};

static const std::array<LVKW_Window*, 8> kWindows = {
    reinterpret_cast<LVKW_Window*>(0x1), reinterpret_cast<LVKW_Window*>(0x2),
    reinterpret_cast<LVKW_Window*>(0x3), reinterpret_cast<LVKW_Window*>(0x4),
    reinterpret_cast<LVKW_Window*>(0x5), reinterpret_cast<LVKW_Window*>(0x6),
    reinterpret_cast<LVKW_Window*>(0x7), reinterpret_cast<LVKW_Window*>(0x8),
};

LVKW_Event make_key_event(int i) {
  LVKW_Event evt = {};
  evt.key.key = static_cast<LVKW_Key>(LVKW_KEY_A + (i % 20));
  evt.key.state = LVKW_BUTTON_STATE_PRESSED;
  return evt;
}

LVKW_Event make_motion_event(int i) {
  LVKW_Event evt = {};
  evt.mouse_motion.position = {static_cast<LVKW_Scalar>(i), static_cast<LVKW_Scalar>(i * 2)};
  evt.mouse_motion.delta = {1.0, 1.0};
  evt.mouse_motion.raw_delta = {1.0, 1.0};
  return evt;
}

LVKW_Event make_scroll_event(int i) {
  LVKW_Event evt = {};
  evt.mouse_scroll.delta = {static_cast<LVKW_Scalar>(i % 3), static_cast<LVKW_Scalar>((i + 1) % 3)};
  return evt;
}

LVKW_Event make_resize_event(int i) {
  LVKW_Event evt = {};
  evt.resized.geometry.logicalSize = {
      static_cast<LVKW_Scalar>(640 + (i % 64)),
      static_cast<LVKW_Scalar>(360 + (i % 32)),
  };
  evt.resized.geometry.pixelSize = {
      1280 + (i % 64),
      720 + (i % 32),
  };
  return evt;
}

std::vector<EventOp> build_workload(Scenario scenario, int insertions) {
  std::vector<EventOp> ops;
  ops.reserve(static_cast<size_t>(insertions));

  for (int i = 0; i < insertions; ++i) {
    LVKW_Window* window = kWindows[static_cast<size_t>(i) % kWindows.size()];
    switch (scenario) {
      case Scenario::AllNonCompressible: {
        ops.push_back(EventOp{
            .type = LVKW_EVENT_TYPE_KEY,
            .window = window,
            .evt = make_key_event(i),
            .compressible = false,
        });
      } break;
      case Scenario::AllCompressible: {
        const int mod = i % 3;
        if (mod == 0) {
          ops.push_back(EventOp{
              .type = LVKW_EVENT_TYPE_MOUSE_MOTION,
              .window = window,
              .evt = make_motion_event(i),
              .compressible = true,
          });
        } else if (mod == 1) {
          ops.push_back(EventOp{
              .type = LVKW_EVENT_TYPE_MOUSE_SCROLL,
              .window = window,
              .evt = make_scroll_event(i),
              .compressible = true,
          });
        } else {
          ops.push_back(EventOp{
              .type = LVKW_EVENT_TYPE_WINDOW_RESIZED,
              .window = window,
              .evt = make_resize_event(i),
              .compressible = true,
          });
        }
      } break;
      case Scenario::Mixed: {
        const int mod = i % 10;
        if (mod < 7) {
          ops.push_back(EventOp{
              .type = LVKW_EVENT_TYPE_KEY,
              .window = window,
              .evt = make_key_event(i),
              .compressible = false,
          });
        } else if (mod < 9) {
          ops.push_back(EventOp{
              .type = LVKW_EVENT_TYPE_MOUSE_MOTION,
              .window = window,
              .evt = make_motion_event(i),
              .compressible = true,
          });
        } else {
          ops.push_back(EventOp{
              .type = LVKW_EVENT_TYPE_MOUSE_SCROLL,
              .window = window,
              .evt = make_scroll_event(i),
              .compressible = true,
          });
        }
      } break;
    }
  }

  return ops;
}

void run_push_workload(benchmark::State& state) {
  const uint32_t queue_capacity = static_cast<uint32_t>(state.range(0));
  const int insertions = static_cast<int>(state.range(1));
  const Scenario scenario = static_cast<Scenario>(state.range(2));
  const std::vector<EventOp> ops = build_workload(scenario, insertions);

  QueueFixture fixture;
  fixture.init(queue_capacity);

  double total_drops = 0.0;
  double total_retained = 0.0;

  for (auto _ : state) {
    fixture.queue.active->count = 0;
    fixture.queue.stable->count = 0;

    for (const EventOp& op : ops) {
      if (op.compressible) {
        benchmark::DoNotOptimize(
            lvkw_event_queue_push_compressible(&fixture.ctx, &fixture.queue, op.type, op.window, &op.evt));
      } else {
        benchmark::DoNotOptimize(
            lvkw_event_queue_push(&fixture.ctx, &fixture.queue, op.type, op.window, &op.evt));
      }
    }

    LVKW_EventMetrics metrics = {};
    lvkw_event_queue_get_metrics(&fixture.queue, &metrics, true);
    total_drops += static_cast<double>(metrics.drop_count);
    total_retained += static_cast<double>(fixture.queue.active->count);
  }

  state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(insertions));
  state.counters["queue_capacity"] = static_cast<double>(queue_capacity);
  state.counters["insertions"] = static_cast<double>(insertions);
  state.counters["drops_per_iter"] = benchmark::Counter(total_drops, benchmark::Counter::kAvgIterations);
  state.counters["retained_per_iter"] =
      benchmark::Counter(total_retained, benchmark::Counter::kAvgIterations);
  state.counters["retained_ratio"] =
      benchmark::Counter(total_retained / static_cast<double>(state.iterations() * insertions));
}

void add_arguments(benchmark::internal::Benchmark* bench) {
  static constexpr int queue_sizes[] = {64, 256, 1024, 4096};
  static constexpr int multipliers[] = {1, 2, 4, 16};  // 0.5x, 1x, 2x, 8x over (Q/2)

  for (int q : queue_sizes) {
    for (int m : multipliers) {
      const int n = (q / 2) * m;
      bench->Args({q, n, static_cast<int>(Scenario::AllNonCompressible)});
      bench->Args({q, n, static_cast<int>(Scenario::AllCompressible)});
      bench->Args({q, n, static_cast<int>(Scenario::Mixed)});
    }
  }
}

}  // namespace

BENCHMARK(run_push_workload)
    ->Apply(add_arguments)
    ->Unit(benchmark::kNanosecond)
    ->UseRealTime();

BENCHMARK_MAIN();
