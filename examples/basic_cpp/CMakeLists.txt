add_executable(lvkw_examples_basic_cpp
  main.cpp
)
target_compile_features(lvkw_examples_basic_cpp PRIVATE cxx_std_20)

target_link_libraries(lvkw_examples_basic_cpp PRIVATE lvkw::lvkw vulkan_shared_renderer)
