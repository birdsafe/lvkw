add_library(lvkw_controller_macos OBJECT
    lvkw_controller_macos.c
)

# target_link_libraries(lvkw_controller_macos PUBLIC lvkw_macos_internal)
set_property(TARGET lvkw_controller_macos PROPERTY C_STANDARD 23)
