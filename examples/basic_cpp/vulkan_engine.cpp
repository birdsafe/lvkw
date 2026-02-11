#include "vulkan_engine.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>

#include "utils.hpp"

static std::vector<char> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + filename);
  }
  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
  file.close();
  return buffer;
}

void VulkanEngine::init(lvkw::Context &ctx, lvkw::Window &window, const std::vector<const char *> &extensions) {
  createInstance(ctx, extensions);
  surface = window.createVkSurface(instance);
  if (!surface) {
    throw std::runtime_error("failed to create window surface!");
  }

  auto geometry = window.getGeometry();
  swapChainExtent = {geometry.physicalSize.width, geometry.physicalSize.height};

  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
  createRenderPass();
  createGraphicsPipeline();
  createFramebuffers();
  createCommandPool();
  createCommandBuffers();
  createSyncObjects();
}

void VulkanEngine::createInstance(lvkw::Context &ctx, const std::vector<const char *> &extensions) {
  (void)ctx;
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "LVKW Showcase";
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  vk_check(vkCreateInstance(&createInfo, nullptr, &instance), "failed to create instance");
}

void VulkanEngine::cleanup() {
  vkDeviceWaitIdle(device);
  cleanupSwapChain();
  vkDestroyPipeline(device, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
  vkDestroyRenderPass(device, renderPass, nullptr);
  for (size_t i = 0; i < imageAvailableSemaphores.size(); i++) {
    vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device, inFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(device, commandPool, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);
}

void VulkanEngine::onResized(uint32_t width, uint32_t height) {
  framebufferResized = true;
  swapChainExtent = {width, height};
}

void VulkanEngine::drawFrame() {
  if (framebufferResized) {
    recreateSwapChain();
    framebufferResized = false;
  }

  vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
                                          VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    framebufferResized = true;
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(device, 1, &inFlightFences[currentFrame]);
  vkResetCommandBuffer(commandBuffers[currentFrame], 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapChainExtent;

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

  VkViewport viewport{};
  viewport.width = static_cast<float>(swapChainExtent.width);
  viewport.height = static_cast<float>(swapChainExtent.height);
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.extent = swapChainExtent;
  vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

  vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
  vkCmdEndRenderPass(commandBuffers[currentFrame]);
  vkEndCommandBuffer(commandBuffers[currentFrame]);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vk_check(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]),
           "failed to submit draw command buffer");

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapChain;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    framebufferResized = true;
  }
  else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  currentFrame = (currentFrame + 1) % 2;
}

void VulkanEngine::recreateSwapChain() {
  vkDeviceWaitIdle(device);
  cleanupSwapChain();
  createSwapChain();
  createImageViews();
  createFramebuffers();
}

void VulkanEngine::cleanupSwapChain() {
  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
  }
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(device, imageView, nullptr);
  }
  vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanEngine::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vk_check(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr), "failed to enumerate physical devices");
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vk_check(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()), "failed to enumerate physical devices");
  for (const auto &d : devices) {
    if (isDeviceSuitable(d)) {
      physicalDevice = d;
      break;
    }
  }
}

void VulkanEngine::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }
  VkPhysicalDeviceFeatures deviceFeatures{};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();
  vk_check(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device), "failed to create logical device");
  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanEngine::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
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
  vk_check(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain), "failed to create swap chain");
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vk_check(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()),
           "failed to get swap chain images");
  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void VulkanEngine::createImageViews() {
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapChainImageFormat;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.layerCount = 1;
    vk_check(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]), "failed to create image views");
  }
}

void VulkanEngine::createRenderPass() {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;
  vk_check(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass), "failed to create render pass");
}

void VulkanEngine::createGraphicsPipeline() {
  auto vert = readFile("shaders/vert.spv");
  auto frag = readFile("shaders/frag.spv");
  VkShaderModule vertMod = createShaderModule(vert);
  VkShaderModule fragMod = createShaderModule(frag);
  VkPipelineShaderStageCreateInfo stages[2] = {};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vertMod;
  stages[0].pName = "main";
  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fragMod;
  stages[1].pName = "main";
  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = 0xf;
  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  vk_check(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout),
           "failed to create pipeline layout");
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = stages;
  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  vk_check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline),
           "failed to create graphics pipeline");
  vkDestroyShaderModule(device, fragMod, nullptr);
  vkDestroyShaderModule(device, vertMod, nullptr);
}

void VulkanEngine::createFramebuffers() {
  swapChainFramebuffers.resize(swapChainImageViews.size());
  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;
    vk_check(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]),
             "failed to create framebuffer");
  }
}

void VulkanEngine::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  vk_check(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool), "failed to create command pool");
}

void VulkanEngine::createCommandBuffers() {
  commandBuffers.resize(2);
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 2;
  vk_check(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()), "failed to allocate command buffers");
}

void VulkanEngine::createSyncObjects() {
  imageAvailableSemaphores.resize(2);
  renderFinishedSemaphores.resize(2);
  inFlightFences.resize(2);
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (size_t i = 0; i < 2; i++) {
    vk_check(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]),
             "failed to create semaphore");
    vk_check(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]),
             "failed to create semaphore");
    vk_check(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]), "failed to create fence");
  }
}

VkShaderModule VulkanEngine::createShaderModule(const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  VkShaderModule shaderModule;
  vk_check(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "failed to create shader module");
  return shaderModule;
}

VkSurfaceFormatKHR VulkanEngine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &f : availableFormats) {
    if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return f;
  }
  return availableFormats[0];
}

VkPresentModeKHR VulkanEngine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &p : availablePresentModes) {
    if (p == VK_PRESENT_MODE_MAILBOX_KHR) return p;
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanEngine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  else {
    VkExtent2D actualExtent = swapChainExtent;
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
  }
}

SwapChainSupportDetails VulkanEngine::querySwapChainSupport(VkPhysicalDevice dev) {
  SwapChainSupportDetails details;
  vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &details.capabilities),
           "failed to get surface capabilities");
  uint32_t formatCount;
  vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, nullptr), "failed to get surface formats");
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, details.formats.data()),
             "failed to get surface formats");
  }
  uint32_t presentModeCount;
  vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, nullptr),
           "failed to get surface present modes");
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, details.presentModes.data()),
             "failed to get surface present modes");
  }
  return details;
}

bool VulkanEngine::isDeviceSuitable(VkPhysicalDevice dev) {
  QueueFamilyIndices indices = findQueueFamilies(dev);
  bool extensionsSupported = checkDeviceExtensionSupport(dev);
  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(dev);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanEngine::checkDeviceExtensionSupport(VkPhysicalDevice dev) {
  uint32_t extensionCount;
  vk_check(vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr),
           "failed to enumerate device extensions");
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vk_check(vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, availableExtensions.data()),
           "failed to enumerate device extensions");
  std::set<std::string> requiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

QueueFamilyIndices VulkanEngine::findQueueFamilies(VkPhysicalDevice dev) {
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, queueFamilies.data());
  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(dev, static_cast<uint32_t>(i), surface, &presentSupport);
    if (presentSupport) indices.presentFamily = i;
    if (indices.isComplete()) break;
    i++;
  }
  return indices;
}
