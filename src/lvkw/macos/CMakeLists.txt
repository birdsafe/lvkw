find_package(Vulkan REQUIRED)

add_library(lvkw_macos_obj OBJECT
    lvkw_macos.c
    cocoa_stubs.c
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  target_sources(lvkw_macos_obj PRIVATE $<TARGET_OBJECTS:lvkw_controller_macos>)
endif()

target_link_libraries(lvkw_macos_obj PUBLIC lvkw_iface Vulkan::Vulkan)
target_link_libraries(lvkw_macos_obj PRIVATE lvkw_warnings)

target_include_directories(lvkw_macos_obj
PRIVATE
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../base
)

set_property(TARGET lvkw_macos_obj PROPERTY C_STANDARD 11)

add_library(lvkw_macos STATIC
    $<TARGET_OBJECTS:lvkw_macos_obj>
    $<TARGET_OBJECTS:lvkw_base_obj>
)
target_link_libraries(lvkw_macos PUBLIC lvkw_macos_obj lvkw_base_obj)

