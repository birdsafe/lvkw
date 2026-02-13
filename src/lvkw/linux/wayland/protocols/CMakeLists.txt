add_library(lvkw_wayland_protocols OBJECT
    wl_protocols.c
)

target_include_directories(lvkw_wayland_protocols 
PUBLIC 
  "${CMAKE_CURRENT_LIST_DIR}/../../dlib/vendor"
  "${CMAKE_CURRENT_LIST_DIR}/../dlib/vendor"
  "${CMAKE_CURRENT_LIST_DIR}/generated"
)
