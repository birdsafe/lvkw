find_package(Vulkan REQUIRED)

enable_language(OBJC)

add_library(lvkw_macos_obj OBJECT
    macos_entry.c
    cocoa_backend.m
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  target_sources(lvkw_macos_obj PRIVATE $<TARGET_OBJECTS:lvkw_controller_macos>)
endif()

target_link_libraries(lvkw_macos_obj PUBLIC lvkw_iface Vulkan::Vulkan)
target_link_libraries(lvkw_macos_obj PRIVATE lvkw_warnings)

target_link_libraries(lvkw_macos_obj PRIVATE 
    "-framework Cocoa"
    "-framework QuartzCore"
    "-framework Metal"
)

target_include_directories(lvkw_macos_obj
PRIVATE
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/context
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/core
)

set_property(TARGET lvkw_macos_obj PROPERTY C_STANDARD 11)

add_library(lvkw_macos STATIC
    $<TARGET_OBJECTS:lvkw_macos_obj>
    $<TARGET_OBJECTS:lvkw_base_obj>
)
target_link_libraries(lvkw_macos PUBLIC lvkw_macos_obj lvkw_base_obj)
