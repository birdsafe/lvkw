# Configure the config header (template in lvkw/base, output in build include tree)
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/lvkw/details")
configure_file(
    "lvkw/base/lvkw_config.h.in"
    "${PROJECT_BINARY_DIR}/include/lvkw/details/lvkw_config.h"
)

add_subdirectory(lvkw/base)

if (WIN32)
  add_subdirectory(lvkw/win32)
elseif (APPLE)
  add_subdirectory(lvkw/macos)
else()
  add_subdirectory(lvkw/linux)
endif()
