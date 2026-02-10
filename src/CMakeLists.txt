find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

# Configure the version header (template in lvkw/base, output in root include)
configure_file(
    "lvkw/base/lvkw_version.h.in"
    "${PROJECT_SOURCE_DIR}/include/lvkw/details/lvkw_version.h"
)

add_subdirectory(lvkw/base)

if (WIN32)
  add_subdirectory(lvkw/win32)
else()
  find_package(PkgConfig REQUIRED)
  add_subdirectory(lvkw/wayland)
  add_subdirectory(lvkw/x11)
  add_subdirectory(lvkw/linux)
endif()
