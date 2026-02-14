find_package(Threads REQUIRED)

set(X11_BACKEND_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/lvkw_x11.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_context.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_window.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_events.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_input.c
    ${CMAKE_CURRENT_LIST_DIR}/dlib/loader.c
)

add_library(lvkw_x11_obj OBJECT
    ${X11_BACKEND_SOURCES}
)

target_link_libraries(lvkw_x11_obj PUBLIC lvkw_iface lvkw_external_lib_base lvkw_linux_internal Threads::Threads)
target_link_libraries(lvkw_x11_obj PRIVATE lvkw_warnings)

target_include_directories(lvkw_x11_obj 
PRIVATE 
    dlib/vendor
    . 
)
set_property(TARGET lvkw_x11_obj PROPERTY C_STANDARD 11)

add_library(lvkw_x11 STATIC
    $<TARGET_OBJECTS:lvkw_x11_obj>
    $<TARGET_OBJECTS:lvkw_base_obj>
    $<TARGET_OBJECTS:lvkw_linux_common_obj>
)
target_link_libraries(lvkw_x11 PUBLIC lvkw_x11_obj lvkw_base_obj lvkw_linux_common_obj)

#################################

add_library(lvkw_x11_indirect_obj OBJECT
    ${X11_BACKEND_SOURCES}
)

target_link_libraries(lvkw_x11_indirect_obj PUBLIC lvkw_iface lvkw_external_lib_base lvkw_linux_internal Threads::Threads)
target_link_libraries(lvkw_x11_indirect_obj PRIVATE lvkw_warnings)

target_include_directories(lvkw_x11_indirect_obj 
PRIVATE 
    dlib/vendor
    . 
)

target_compile_definitions(lvkw_x11_indirect_obj PRIVATE LVKW_INDIRECT_BACKEND)
set_property(TARGET lvkw_x11_indirect_obj PROPERTY C_STANDARD 11)