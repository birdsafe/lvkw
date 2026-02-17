add_subdirectory(mock_backend)
add_subdirectory(unit)

add_test(
  NAME taxonomy_headers_self_contained
  COMMAND "${CMAKE_SOURCE_DIR}/tests/check_taxonomy_headers.sh"
          "${CMAKE_C_COMPILER}"
          "${CMAKE_CXX_COMPILER}"
          "${CMAKE_SOURCE_DIR}"
)
