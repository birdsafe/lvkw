add_library(lvkw_controller_linux OBJECT
    lvkw_controller_linux.c
)

target_link_libraries(lvkw_controller_linux PUBLIC lvkw_linux_internal lvkw_controller_base_obj)
set_property(TARGET lvkw_controller_linux PROPERTY C_STANDARD 11)

if (NOT WIN32)
  add_library(lvkw_controller_linux_indirect OBJECT
      lvkw_controller_linux.c
  )

  target_link_libraries(lvkw_controller_linux_indirect PUBLIC lvkw_linux_internal lvkw_controller_base_obj)
  target_compile_definitions(lvkw_controller_linux_indirect PUBLIC LVKW_INDIRECT_BACKEND)
  set_property(TARGET lvkw_controller_linux_indirect PROPERTY C_STANDARD 11)
endif()
