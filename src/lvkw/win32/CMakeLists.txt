find_package(Vulkan REQUIRED)

add_library(lvkw_win32_obj OBJECT
    lvkw_win32.c
    win32_context.c
    win32_window.c
    win32_events.c
    win32_core.c
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  target_sources(lvkw_win32_obj PRIVATE $<TARGET_OBJECTS:lvkw_controller_win32>)
endif()

target_link_libraries(lvkw_win32_obj PUBLIC lvkw_iface Vulkan::Vulkan)
target_link_libraries(lvkw_win32_obj PRIVATE lvkw_warnings)

target_include_directories(lvkw_win32_obj
PRIVATE
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../base
)

target_compile_definitions(lvkw_win32_obj PRIVATE UNICODE _UNICODE)

set_property(TARGET lvkw_win32_obj PROPERTY C_STANDARD 23)

add_library(lvkw_win32 STATIC
    $<TARGET_OBJECTS:lvkw_win32_obj>
    $<TARGET_OBJECTS:lvkw_base_obj>
)
target_link_libraries(lvkw_win32 PUBLIC lvkw_win32_obj lvkw_base_obj dwmapi)

