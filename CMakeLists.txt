cmake_minimum_required(VERSION 3.20)
project(Lvkw VERSION 0.1.0)

# AKA Are we building lvkw as a subdirectory or not?
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(LVKW_IS_ROOT_CMAKE_PROJECT ON)
else()
    set(LVKW_IS_ROOT_CMAKE_PROJECT OFF)
endif()

option(LVKW_ENABLE_DIAGNOSTICS "Enable diagnostics reporting through callbacks" ON)
option(LVKW_ENABLE_DEBUG_DIAGNOSTICS "Enable intensive debug diagnostics checking. Implies LVKW_ENABLE_DIAGNOSTICS" OFF)
option(LVKW_ENABLE_CONTROLLER "Enable controller support" ON)
option(LVKW_BUILD_EXAMPLES "Build example applications" ${LVKW_IS_ROOT_CMAKE_PROJECT})
option(LVKW_BUILD_TESTS "Build Tests" ${LVKW_IS_ROOT_CMAKE_PROJECT})


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