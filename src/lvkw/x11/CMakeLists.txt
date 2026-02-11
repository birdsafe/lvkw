find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

set(X11_BACKEND_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/lvkw_x11.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_context.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_window.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_events.c
    ${CMAKE_CURRENT_LIST_DIR}/x11_input.c
    ${CMAKE_CURRENT_LIST_DIR}/dlib/loader.c
)

add_library(lvkw_x11 STATIC
    ${X11_BACKEND_SOURCES}
)

target_link_libraries(lvkw_x11 PUBLIC lvkw_base lvkw_external_lib_base lvkw_linux_common Vulkan::Vulkan ${CMAKE_DL_LIBS} Threads::Threads)
target_link_libraries(lvkw_x11 PRIVATE lvkw_warnings)

target_include_directories(lvkw_x11 
PRIVATE 
    dlib/vendor
    . 
)
set_property(TARGET lvkw_x11 PROPERTY C_STANDARD 23)

add_library(lvkw_x11_dynamic OBJECT
    ${X11_BACKEND_SOURCES}
)

target_link_libraries(lvkw_x11_dynamic PUBLIC lvkw_dynamic lvkw_external_lib_base lvkw_linux_common Vulkan::Vulkan ${CMAKE_DL_LIBS} Threads::Threads)
target_link_libraries(lvkw_x11_dynamic PRIVATE lvkw_warnings)

target_include_directories(lvkw_x11_dynamic 
PRIVATE 
    dlib/vendor
    . 
)

target_compile_definitions(lvkw_x11_dynamic PRIVATE LVKW_INDIRECT_BACKEND)
set_property(TARGET lvkw_x11_dynamic PROPERTY C_STANDARD 23)