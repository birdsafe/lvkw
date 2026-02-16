cmake_minimum_required(VERSION 3.20)
project(lvkw_testbed)

find_package(Vulkan REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)

add_library(imgui_lib STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui_lib 
PUBLIC
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)

target_link_libraries(imgui_lib 
PRIVATE
  Vulkan::Vulkan
)

set(TESTBED_SOURCES
  src/main.cpp
  src/app.cpp
  src/modules/event_module.cpp
  src/modules/clipboard_module.cpp
  src/modules/clipboard_module.hpp
  src/modules/metrics_module.cpp
  src/modules/metrics_module.hpp
  src/modules/controller_module.cpp
  src/modules/monitor_module.cpp
  src/modules/cursor_module.cpp
  src/modules/dnd_module.cpp
  src/modules/dnd_module.hpp
  src/modules/event_log_module.cpp
  src/modules/event_log_module.hpp
  src/modules/input_module.cpp
  src/modules/input_module.hpp
  src/modules/window_module.cpp
  src/modules/window_module.hpp
)

function(add_testbed_variant name backend)
  add_executable(${name} ${TESTBED_SOURCES})
  target_include_directories(${name} PRIVATE src)
  target_link_libraries(${name} 
    PRIVATE
      ${backend}
      imgui_lib
      Vulkan::Vulkan
  )
  target_compile_features(${name} PRIVATE cxx_std_20)
endfunction()

if (UNIX AND NOT APPLE)
  add_testbed_variant(lvkw_testbed_linux lvkw::linux)
  add_testbed_variant(lvkw_testbed_wayland lvkw::wayland)
  add_testbed_variant(lvkw_testbed_x11 lvkw::x11)
else()
  add_testbed_variant(lvkw_testbed lvkw::lvkw)
endif()
