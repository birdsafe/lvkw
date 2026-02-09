
find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)
if (NOT WAYLAND_SCANNER_EXECUTABLE)
    message(FATAL_ERROR "Failed to find wayland-scanner")
endif()

set(WAYLAND_PROTOCOL_SOURCES "")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/generated")

macro(generate_wayland_protocol protocol_file)
    set(protocol_path "${CMAKE_CURRENT_LIST_DIR}/xml/${protocol_file}")

    string(REGEX REPLACE "\\.xml$" "-client-protocol.h" header_file_name ${protocol_file})
    string(REGEX REPLACE "\\.xml$" "-client-protocol.inc.h" code_file_name ${protocol_file})

    set(header_file "${CMAKE_CURRENT_LIST_DIR}/generated/${header_file_name}")
    set(code_file "${CMAKE_CURRENT_LIST_DIR}/generated/${code_file_name}")

    add_custom_command(OUTPUT ${header_file}
        COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_path}" ${header_file}
        DEPENDS "${protocol_path}"
        VERBATIM)

    add_custom_command(OUTPUT ${code_file}
        COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_path}" ${code_file}
        DEPENDS "${protocol_path}"
        VERBATIM)

    list(APPEND WAYLAND_PROTOCOL_SOURCES ${header_file} ${code_file})
endmacro()

generate_wayland_protocol("wayland.xml")
generate_wayland_protocol("viewporter.xml")
generate_wayland_protocol("xdg-shell.xml")
generate_wayland_protocol("idle-inhibit-unstable-v1.xml")
generate_wayland_protocol("pointer-constraints-unstable-v1.xml")
generate_wayland_protocol("relative-pointer-unstable-v1.xml")
generate_wayland_protocol("fractional-scale-v1.xml")
generate_wayland_protocol("xdg-activation-v1.xml")
generate_wayland_protocol("xdg-decoration-unstable-v1.xml")
generate_wayland_protocol("cursor-shape-v1.xml")
generate_wayland_protocol("tablet-v2.xml")
generate_wayland_protocol("content-type-v1.xml")
generate_wayland_protocol("ext-idle-notify-v1.xml")



add_custom_target(lvkw_wayland_protocols_generate DEPENDS ${WAYLAND_PROTOCOL_SOURCES})

add_library(lvkw_wayland_protocols 
    wl_protocols.c
)

target_include_directories(lvkw_wayland_protocols PUBLIC "${CMAKE_CURRENT_LIST_DIR}/generated")
add_dependencies(lvkw_wayland_protocols lvkw_wayland_protocols_generate)
