find_package(Vulkan REQUIRED)

add_library(lvkw_linux_internal INTERFACE)
target_include_directories(lvkw_linux_internal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lvkw_linux_common STATIC
    dlib/linux_loader.c
    lvkw_linux_keys.c
)
target_link_libraries(lvkw_linux_common PUBLIC lvkw_linux_internal lvkw_external_lib_base)
target_include_directories(lvkw_linux_common PUBLIC dlib/vendor)
set_property(TARGET lvkw_linux_common PROPERTY C_STANDARD 23)

add_library(lvkw_linux
    lvkw_linux.c
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  set(LVKW_CTRL_LINUX_OBJS $<TARGET_OBJECTS:lvkw_controller_linux>)
  target_sources(lvkw_linux PRIVATE ${LVKW_CTRL_LINUX_OBJS})
  target_sources(lvkw_wayland PRIVATE ${LVKW_CTRL_LINUX_OBJS})
  target_sources(lvkw_wayland_dynamic PRIVATE ${LVKW_CTRL_LINUX_OBJS})
  target_sources(lvkw_x11 PRIVATE ${LVKW_CTRL_LINUX_OBJS})
  target_sources(lvkw_x11_dynamic PRIVATE ${LVKW_CTRL_LINUX_OBJS})
endif()

target_compile_definitions(lvkw_linux PRIVATE LVKW_INDIRECT_BACKEND)

target_link_libraries(lvkw_linux PUBLIC 
    lvkw_dynamic
    lvkw_external_lib_base
    Vulkan::Vulkan 
    lvkw_linux_internal
    lvkw_wayland_dynamic
    lvkw_x11_dynamic
)
target_link_libraries(lvkw_linux PRIVATE lvkw_warnings)

target_include_directories(lvkw_linux
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/dlib/vendor
PRIVATE 
    . 
)
set_property(TARGET lvkw_linux PROPERTY C_STANDARD 23)