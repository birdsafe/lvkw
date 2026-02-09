#ifndef VULKAN_ENGINE_H_INCLUDED
#define VULKAN_ENGINE_H_INCLUDED

#include <lvkw/lvkw.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct VulkanEngine {
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
} VulkanEngine;

void vulkan_engine_init(VulkanEngine* engine, LVKW_Context* ctx, LVKW_Window* window, uint32_t extension_count,
                        const char** extensions);
void vulkan_engine_cleanup(VulkanEngine* engine);
void vulkan_engine_draw_frame(VulkanEngine* engine);
void vulkan_engine_on_resized(VulkanEngine* engine, uint32_t width, uint32_t height);

#endif
