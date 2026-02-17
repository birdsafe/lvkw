add_executable(lvkw_examples_basic_c
  main.c
)

target_link_libraries(lvkw_examples_basic_c PRIVATE lvkw::lvkw vulkan_shared_renderer)

# Ensure C11 standard
target_compile_features(lvkw_examples_basic_c PRIVATE c_std_11)
