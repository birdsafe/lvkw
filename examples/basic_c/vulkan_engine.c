// SPDX-License-Identifier: Zlib
// Copyright (c) 2026 François Chabot

#include "vulkan_engine.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

typedef struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  bool hasGraphicsFamily;
  uint32_t presentFamily;
  bool hasPresentFamily;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR* formats;
  uint32_t formatCount;
  VkPresentModeKHR* presentModes;
  uint32_t presentModeCount;
} SwapChainSupportDetails;

static char* read_file(const char* filename, size_t* out_size) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "failed to open file: %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(file_size);
  if (!buffer) {
    fprintf(stderr, "failed to allocate buffer for file: %s\n", filename);
    fclose(file);
    exit(EXIT_FAILURE);
  }

  size_t read_size = fread(buffer, 1, file_size, file);
  if (read_size != (size_t)file_size) {
    fprintf(stderr, "failed to read file: %s\n", filename);
    free(buffer);
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fclose(file);
  *out_size = (size_t)file_size;
  return buffer;
}

static void create_instance(VulkanEngine* engine, uint32_t extension_count, const char** extensions);
static void pick_physical_device(VulkanEngine* engine);
static void create_logical_device(VulkanEngine* engine);
static void create_swap_chain(VulkanEngine* engine);
static void create_image_views(VulkanEngine* engine);
static void create_render_pass(VulkanEngine* engine);
static void create_graphics_pipeline(VulkanEngine* engine);
static void create_framebuffers(VulkanEngine* engine);
static void create_command_pool(VulkanEngine* engine);
static void create_command_buffers(VulkanEngine* engine);
static void create_sync_objects(VulkanEngine* engine);
static void recreate_swap_chain(VulkanEngine* engine);
static void cleanup_swap_chain(VulkanEngine* engine);

static VkShaderModule create_shader_module(VulkanEngine* engine, const char* code, size_t size);
static VkSurfaceFormatKHR choose_swap_surface_format(const VkSurfaceFormatKHR* available_formats, uint32_t count);
static VkPresentModeKHR choose_swap_present_mode(const VkPresentModeKHR* available_present_modes, uint32_t count);
static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR* capabilities, VkExtent2D actual_extent);
static SwapChainSupportDetails query_swap_chain_support(VulkanEngine* engine, VkPhysicalDevice dev);
static void free_swap_chain_support_details(SwapChainSupportDetails* details);
static bool is_device_suitable(VulkanEngine* engine, VkPhysicalDevice dev);
static bool check_device_extension_support(VkPhysicalDevice dev);
static QueueFamilyIndices find_queue_families(VulkanEngine* engine, VkPhysicalDevice dev);

void vulkan_engine_init(VulkanEngine* engine, LVKW_Context* ctx, LVKW_Window* window, uint32_t extension_count,
                        const char** extensions) {
  (void)ctx;
  engine->framebufferResized = false;
  engine->currentFrame = 0;
  engine->swapChainImages = NULL;
  engine->swapChainImageViews = NULL;
  engine->swapChainFramebuffers = NULL;

  create_instance(engine, extension_count, extensions);

  if (lvkw_display_createVkSurface(window, engine->instance, &engine->surface) != LVKW_SUCCESS) {
    fprintf(stderr, "failed to create window surface!\n");
    exit(EXIT_FAILURE);
  }

  LVKW_WindowGeometry geometry;
  if (lvkw_display_getWindowGeometry(window, &geometry) != LVKW_SUCCESS) {
    fprintf(stderr, "failed to get window geometry!\n");
    exit(EXIT_FAILURE);
  }
  engine->swapChainExtent.width = geometry.pixelSize.x;
  engine->swapChainExtent.height = geometry.pixelSize.y;

  pick_physical_device(engine);
  create_logical_device(engine);
  create_swap_chain(engine);
  create_image_views(engine);
  create_render_pass(engine);
  create_graphics_pipeline(engine);
  create_framebuffers(engine);
  create_command_pool(engine);
  create_command_buffers(engine);
  create_sync_objects(engine);
}

