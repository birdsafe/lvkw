include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.4
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

set(LVKW_BENCH_EVENT_QUEUE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/event_queue/bench_event_queue.cpp
)

set(LVKW_BENCH_EVENT_QUEUE_COMMON_INCLUDES
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/lvkw/base
)

function(lvkw_add_event_queue_policy_bench target_name policy_define)
  add_library(${target_name}_queue_core OBJECT
      ${PROJECT_SOURCE_DIR}/src/lvkw/base/lvkw_event_queue.c
  )
  target_include_directories(${target_name}_queue_core PRIVATE
      ${LVKW_BENCH_EVENT_QUEUE_COMMON_INCLUDES}
  )
  target_compile_definitions(${target_name}_queue_core PRIVATE
      LVKW_QUEUE_EVICT_STRATEGY=${policy_define}
  )
  set_property(TARGET ${target_name}_queue_core PROPERTY C_STANDARD 11)

  add_executable(${target_name}
      ${LVKW_BENCH_EVENT_QUEUE_SRC}
      $<TARGET_OBJECTS:${target_name}_queue_core>
  )
  target_include_directories(${target_name} PRIVATE
      ${LVKW_BENCH_EVENT_QUEUE_COMMON_INCLUDES}
  )
  target_link_libraries(${target_name}
      PRIVATE
      benchmark::benchmark
  )
  target_compile_definitions(${target_name} PRIVATE
      LVKW_QUEUE_EVICT_STRATEGY=${policy_define}
  )
  target_compile_features(${target_name} PRIVATE cxx_std_20)
endfunction()

lvkw_add_event_queue_policy_bench(
    lvkw_bench_event_queue_oldest
    LVKW_QUEUE_EVICT_STRATEGY_OLDEST_ONLY
)

lvkw_add_event_queue_policy_bench(
    lvkw_bench_event_queue_half_by_type
    LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE
)

lvkw_add_event_queue_policy_bench(
    lvkw_bench_event_queue_half_by_type_window
    LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE_WINDOW
)

add_custom_target(lvkw_bench_event_queue_all
    COMMAND $<TARGET_FILE:lvkw_bench_event_queue_oldest>
            --benchmark_format=json
            --benchmark_out=event_queue_oldest.json
    COMMAND $<TARGET_FILE:lvkw_bench_event_queue_half_by_type>
            --benchmark_format=json
            --benchmark_out=event_queue_half_by_type.json
    COMMAND $<TARGET_FILE:lvkw_bench_event_queue_half_by_type_window>
            --benchmark_format=json
            --benchmark_out=event_queue_half_by_type_window.json
    COMMENT "Running event queue eviction policy benchmarks (JSON outputs in current working directory)"
    DEPENDS
      lvkw_bench_event_queue_oldest
      lvkw_bench_event_queue_half_by_type
      lvkw_bench_event_queue_half_by_type_window
)
