find_package(Vulkan REQUIRED)

add_library(lvkw_macos
    lvkw_macos.c
    cocoa_stubs.c
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  target_sources(lvkw_macos PRIVATE $<TARGET_OBJECTS:lvkw_controller_macos>)
endif()

target_link_libraries(lvkw_macos PUBLIC lvkw_base Vulkan::Vulkan)
target_link_libraries(lvkw_macos PRIVATE lvkw_warnings)

target_include_directories(lvkw_macos
PRIVATE
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../base
)

set_property(TARGET lvkw_macos PROPERTY C_STANDARD 23)
