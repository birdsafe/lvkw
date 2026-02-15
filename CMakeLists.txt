cmake_minimum_required(VERSION 3.20)
project(Lvkw VERSION 0.1.0)

# AKA Are we building lvkw as a subdirectory or not?
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(LVKW_IS_ROOT_CMAKE_PROJECT ON)
else()
    set(LVKW_IS_ROOT_CMAKE_PROJECT OFF)
endif()

# Optional modules you can disable if you so chose.
option(LVKW_ENABLE_CONTROLLER "Enable controller support" ON)

# Debug and diagnostics options

# Disabling diagnostics will notably prevent any error handling string from making it to the final binary.
option(LVKW_ENABLE_DIAGNOSTICS "Enable diagnostics reporting through callbacks" ON)

# Telemetry allows monitoring watermarks and performance metrics.
option(LVKW_GATHER_TELEMETRY "Enable telemetry gathering" ON)

# API call validation specifically checks for programmer errors in API usage.
# Typically, you want this enabled during development, and disabled in production.
# The checking is quite thorough, It verifies thread affinity, context and window state, etc...
option(LVKW_VALIDATE_API_CALLS "Should API calls be validated?" OFF)

# By default, invalid API calls will cause the library to intentionally abort.
# You CAN make API calls early-out without crashing, but you should only do so if you
# actively need it for some reason (e.g if the calls are being driven by a scripting environment)
option(LVKW_RECOVERABLE_API_CALLS "Should invalid API calls be recoverable?" OFF)

# Mostly useful for debugging the library itself.
option(LVKW_ENABLE_INTERNAL_CHECKS "Should the library agressively validate internal state" OFF)

# Auxiliary targets
option(LVKW_BUILD_EXAMPLES "Build example applications" ${LVKW_IS_ROOT_CMAKE_PROJECT})
option(LVKW_BUILD_TESTS "Build Tests" ${LVKW_IS_ROOT_CMAKE_PROJECT})

# Arithmetic precision
option(LVKW_USE_FLOAT "Use float instead of double for logical coordinates" OFF)



add_subdirectory(src)

# You most likely want to simply use the lvkw::lvkw target,
# but you can also refer to a specific backend if you want.
if (WIN32)
  add_library(lvkw::lvkw ALIAS lvkw_win32)
  add_library(lvkw::win32 ALIAS lvkw_win32)
elseif (APPLE)
  add_library(lvkw::lvkw ALIAS lvkw_macos)
  add_library(lvkw::macos ALIAS lvkw_macos)
else()
  add_library(lvkw::lvkw ALIAS lvkw_linux)
  add_library(lvkw::linux ALIAS lvkw_linux)
  add_library(lvkw::wayland ALIAS lvkw_wayland)
  add_library(lvkw::x11 ALIAS lvkw_x11)
endif()

if(LVKW_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(LVKW_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Installation
install(DIRECTORY include/lvkw DESTINATION include)
install(FILES "${PROJECT_BINARY_DIR}/include/lvkw/details/lvkw_config.h"
        DESTINATION include/lvkw/details)

if (WIN32)
  install(TARGETS lvkw_win32 DESTINATION lib/lvkw)
elseif (APPLE)
  install(TARGETS lvkw_macos DESTINATION lib/lvkw)
else()
  install(TARGETS lvkw_linux lvkw_wayland lvkw_x11 DESTINATION lib/lvkw)
endif()
