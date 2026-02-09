find_package(Vulkan REQUIRED)

if(NOT TARGET Vulkan::glslc)
  message(FATAL_ERROR "Vulkan::glslc not found! Please ensure Vulkan SDK is installed correctly.")
endif()

set(SHADER_SOURCES
  vert.glsl
  frag.glsl
)

set(SHADER_OUTPUTS)
foreach(SHADER IN LISTS SHADER_SOURCES)
  get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
  get_filename_component(SHADER_EXT ${SHADER} EXT)
  
  set(OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
  
  set(SHADER_STAGE "")
  if(SHADER_NAME MATCHES "vert")
    set(SHADER_STAGE "-fshader-stage=vertex")
  elseif(SHADER_NAME MATCHES "frag")
    set(SHADER_STAGE "-fshader-stage=fragment")
  endif()

  add_custom_command(
    OUTPUT ${OUTPUT_NAME}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
    COMMAND Vulkan::glslc ${SHADER_STAGE} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${OUTPUT_NAME}
    DEPENDS ${SHADER}
    COMMENT "Compiling shader ${SHADER}"
  )
  list(APPEND SHADER_OUTPUTS ${OUTPUT_NAME})
endforeach()

add_custom_target(c_shaders ALL DEPENDS ${SHADER_OUTPUTS})

add_executable(lvkw_examples_basic_c
  main.c
  vulkan_engine.c
)

add_dependencies(lvkw_examples_basic_c c_shaders)

add_custom_command(TARGET lvkw_examples_basic_c POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:lvkw_examples_basic_c>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_OUTPUTS} $<TARGET_FILE_DIR:lvkw_examples_basic_c>/shaders
  COMMENT "Copying shaders to executable directory"
)

target_link_libraries(lvkw_examples_basic_c PRIVATE lvkw::lvkw Vulkan::Vulkan)

# Ensure C11 standard
target_compile_features(lvkw_examples_basic_c PRIVATE c_std_11)
