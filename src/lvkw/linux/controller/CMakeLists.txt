add_library(lvkw_controller_linux OBJECT
    lvkw_controller_linux.c
)

target_link_libraries(lvkw_controller_linux PUBLIC lvkw_linux_internal lvkw_controller_base_obj)
set_property(TARGET lvkw_controller_linux PROPERTY C_STANDARD 23)
