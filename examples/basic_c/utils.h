#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

static inline void vk_check(VkResult res, const char *msg) {
  if (res != VK_SUCCESS) {
    fprintf(stderr, "%s (VkResult: %d)\n", msg, res);
    exit(EXIT_FAILURE);
  }
}

#endif
