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

add_executable(lvkw_tests
    test_event_queue.cpp
    test_internal_base.cpp
    test_string_cache.cpp
    test_transient_pool.cpp
    test_mock_backend.cpp
    test_cpp_api.cpp
    test_validation.cpp
    test_version.cpp
    test_dnd.cpp
    test_haptics.cpp
    test_cursor.cpp
    test_abi.cpp
)

target_include_directories(lvkw_tests PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lvkw/base
)

target_link_libraries(lvkw_tests
    GTest::gtest_main
    lvkw_mock
)

target_compile_features(lvkw_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(lvkw_tests)

add_executable(lvkw_tests_half_by_type_window
    test_event_queue_half_by_type_window.cpp
)

target_include_directories(lvkw_tests_half_by_type_window PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lvkw/base
)

target_link_libraries(lvkw_tests_half_by_type_window
    GTest::gtest_main
    lvkw_mock
)

target_compile_definitions(lvkw_tests_half_by_type_window PRIVATE
    LVKW_QUEUE_EVICT_STRATEGY=LVKW_QUEUE_EVICT_STRATEGY_HALF_BY_TYPE_WINDOW
)

target_compile_features(lvkw_tests_half_by_type_window PRIVATE cxx_std_20)
gtest_discover_tests(lvkw_tests_half_by_type_window)
