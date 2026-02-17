// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#ifndef VULKAN_RENDERER_H_INCLUDED
#define VULKAN_RENDERER_H_INCLUDED

#include <lvkw/lvkw.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct VulkanRenderer {
  VkInstance instance;
  VkSurfaceKHR surface;
  VkPhysicalDevice physicalDevice;
  VkDevice device;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkSwapchainKHR swapChain;
  VkImage* swapChainImages;
  uint32_t swapChainImageCount;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  VkImageView* swapChainImageViews;
  VkFramebuffer* swapChainFramebuffers;

  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  VkCommandPool commandPool;
  VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];

  VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
  VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
  uint32_t currentFrame;

  bool framebufferResized;
} VulkanRenderer;

void vulkan_renderer_init(VulkanRenderer* renderer, uint32_t extension_count, const char** extensions);
void vulkan_renderer_setup_surface(VulkanRenderer* renderer, VkSurfaceKHR surface, uint32_t width,
                                   uint32_t height);
void vulkan_renderer_cleanup(VulkanRenderer* renderer);
void vulkan_renderer_draw_frame(VulkanRenderer* renderer);
void vulkan_renderer_on_resized(VulkanRenderer* renderer, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif

#endif
