add_library(lvkw_linux_internal INTERFACE)
target_include_directories(lvkw_linux_internal INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dlib/vendor
)

add_library(lvkw_linux_common_obj OBJECT
    dlib/linux_loader.c
    lvkw_linux_keys.c
)
target_link_libraries(lvkw_linux_common_obj PUBLIC lvkw_linux_internal lvkw_external_lib_base)
target_include_directories(lvkw_linux_common_obj PUBLIC dlib/vendor)
set_property(TARGET lvkw_linux_common_obj PROPERTY C_STANDARD 23)

add_subdirectory(wayland)
add_subdirectory(x11)

add_library(lvkw_linux_obj OBJECT
    lvkw_linux.c
)
target_link_libraries(lvkw_linux_obj PUBLIC lvkw_iface lvkw_linux_internal lvkw_external_lib_base)
target_compile_definitions(lvkw_linux_obj PRIVATE LVKW_INDIRECT_BACKEND)
set_property(TARGET lvkw_linux_obj PROPERTY C_STANDARD 23)

set(LVKW_LINUX_ALL_OBJS
    $<TARGET_OBJECTS:lvkw_linux_obj>
    $<TARGET_OBJECTS:lvkw_base_indirect_obj>
    $<TARGET_OBJECTS:lvkw_linux_common_obj>
    $<TARGET_OBJECTS:lvkw_wayland_indirect_obj>
    $<TARGET_OBJECTS:lvkw_wayland_protocols>
    $<TARGET_OBJECTS:lvkw_x11_indirect_obj>
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  set(LVKW_CTRL_LINUX_OBJS $<TARGET_OBJECTS:lvkw_controller_linux>)
  list(APPEND LVKW_LINUX_ALL_OBJS ${LVKW_CTRL_LINUX_OBJS})
  
  # Also add linux controller to standalone backends
  target_sources(lvkw_wayland PRIVATE ${LVKW_CTRL_LINUX_OBJS})
  target_sources(lvkw_x11 PRIVATE ${LVKW_CTRL_LINUX_OBJS})
endif()

add_library(lvkw_linux STATIC ${LVKW_LINUX_ALL_OBJS})

target_link_libraries(lvkw_linux PUBLIC 
    lvkw_linux_obj
    lvkw_base_indirect_obj
    lvkw_linux_common_obj
    lvkw_wayland_indirect_obj
    lvkw_x11_indirect_obj
)

if(LVKW_ENABLE_CONTROLLER)
  target_link_libraries(lvkw_linux PUBLIC lvkw_controller_linux)
endif()

target_link_libraries(lvkw_linux PRIVATE lvkw_warnings)

target_include_directories(lvkw_linux
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dlib/vendor
PRIVATE 
    . 
)
set_property(TARGET lvkw_linux PROPERTY C_STANDARD 23)