add_library(lvkw_controller_base OBJECT
    lvkw_controller_base.c
)

target_link_libraries(lvkw_controller_base PUBLIC lvkw_iface lvkw_external_lib_base)
target_include_directories(lvkw_controller_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET lvkw_controller_base PROPERTY C_STANDARD 23)
