add_library(lvkw_iface INTERFACE)
target_include_directories(lvkw_iface INTERFACE 
    ${PROJECT_SOURCE_DIR}/include
)

add_library(lvkw_base OBJECT
    lvkw_base.c
    lvkw_event_queue.c
    lvkw_string_cache.c
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  target_sources(lvkw_base PRIVATE $<TARGET_OBJECTS:lvkw_controller_base>)
  target_compile_definitions(lvkw_iface INTERFACE LVKW_CONTROLLER_ENABLED)
endif()

target_link_libraries(lvkw_base PUBLIC lvkw_iface)
set_property(TARGET lvkw_base PROPERTY C_STANDARD 23)

# Win 32 doesn't get a dynamic backend
if (NOT WIN32)
  add_library(lvkw_dynamic OBJECT
      lvkw_base.c
      lvkw_event_queue.c
      lvkw_string_cache.c
  )


  target_link_libraries(lvkw_dynamic PUBLIC lvkw_iface)
  if(LVKW_ENABLE_CONTROLLER)
    target_sources(lvkw_dynamic PRIVATE $<TARGET_OBJECTS:lvkw_controller_base>)
  endif()
  target_compile_definitions(lvkw_dynamic PUBLIC LVKW_INDIRECT_BACKEND)
  set_property(TARGET lvkw_dynamic PROPERTY C_STANDARD 23)
endif()

add_library(lvkw_warnings INTERFACE)
if(MSVC)
  target_compile_options(lvkw_warnings INTERFACE /W4 /wd4100)
else()
  target_compile_options(lvkw_warnings INTERFACE 
    -Wall 
    -Wextra 
    -Wpedantic 
    -Wshadow 
    -Wcast-align 
    -Wunused 
    -Wpedantic 
    -Wconversion 
    -Wsign-conversion 
    -Wnull-dereference 
    -Wdouble-promotion 
    -Wformat=2
    -Wno-unused-parameter
    -Wno-unused-variable
  )
endif()

if (LVKW_ENABLE_DIAGNOSTICS)
  target_compile_definitions(lvkw_iface INTERFACE LVKW_ENABLE_DIAGNOSTICS)
endif()
if(LVKW_VALIDATE_API_CALLS)
  target_compile_definitions(lvkw_iface INTERFACE LVKW_VALIDATE_API_CALLS=1)
endif()
if(LVKW_RECOVERABLE_API_CALLS)
  target_compile_definitions(lvkw_iface INTERFACE LVKW_RECOVERABLE_API_CALLS=1)
endif()
if(LVKW_ENABLE_INTERNAL_CHECKS)
  target_compile_definitions(lvkw_iface INTERFACE LVKW_ENABLE_INTERNAL_CHECKS=1)
endif()

add_library(lvkw_external_lib_base INTERFACE)
target_include_directories(lvkw_external_lib_base INTERFACE .)
target_link_libraries(lvkw_external_lib_base INTERFACE lvkw_iface)