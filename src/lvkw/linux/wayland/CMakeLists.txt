find_package(Threads REQUIRED)

add_subdirectory(protocols)

set(WAYLAND_BACKEND_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/dlib/loader.c    
    ${CMAKE_CURRENT_LIST_DIR}/wayland_core.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_misc.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_input.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_events.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_display.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_context.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_monitor.c
    ${CMAKE_CURRENT_LIST_DIR}/wayland_window.c
)

#####################################
add_library(lvkw_wayland_obj OBJECT
    ${WAYLAND_BACKEND_SOURCES}
)

target_link_libraries(lvkw_wayland_obj PUBLIC lvkw_iface lvkw_external_lib_base lvkw_linux_internal lvkw_wayland_protocols Threads::Threads)
target_link_libraries(lvkw_wayland_obj PRIVATE lvkw_warnings)

target_include_directories(lvkw_wayland_obj 
PRIVATE 
    . 
    dlib/vendor
)
set_property(TARGET lvkw_wayland_obj PROPERTY C_STANDARD 23)

add_library(lvkw_wayland STATIC
    $<TARGET_OBJECTS:lvkw_wayland_obj>
    $<TARGET_OBJECTS:lvkw_base_obj>
    $<TARGET_OBJECTS:lvkw_linux_common_obj>
)
target_link_libraries(lvkw_wayland PUBLIC lvkw_wayland_obj lvkw_base_obj lvkw_linux_common_obj)

#################################

add_library(lvkw_wayland_indirect_obj OBJECT
    ${WAYLAND_BACKEND_SOURCES}
)

target_link_libraries(lvkw_wayland_indirect_obj PUBLIC lvkw_iface lvkw_external_lib_base lvkw_linux_internal lvkw_wayland_protocols ${CMAKE_DL_LIBS} Threads::Threads)
target_link_libraries(lvkw_wayland_indirect_obj PRIVATE lvkw_warnings)

target_include_directories(lvkw_wayland_indirect_obj 
PRIVATE 
    . 
    dlib/vendor
    protocols/generated
)

target_compile_definitions(lvkw_wayland_indirect_obj PUBLIC LVKW_INDIRECT_BACKEND)
set_property(TARGET lvkw_wayland_indirect_obj PROPERTY C_STANDARD 23)


################################

if(LVKW_ENABLE_CONTROLLER)
endif()
