add_library(lvkw_mock_obj OBJECT
    mock_context.c
    mock_window.c
    mock_events.c
    mock_core.c
    mock_controller.c
)

target_link_libraries(lvkw_mock_obj PUBLIC lvkw_iface lvkw_base_obj)
target_link_libraries(lvkw_mock_obj PRIVATE lvkw_warnings)

target_include_directories(lvkw_mock_obj
    PUBLIC
    .
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lvkw/common/context
    ${CMAKE_SOURCE_DIR}/src/lvkw/common/core
)

set_property(TARGET lvkw_mock_obj PROPERTY C_STANDARD 11)

add_library(lvkw_mock STATIC
    $<TARGET_OBJECTS:lvkw_mock_obj>
    $<TARGET_OBJECTS:lvkw_base_obj>
)
target_link_libraries(lvkw_mock PUBLIC lvkw_mock_obj lvkw_base_obj)
