include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

function(lvkw_add_unit_target target)
  cmake_parse_arguments(ARG "" "" "SOURCES;LABELS;COMPILE_DEFINITIONS" ${ARGN})

  add_executable(${target} ${ARG_SOURCES})
  target_include_directories(${target} PRIVATE
      ${CMAKE_BINARY_DIR}/include
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/src/lvkw/common/context
      ${CMAKE_SOURCE_DIR}/src/lvkw/common/core
  )

  target_link_libraries(${target}
      GTest::gtest_main
      lvkw_mock
  )

  target_compile_features(${target} PRIVATE cxx_std_20)

  if (ARG_COMPILE_DEFINITIONS)
    target_compile_definitions(${target} PRIVATE ${ARG_COMPILE_DEFINITIONS})
  endif()

  gtest_discover_tests(${target} PROPERTIES LABELS "${ARG_LABELS}")
endfunction()

lvkw_add_unit_target(lvkw_tests_core
  SOURCES
    test_version.cpp
    test_abi.cpp
  LABELS "core"
)

lvkw_add_unit_target(lvkw_tests_context
  SOURCES
    test_validation.cpp
  LABELS "context;common_api_contract"
)

lvkw_add_unit_target(lvkw_tests_display
  SOURCES
    test_cursor.cpp
    test_common_api_contract.cpp
  LABELS "display;common_api_contract"
)

lvkw_add_unit_target(lvkw_tests_events
  SOURCES
    test_dnd.cpp
  LABELS "events;common_api_contract"
)

lvkw_add_unit_target(lvkw_tests_data
  SOURCES
    test_data_contract.cpp
  LABELS "data;common_api_contract"
)

lvkw_add_unit_target(lvkw_tests_input
  SOURCES
    test_input_contract.cpp
  LABELS "input;common_api_contract"
)

lvkw_add_unit_target(lvkw_tests_cpp
  SOURCES
    test_cpp_api.cpp
  LABELS "cpp_wrapper;common_api_contract"
)

lvkw_add_unit_target(lvkw_tests_common_queue
  SOURCES
    test_event_queue.cpp
  LABELS "events;common_internal"
)

lvkw_add_unit_target(lvkw_tests_common_queue_half_by_type_window
  SOURCES
    test_event_queue_half_by_type_window.cpp
  LABELS "events;common_internal"
)

lvkw_add_unit_target(lvkw_tests_common_internals
  SOURCES
    test_internal_base.cpp
    test_string_cache.cpp
    test_transient_pool.cpp
  LABELS "common_internal"
)
