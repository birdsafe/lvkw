add_library(lvkw_iface INTERFACE)
target_include_directories(lvkw_iface INTERFACE
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(lvkw_iface INTERFACE
    LVKW_QUEUE_EVICT_STRATEGY=${LVKW_EVENT_QUEUE_EVICT_STRATEGY_DEFINE}
)

add_library(lvkw_c_atomics INTERFACE)
if(MSVC)
  target_compile_options(lvkw_c_atomics INTERFACE
    $<$<COMPILE_LANGUAGE:C>:/experimental:c11atomics>
  )
endif()
target_link_libraries(lvkw_iface INTERFACE lvkw_c_atomics)

add_library(lvkw_base_obj OBJECT
    context/context.c
    context/event_queue.c
    core/string_cache.c
    core/transient_pool.c
)
target_include_directories(lvkw_base_obj PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/context
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)

if(LVKW_ENABLE_CONTROLLER)
  add_subdirectory(controller)
  target_sources(lvkw_base_obj PRIVATE $<TARGET_OBJECTS:lvkw_controller_base_obj>)
endif()

target_link_libraries(lvkw_base_obj PUBLIC lvkw_iface)
set_property(TARGET lvkw_base_obj PROPERTY C_STANDARD 11)

# Win 32 doesn't get a dynamic backend
if (NOT WIN32)
  add_library(lvkw_base_indirect_obj OBJECT
      context/context.c
      context/event_queue.c
      core/string_cache.c
      core/transient_pool.c
  )
  target_include_directories(lvkw_base_indirect_obj PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/context
    ${CMAKE_CURRENT_SOURCE_DIR}/core
  )


  target_link_libraries(lvkw_base_indirect_obj PUBLIC lvkw_iface)
  if(LVKW_ENABLE_CONTROLLER)
    target_sources(lvkw_base_indirect_obj PRIVATE $<TARGET_OBJECTS:lvkw_controller_base_obj>)
  endif()
  target_compile_definitions(lvkw_base_indirect_obj PUBLIC LVKW_INDIRECT_BACKEND)
  set_property(TARGET lvkw_base_indirect_obj PROPERTY C_STANDARD 11)
endif()

add_library(lvkw_warnings INTERFACE)
if(MSVC)
  target_compile_options(lvkw_warnings INTERFACE /W4 /wd4100 /wd4189)
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

add_library(lvkw_external_lib_base INTERFACE)
target_include_directories(lvkw_external_lib_base INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/context
  ${CMAKE_CURRENT_SOURCE_DIR}/core
)
target_link_libraries(lvkw_external_lib_base INTERFACE lvkw_iface)
