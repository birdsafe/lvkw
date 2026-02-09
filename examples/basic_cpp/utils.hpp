#ifndef LVKW_EXAMPLE_UTILS_HPP_INCLUDED
#define LVKW_EXAMPLE_UTILS_HPP_INCLUDED

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <string>

inline void vk_check(VkResult res, const char *msg) {
  if (res != VK_SUCCESS) {
    throw std::runtime_error(std::string(msg) + " (VkResult: " + std::to_string(res) + ")");
  }
}

#endif
