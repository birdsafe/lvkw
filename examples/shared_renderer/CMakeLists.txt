find_package(Vulkan REQUIRED)
find_package(Python3 REQUIRED)

if(NOT TARGET Vulkan::glslc)
  message(FATAL_ERROR "Vulkan::glslc not found! Please ensure Vulkan SDK is installed correctly.")
endif()

set(VERT_GLSL "${CMAKE_CURRENT_SOURCE_DIR}/vert.glsl")
set(FRAG_GLSL "${CMAKE_CURRENT_SOURCE_DIR}/frag.glsl")

set(VERT_SPV "${CMAKE_CURRENT_BINARY_DIR}/vert.spv")
set(FRAG_SPV "${CMAKE_CURRENT_BINARY_DIR}/frag.spv")

add_custom_command(
    OUTPUT ${VERT_SPV}
    COMMAND Vulkan::glslc -fshader-stage=vertex ${VERT_GLSL} -o ${VERT_SPV}
    DEPENDS ${VERT_GLSL}
    COMMENT "Compiling vertex shader"
)

add_custom_command(
    OUTPUT ${FRAG_SPV}
    COMMAND Vulkan::glslc -fshader-stage=fragment ${FRAG_GLSL} -o ${FRAG_SPV}
    DEPENDS ${FRAG_GLSL}
    COMMENT "Compiling fragment shader"
)

set(GENERATED_SHADER_C "${CMAKE_CURRENT_BINARY_DIR}/shaders.c")
set(GENERATED_SHADER_H "${CMAKE_CURRENT_BINARY_DIR}/shaders.h")

add_custom_command(
    OUTPUT ${GENERATED_SHADER_C} ${GENERATED_SHADER_H}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/file_to_c.py 
            ${GENERATED_SHADER_C} ${GENERATED_SHADER_H}
            vert_glsl_spv ${VERT_SPV}
            frag_glsl_spv ${FRAG_SPV}
    DEPENDS ${VERT_SPV} ${FRAG_SPV} file_to_c.py
    COMMENT "Embedding shader byte arrays into C"
)

add_library(vulkan_shared_renderer STATIC
    vulkan_renderer.c
    vulkan_renderer.h
    ${GENERATED_SHADER_C}
    ${GENERATED_SHADER_H}
)

target_include_directories(vulkan_shared_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(vulkan_shared_renderer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(vulkan_shared_renderer PUBLIC lvkw::lvkw Vulkan::Vulkan)

# Ensure C11 standard
target_compile_features(vulkan_shared_renderer PRIVATE c_std_11)
