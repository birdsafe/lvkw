add_library(lvkw_controller_win32 OBJECT
    lvkw_controller_win32.c
)

# target_link_libraries(lvkw_controller_win32 PUBLIC lvkw_win32_internal)
set_property(TARGET lvkw_controller_win32 PROPERTY C_STANDARD 11)
