find_package(Vulkan REQUIRED)

add_library(lvkw_win32
    lvkw_win32.c
    win32_context.c
    win32_window.c
    win32_events.c
    win32_core.c
)

target_link_libraries(lvkw_win32 PUBLIC lvkw_base Vulkan::Vulkan dwmapi)
target_link_libraries(lvkw_win32 PRIVATE lvkw_warnings)

target_include_directories(lvkw_win32
PRIVATE
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../base
)

target_compile_definitions(lvkw_win32 PRIVATE UNICODE _UNICODE)

set_property(TARGET lvkw_win32 PROPERTY C_STANDARD 23)
