# Configure the config header (template in lvkw/common/context, output in build include tree)
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/lvkw/details")
configure_file(
    "lvkw/common/context/config.h.in"
    "${PROJECT_BINARY_DIR}/include/lvkw/details/lvkw_config.h"
)

add_subdirectory(lvkw/common)

if (WIN32)
  add_subdirectory(lvkw/win32)
elseif (APPLE)
  add_subdirectory(lvkw/macos)
else()
  add_subdirectory(lvkw/linux)
endif()
