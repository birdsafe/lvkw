find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(protocols)

set(WAYLAND_BACKEND_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/dlib/loader.c    
    ${CMAKE_CURRENT_LIST_DIR}/wayland_core.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_misc.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_csd.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_input.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_events.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_display.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_context.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_window.c
)

add_library(lvkw_wayland STATIC
    ${WAYLAND_BACKEND_SOURCES}
)

target_link_libraries(lvkw_wayland PUBLIC lvkw_base lvkw_external_lib_base lvkw_linux_common lvkw_wayland_protocols Vulkan::Vulkan Threads::Threads)
target_link_libraries(lvkw_wayland PRIVATE lvkw_warnings)

target_include_directories(lvkw_wayland 
PRIVATE 
    . 
    dlib/vendor
)
set_property(TARGET lvkw_wayland PROPERTY C_STANDARD 23)

add_library(lvkw_wayland_dynamic OBJECT
    ${WAYLAND_BACKEND_SOURCES}
)

target_link_libraries(lvkw_wayland_dynamic PUBLIC lvkw_dynamic lvkw_external_lib_base lvkw_wayland_protocols Vulkan::Vulkan lvkw_linux_common ${CMAKE_DL_LIBS} Threads::Threads)
target_link_libraries(lvkw_wayland_dynamic PRIVATE lvkw_warnings)

target_include_directories(lvkw_wayland_dynamic 
PRIVATE 
    . 
    dlib/vendor
    protocols/generated
)

target_compile_definitions(lvkw_wayland_dynamic PUBLIC LVKW_INDIRECT_BACKEND)
set_property(TARGET lvkw_wayland_dynamic PROPERTY C_STANDARD 23)
