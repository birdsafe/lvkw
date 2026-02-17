add_library(lvkw_controller_base_obj OBJECT
    controller_base.c
)

target_link_libraries(lvkw_controller_base_obj PUBLIC lvkw_iface lvkw_external_lib_base)
target_include_directories(lvkw_controller_base_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET lvkw_controller_base_obj PROPERTY C_STANDARD 11)
