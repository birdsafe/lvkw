add_library(lvkw_mock
    mock_context.c
    mock_window.c
    mock_events.c
    mock_core.c
)

target_link_libraries(lvkw_mock PUBLIC lvkw_base)
target_link_libraries(lvkw_mock PRIVATE lvkw_warnings)

target_include_directories(lvkw_mock
    PUBLIC
    .
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lvkw/base
)

set_property(TARGET lvkw_mock PROPERTY C_STANDARD 23)
