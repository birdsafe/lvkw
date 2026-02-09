add_library(lvkw_iface INTERFACE)
target_include_directories(lvkw_iface INTERFACE ${PROJECT_SOURCE_DIR}/include)

add_library(lvkw_base OBJECT
    lvkw_base.c
    lvkw_event_queue.c
)

target_link_libraries(lvkw_base PUBLIC lvkw_iface)
set_property(TARGET lvkw_base PROPERTY C_STANDARD 23)

# Win 32 doesn't get a dynamic backend
if (NOT WIN32)
  add_library(lvkw_dynamic OBJECT
      lvkw_base.c
      lvkw_event_queue.c
  )


  target_link_libraries(lvkw_dynamic PUBLIC lvkw_iface)
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
  )
endif()

if (LVKW_ENABLE_DIAGNOSIS)
  target_compile_definitions(lvkw_iface INTERFACE LVKW_ENABLE_DIAGNOSIS)
endif()

if (LVKW_ENABLE_DEBUG_DIAGNOSIS)
  target_compile_definitions(lvkw_iface INTERFACE LVKW_ENABLE_DEBUG_DIAGNOSIS)
endif()

add_library(lvkw_external_lib_base INTERFACE)
target_include_directories(lvkw_external_lib_base INTERFACE .)
target_link_libraries(lvkw_external_lib_base INTERFACE lvkw_iface)