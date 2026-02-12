include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lvkw_tests
    test_event_queue.cpp
    test_internal_base.cpp
    test_string_cache.cpp
    test_mock_backend.cpp
    test_cpp_api.cpp
    test_validation.cpp
    test_version.cpp
    test_loop_hazard.cpp
)

target_include_directories(lvkw_tests PRIVATE
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