void vulkan_engine_cleanup(VulkanEngine* engine) {
  vkDeviceWaitIdle(engine->device);

  cleanup_swap_chain(engine);

  vkDestroyPipeline(engine->device, engine->graphicsPipeline, NULL);
  vkDestroyPipelineLayout(engine->device, engine->pipelineLayout, NULL);
  vkDestroyRenderPass(engine->device, engine->renderPass, NULL);

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(engine->device, engine->renderFinishedSemaphores[i], NULL);
    vkDestroySemaphore(engine->device, engine->imageAvailableSemaphores[i], NULL);
    vkDestroyFence(engine->device, engine->inFlightFences[i], NULL);
  }

  vkDestroyCommandPool(engine->device, engine->commandPool, NULL);
  vkDestroyDevice(engine->device, NULL);
  vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
  vkDestroyInstance(engine->instance, NULL);
}

void vulkan_engine_on_resized(VulkanEngine* engine, uint32_t width, uint32_t height) {
  engine->framebufferResized = true;
  engine->swapChainExtent.width = width;
  engine->swapChainExtent.height = height;
}

void vulkan_engine_draw_frame(VulkanEngine* engine) {
  if (engine->framebufferResized) {
    recreate_swap_chain(engine);
    engine->framebufferResized = false;
  }

  vkWaitForFences(engine->device, 1, &engine->inFlightFences[engine->currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result =
      vkAcquireNextImageKHR(engine->device, engine->swapChain, UINT64_MAX,
                            engine->imageAvailableSemaphores[engine->currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    engine->framebufferResized = true;
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    fprintf(stderr, "failed to acquire swap chain image!\n");
    exit(EXIT_FAILURE);
  }

  vkResetFences(engine->device, 1, &engine->inFlightFences[engine->currentFrame]);
  vkResetCommandBuffer(engine->commandBuffers[engine->currentFrame], 0);

  VkCommandBufferBeginInfo beginInfo = {0};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(engine->commandBuffers[engine->currentFrame], &beginInfo) != VK_SUCCESS) {
    fprintf(stderr, "failed to begin recording command buffer!\n");
    exit(EXIT_FAILURE);
  }

  VkRenderPassBeginInfo renderPassInfo = {0};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = engine->renderPass;
  renderPassInfo.framebuffer = engine->swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
  renderPassInfo.renderArea.extent = engine->swapChainExtent;

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(engine->commandBuffers[engine->currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(engine->commandBuffers[engine->currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                    engine->graphicsPipeline);

  VkViewport viewport = {0};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)engine->swapChainExtent.width;
  viewport.height = (float)engine->swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(engine->commandBuffers[engine->currentFrame], 0, 1, &viewport);

  VkRect2D scissor = {0};
  scissor.offset = (VkOffset2D){0, 0};
  scissor.extent = engine->swapChainExtent;
  vkCmdSetScissor(engine->commandBuffers[engine->currentFrame], 0, 1, &scissor);

  vkCmdDraw(engine->commandBuffers[engine->currentFrame], 3, 1, 0, 0);
  vkCmdEndRenderPass(engine->commandBuffers[engine->currentFrame]);

  if (vkEndCommandBuffer(engine->commandBuffers[engine->currentFrame]) != VK_SUCCESS) {
    fprintf(stderr, "failed to record command buffer!\n");
    exit(EXIT_FAILURE);
  }

  VkSubmitInfo submitInfo = {0};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {engine->imageAvailableSemaphores[engine->currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &engine->commandBuffers[engine->currentFrame];

  VkSemaphore signalSemaphores[] = {engine->renderFinishedSemaphores[engine->currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vk_check(vkQueueSubmit(engine->graphicsQueue, 1, &submitInfo, engine->inFlightFences[engine->currentFrame]),
           "failed to submit draw command buffer");

  VkPresentInfoKHR presentInfo = {0};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {engine->swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(engine->presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    engine->framebufferResized = true;
  }
  else if (result != VK_SUCCESS) {
    fprintf(stderr, "failed to present swap chain image!\n");
    exit(EXIT_FAILURE);
  }

  engine->currentFrame = (engine->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

static void create_instance(VulkanEngine* engine, uint32_t extension_count, const char** extensions) {
  VkApplicationInfo appInfo = {0};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "LVKW Showcase C";
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = extension_count;
  createInfo.ppEnabledExtensionNames = extensions;

  vk_check(vkCreateInstance(&createInfo, NULL, &engine->instance), "failed to create instance");
}

static void pick_physical_device(VulkanEngine* engine) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(engine->instance, &deviceCount, NULL);
  if (deviceCount == 0) {
    fprintf(stderr, "failed to find GPUs with Vulkan support!\n");
    exit(EXIT_FAILURE);
  }
  VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(engine->instance, &deviceCount, devices);

  engine->physicalDevice = VK_NULL_HANDLE;
  for (uint32_t i = 0; i < deviceCount; i++) {
    if (is_device_suitable(engine, devices[i])) {
      engine->physicalDevice = devices[i];
      break;
    }
  }
  free(devices);

  if (engine->physicalDevice == VK_NULL_HANDLE) {
    fprintf(stderr, "failed to find a suitable GPU!\n");
    exit(EXIT_FAILURE);
  }
}

static void create_logical_device(VulkanEngine* engine) {
  QueueFamilyIndices indices = find_queue_families(engine, engine->physicalDevice);

  VkDeviceQueueCreateInfo queueCreateInfos[2];
  uint32_t uniqueQueueFamilies[2] = {indices.graphicsFamily, indices.presentFamily};
  uint32_t uniqueQueueFamilyCount = 0;

  // Simple unique check
  if (indices.graphicsFamily == indices.presentFamily) {
    uniqueQueueFamilyCount = 1;
  }
  else {
    uniqueQueueFamilyCount = 2;
  }

  float queuePriority = 1.0f;
  for (uint32_t i = 0; i < uniqueQueueFamilyCount; i++) {
    queueCreateInfos[i] = (VkDeviceQueueCreateInfo){0};
    queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamilies[i];
    queueCreateInfos[i].queueCount = 1;
    queueCreateInfos[i].pQueuePriorities = &queuePriority;
  }

  VkPhysicalDeviceFeatures deviceFeatures = {0};
  const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = uniqueQueueFamilyCount;
  createInfo.pQueueCreateInfos = queueCreateInfos;
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = 1;
  createInfo.ppEnabledExtensionNames = deviceExtensions;

  vk_check(vkCreateDevice(engine->physicalDevice, &createInfo, NULL, &engine->device),
           "failed to create logical device");

  vkGetDeviceQueue(engine->device, indices.graphicsFamily, 0, &engine->graphicsQueue);
  vkGetDeviceQueue(engine->device, indices.presentFamily, 0, &engine->presentQueue);
}

static void create_swap_chain(VulkanEngine* engine) {
  SwapChainSupportDetails swapChainSupport = query_swap_chain_support(engine, engine->physicalDevice);

  VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.formats, swapChainSupport.formatCount);
  VkPresentModeKHR presentMode =
      choose_swap_present_mode(swapChainSupport.presentModes, swapChainSupport.presentModeCount);
  VkExtent2D extent = choose_swap_extent(&swapChainSupport.capabilities, engine->swapChainExtent);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = engine->surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = find_queue_families(engine, engine->physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  vk_check(vkCreateSwapchainKHR(engine->device, &createInfo, NULL, &engine->swapChain), "failed to create swap chain");

  vkGetSwapchainImagesKHR(engine->device, engine->swapChain, &imageCount, NULL);
  engine->swapChainImages = malloc(sizeof(VkImage) * imageCount);
  vkGetSwapchainImagesKHR(engine->device, engine->swapChain, &imageCount, engine->swapChainImages);
  engine->swapChainImageCount = imageCount;

  engine->swapChainImageFormat = surfaceFormat.format;
  engine->swapChainExtent = extent;

  free_swap_chain_support_details(&swapChainSupport);
}

static void create_image_views(VulkanEngine* engine) {
  engine->swapChainImageViews = malloc(sizeof(VkImageView) * engine->swapChainImageCount);

  for (uint32_t i = 0; i < engine->swapChainImageCount; i++) {
    VkImageViewCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = engine->swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = engine->swapChainImageFormat;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.layerCount = 1;

    vk_check(vkCreateImageView(engine->device, &createInfo, NULL, &engine->swapChainImageViews[i]),
             "failed to create image views");
  }
}

static void create_render_pass(VulkanEngine* engine) {
  VkAttachmentDescription colorAttachment = {0};
  colorAttachment.format = engine->swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {0};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency = {0};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo = {0};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  vk_check(vkCreateRenderPass(engine->device, &renderPassInfo, NULL, &engine->renderPass),
           "failed to create render pass");
}

static void create_graphics_pipeline(VulkanEngine* engine) {
  size_t vertSize, fragSize;
  char* vertCode = read_file("shaders/vert.spv", &vertSize);
  char* fragCode = read_file("shaders/frag.spv", &fragSize);

  VkShaderModule vertShaderModule = create_shader_module(engine, vertCode, vertSize);
  VkShaderModule fragShaderModule = create_shader_module(engine, fragCode, fragSize);

  free(vertCode);
  free(fragCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState = {0};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer = {0};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling = {0};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending = {0};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState = {0};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  vk_check(vkCreatePipelineLayout(engine->device, &pipelineLayoutInfo, NULL, &engine->pipelineLayout),
           "failed to create pipeline layout");

  VkGraphicsPipelineCreateInfo pipelineInfo = {0};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = NULL;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = engine->pipelineLayout;
  pipelineInfo.renderPass = engine->renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  vk_check(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &engine->graphicsPipeline),
           "failed to create graphics pipeline");

  vkDestroyShaderModule(engine->device, fragShaderModule, NULL);
  vkDestroyShaderModule(engine->device, vertShaderModule, NULL);
}

static void create_framebuffers(VulkanEngine* engine) {
  engine->swapChainFramebuffers = malloc(sizeof(VkFramebuffer) * engine->swapChainImageCount);

  for (uint32_t i = 0; i < engine->swapChainImageCount; i++) {
    VkImageView attachments[] = {engine->swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo = {0};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = engine->renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = engine->swapChainExtent.width;
    framebufferInfo.height = engine->swapChainExtent.height;
    framebufferInfo.layers = 1;

    vk_check(vkCreateFramebuffer(engine->device, &framebufferInfo, NULL, &engine->swapChainFramebuffers[i]),
             "failed to create framebuffer");
  }
}

static void create_command_pool(VulkanEngine* engine) {
  QueueFamilyIndices queueFamilyIndices = find_queue_families(engine, engine->physicalDevice);

  VkCommandPoolCreateInfo poolInfo = {0};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

  vk_check(vkCreateCommandPool(engine->device, &poolInfo, NULL, &engine->commandPool), "failed to create command pool");
}

static void create_command_buffers(VulkanEngine* engine) {
  VkCommandBufferAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = engine->commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

  vk_check(vkAllocateCommandBuffers(engine->device, &allocInfo, engine->commandBuffers),
           "failed to allocate command buffers");
}

static void create_sync_objects(VulkanEngine* engine) {
  VkSemaphoreCreateInfo semaphoreInfo = {0};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {0};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk_check(vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, &engine->imageAvailableSemaphores[i]),
             "failed to create semaphore");
    vk_check(vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, &engine->renderFinishedSemaphores[i]),
             "failed to create semaphore");
    vk_check(vkCreateFence(engine->device, &fenceInfo, NULL, &engine->inFlightFences[i]), "failed to create fence");
  }
}

static void recreate_swap_chain(VulkanEngine* engine) {
  vkDeviceWaitIdle(engine->device);

  cleanup_swap_chain(engine);

  create_swap_chain(engine);
  create_image_views(engine);
  create_framebuffers(engine);
}

static void cleanup_swap_chain(VulkanEngine* engine) {
  for (uint32_t i = 0; i < engine->swapChainImageCount; i++) {
    vkDestroyFramebuffer(engine->device, engine->swapChainFramebuffers[i], NULL);
  }
  free(engine->swapChainFramebuffers);

  for (uint32_t i = 0; i < engine->swapChainImageCount; i++) {
    vkDestroyImageView(engine->device, engine->swapChainImageViews[i], NULL);
  }
  free(engine->swapChainImageViews);

  vkDestroySwapchainKHR(engine->device, engine->swapChain, NULL);
  free(engine->swapChainImages);
}

static VkShaderModule create_shader_module(VulkanEngine* engine, const char* code, size_t size) {
  VkShaderModuleCreateInfo createInfo = {0};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = size;
  createInfo.pCode = (const uint32_t*)code;

  VkShaderModule shaderModule;
  vk_check(vkCreateShaderModule(engine->device, &createInfo, NULL, &shaderModule), "failed to create shader module");

  return shaderModule;
}

static VkSurfaceFormatKHR choose_swap_surface_format(const VkSurfaceFormatKHR* available_formats, uint32_t count) {
  for (uint32_t i = 0; i < count; i++) {
    if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_formats[i];
    }
  }
  return available_formats[0];
}

static VkPresentModeKHR choose_swap_present_mode(const VkPresentModeKHR* available_present_modes, uint32_t count) {
  for (uint32_t i = 0; i < count; i++) {
    if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_modes[i];
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR* capabilities, VkExtent2D actual_extent) {
  if (capabilities->currentExtent.width != UINT32_MAX) {
    return capabilities->currentExtent;
  }
  else {
    VkExtent2D extent = actual_extent;
    extent.width = CLAMP(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    extent.height = CLAMP(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);
    return extent;
  }
}

static SwapChainSupportDetails query_swap_chain_support(VulkanEngine* engine, VkPhysicalDevice dev) {
  SwapChainSupportDetails details = {0};

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, engine->surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(dev, engine->surface, &formatCount, NULL);

  if (formatCount != 0) {
    details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    details.formatCount = formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, engine->surface, &formatCount, details.formats);
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(dev, engine->surface, &presentModeCount, NULL);

  if (presentModeCount != 0) {
    details.presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    details.presentModeCount = presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, engine->surface, &presentModeCount, details.presentModes);
  }

  return details;
}

static void free_swap_chain_support_details(SwapChainSupportDetails* details) {
  free(details->formats);
  free(details->presentModes);
}

static bool is_device_suitable(VulkanEngine* engine, VkPhysicalDevice dev) {
  QueueFamilyIndices indices = find_queue_families(engine, dev);

  bool extensionsSupported = check_device_extension_support(dev);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = query_swap_chain_support(engine, dev);
    swapChainAdequate = (swapChainSupport.formatCount > 0) && (swapChainSupport.presentModeCount > 0);
    free_swap_chain_support_details(&swapChainSupport);
  }

  return indices.hasGraphicsFamily && indices.hasPresentFamily && extensionsSupported && swapChainAdequate;
}

static bool check_device_extension_support(VkPhysicalDevice dev) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(dev, NULL, &extensionCount, NULL);

  VkExtensionProperties* availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
  vkEnumerateDeviceExtensionProperties(dev, NULL, &extensionCount, availableExtensions);

  bool swapChainFound = false;
  for (uint32_t i = 0; i < extensionCount; i++) {
    if (strcmp(availableExtensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      swapChainFound = true;
      break;
    }
  }

  free(availableExtensions);
  return swapChainFound;
}

static QueueFamilyIndices find_queue_families(VulkanEngine* engine, VkPhysicalDevice dev) {
  QueueFamilyIndices indices = {0};

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, NULL);

  VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, queueFamilies);

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.hasGraphicsFamily = true;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, engine->surface, &presentSupport);

    if (presentSupport) {
      indices.presentFamily = i;
      indices.hasPresentFamily = true;
    }

    if (indices.hasGraphicsFamily && indices.hasPresentFamily) {
      break;
    }
  }

  free(queueFamilies);
  return indices;
}
