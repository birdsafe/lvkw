find_package(Vulkan REQUIRED)

add_library(lvkw_macos
    lvkw_macos.c
    cocoa_stubs.c
)

target_link_libraries(lvkw_macos PUBLIC lvkw_base Vulkan::Vulkan)
target_link_libraries(lvkw_macos PRIVATE lvkw_warnings)

target_include_directories(lvkw_macos
PRIVATE
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../base
)

set_property(TARGET lvkw_macos PROPERTY C_STANDARD 23)
