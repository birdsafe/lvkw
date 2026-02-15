#include "window_module.hpp"
#include "imgui.h"
#include <cstdio>

WindowModule::WindowModule(lvkw::Window &primary_window, VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, uint32_t queue_family, VkQueue queue, VkDescriptorPool descriptor_pool)
    : primary_window_(primary_window), instance_(instance), physical_device_(physical_device), device_(device), queue_family_(queue_family), queue_(queue), descriptor_pool_(descriptor_pool) {}

WindowModule::~WindowModule() {
    for (auto &sw : secondary_windows_) {
        cleanupSecondaryWindow(*sw);
    }
}

void WindowModule::update(lvkw::Context &ctx, lvkw::Window &window) {
    if (!enabled_) return;

    // Handle events for secondary windows
    lvkw::scanEvents(ctx, [&](lvkw::WindowCloseEvent e) {
        for (auto &sw : secondary_windows_) {
            if (sw->window->get() == e.window) {
                sw->should_close = true;
            }
        }
    });

    // Remove closed windows
    secondary_windows_.erase(
        std::remove_if(secondary_windows_.begin(), secondary_windows_.end(),
                       [&](auto &sw) {
                           if (sw->should_close) {
                               cleanupSecondaryWindow(*sw);
                               return true;
                           }
                           return false;
                       }),
        secondary_windows_.end());

    // Render secondary windows
    for (auto &sw : secondary_windows_) {
        if (!sw->window->isReady()) continue;

        // Skip resizing if we haven't created the surface yet. 
        // renderSecondaryWindow will handle the initial creation.
        if (sw->surface != VK_NULL_HANDLE) {
            auto geom = sw->window->getGeometry();
            if (geom.pixelSize.x > 0 && geom.pixelSize.y > 0 &&
                (sw->swapchain_rebuild || sw->wd.Width != geom.pixelSize.x || sw->wd.Height != geom.pixelSize.y)) {
                
                ImGui_ImplVulkanH_CreateOrResizeWindow(instance_, physical_device_, device_, &sw->wd, queue_family_, nullptr, geom.pixelSize.x, geom.pixelSize.y, 2, 0);
                sw->wd.FrameIndex = 0;
                sw->swapchain_rebuild = false;
            }
        }

        renderSecondaryWindow(*sw);
    }
}

void WindowModule::render(lvkw::Context &ctx, lvkw::Window &window) {
  if (!enabled_)
    return;

  ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Windowing", &enabled_)) {
    ImGui::End();
    return;
  }

  if (ImGui::Button("Create Secondary Window")) {
      createSecondaryWindow(ctx);
  }

  ImGui::Separator();
  if (ImGui::CollapsingHeader("Primary Window", ImGuiTreeNodeFlags_DefaultOpen)) {
      static bool primary_resizable = true;
      static bool primary_passthrough = false;
      static LVKW_LogicalVec primary_min = {0, 0};
      static LVKW_LogicalVec primary_max = {0, 0};
      static LVKW_Fraction primary_fraction = {0, 0};

      auto render_primary_controls = [&]() {
          LVKW_Window* handle = primary_window_.get();
          ImGui::Text("Flags: %s%s%s%s%s", 
              (handle->flags & LVKW_WND_STATE_READY) ? "[Ready] " : "",
              (handle->flags & LVKW_WND_STATE_FOCUSED) ? "[Focused] " : "",
              (handle->flags & LVKW_WND_STATE_MAXIMIZED) ? "[Maximized] " : "",
              (handle->flags & LVKW_WND_STATE_FULLSCREEN) ? "[Fullscreen] " : "",
              (handle->flags & LVKW_WND_STATE_LOST) ? "[LOST] " : "");

          LVKW_WindowGeometry geom = primary_window_.getGeometry();
          ImGui::Text("Origin:  %.1f, %.1f", geom.origin.x, geom.origin.y);
          ImGui::Text("Logical: %.1f x %.1f", geom.logicalSize.x, geom.logicalSize.y);
          ImGui::Text("Pixel:   %d x %d", geom.pixelSize.x, geom.pixelSize.y);

          bool is_max = primary_window_.isMaximized();
          bool is_fs = primary_window_.isFullscreen();
          if (ImGui::Checkbox("Maximized State", &is_max)) primary_window_.setMaximized(is_max);
          ImGui::SameLine();
          if (ImGui::Checkbox("Fullscreen State", &is_fs)) primary_window_.setFullscreen(is_fs);
          
          if (ImGui::Checkbox("Decorated (Tracked)", &primary_is_decorated_)) primary_window_.setDecorated(primary_is_decorated_);
          ImGui::SameLine();
          if (ImGui::Checkbox("Resizable", &primary_resizable)) primary_window_.setResizable(primary_resizable);
          ImGui::SameLine();
          if (ImGui::Checkbox("Mouse Passthrough", &primary_passthrough)) primary_window_.setMousePassthrough(primary_passthrough);

#ifdef LVKW_USE_FLOAT
          ImGuiDataType scalar_type = ImGuiDataType_Float;
#else
          ImGuiDataType scalar_type = ImGuiDataType_Double;
#endif

          if (ImGui::InputScalarN("Min Size", scalar_type, &primary_min.x, 2, NULL, NULL, "%.0f")) primary_window_.setMinSize(primary_min);
          if (ImGui::InputScalarN("Max Size", scalar_type, &primary_max.x, 2, NULL, NULL, "%.0f")) primary_window_.setMaxSize(primary_max);
          if (ImGui::InputInt2("Aspect Fraction (num/den)", &primary_fraction.numerator)) primary_window_.setAspectRatio(primary_fraction);

          if (ImGui::Button("Request Focus")) primary_window_.requestFocus();
      };
      render_primary_controls();
  }

  ImGui::Separator();
  ImGui::Text("Secondary Windows: %zu", secondary_windows_.size());

  for (size_t i = 0; i < secondary_windows_.size(); ++i) {
      auto &sw = *secondary_windows_[i];
      ImGui::PushID(static_cast<int>(i));
      if (ImGui::CollapsingHeader(sw.title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
          
          char title_buf[256];
          strncpy(title_buf, sw.title.c_str(), sizeof(title_buf));
          if (ImGui::InputText("Title", title_buf, sizeof(title_buf))) {
              sw.title = title_buf;
              sw.window->setTitle(sw.title.c_str());
          }

          LVKW_Window* handle = sw.window->get();
          ImGui::Text("Flags: %s%s%s%s%s", 
              (handle->flags & LVKW_WND_STATE_READY) ? "[Ready] " : "",
              (handle->flags & LVKW_WND_STATE_FOCUSED) ? "[Focused] " : "",
              (handle->flags & LVKW_WND_STATE_MAXIMIZED) ? "[Maximized] " : "",
              (handle->flags & LVKW_WND_STATE_FULLSCREEN) ? "[Fullscreen] " : "",
              (handle->flags & LVKW_WND_STATE_LOST) ? "[LOST] " : "");

          LVKW_WindowGeometry geom = sw.window->getGeometry();
          ImGui::Text("Origin:  %.1f, %.1f", geom.origin.x, geom.origin.y);
          ImGui::Text("Logical: %.1f x %.1f", geom.logicalSize.x, geom.logicalSize.y);
          ImGui::Text("Pixel:   %d x %d", geom.pixelSize.x, geom.pixelSize.y);

          bool is_max = sw.window->isMaximized();
          bool is_fs = sw.window->isFullscreen();
          if (ImGui::Checkbox("Maximized State", &is_max)) sw.window->setMaximized(is_max);
          ImGui::SameLine();
          if (ImGui::Checkbox("Fullscreen State", &is_fs)) sw.window->setFullscreen(is_fs);

          if (ImGui::Checkbox("Decorated", &sw.is_decorated)) sw.window->setDecorated(sw.is_decorated);
          ImGui::SameLine();
          if (ImGui::Checkbox("Resizable", &sw.is_resizable)) sw.window->setResizable(sw.is_resizable);
          ImGui::SameLine();
          if (ImGui::Checkbox("Mouse Passthrough", &sw.mouse_passthrough)) sw.window->setMousePassthrough(sw.mouse_passthrough);

#ifdef LVKW_USE_FLOAT
          ImGuiDataType scalar_type = ImGuiDataType_Float;
#else
          ImGuiDataType scalar_type = ImGuiDataType_Double;
#endif

          if (ImGui::InputScalarN("Min Size", scalar_type, &sw.min_size.x, 2, NULL, NULL, "%.0f")) sw.window->setMinSize(sw.min_size);
          if (ImGui::InputScalarN("Max Size", scalar_type, &sw.max_size.x, 2, NULL, NULL, "%.0f")) sw.window->setMaxSize(sw.max_size);
          if (ImGui::InputInt2("Aspect Fraction (num/den)", &sw.aspect_ratio.numerator)) sw.window->setAspectRatio(sw.aspect_ratio);

          ImGui::ColorEdit3("Clear Color", (float*)&sw.clear_color);

          if (ImGui::Button("Request Focus")) sw.window->requestFocus();
          ImGui::SameLine();
          if (ImGui::Button("Close")) sw.should_close = true;
      }
      ImGui::PopID();
  }

  ImGui::End();
}

void WindowModule::createSecondaryWindow(lvkw::Context &ctx) {
    LVKW_WindowCreateInfo win_info = LVKW_WINDOW_CREATE_INFO_DEFAULT;
    static int win_count = 0;
    std::string title = "Secondary Window " + std::to_string(++win_count);
    win_info.attributes.title = title.c_str();
    win_info.attributes.logicalSize = {640, 480};

    auto sw = std::make_unique<SecondaryWindow>();
    sw->title = title;
    sw->window = std::make_unique<lvkw::Window>(ctx.createWindow(win_info));
    
    // We can't create the surface yet, we must wait for it to be ready.
    // In this simple testbed, we'll just poll for readiness in update.
    
    secondary_windows_.push_back(std::move(sw));
}

static void check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
}

void WindowModule::renderSecondaryWindow(SecondaryWindow &sw) {
    if (sw.surface == VK_NULL_HANDLE) {
        if (sw.window->isReady()) {
            sw.surface = sw.window->createVkSurface(instance_);
            
            // Setup Vulkan Window Data
            sw.wd.Surface = sw.surface;
            
            // Select Surface Format
            const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
            const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            sw.wd.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physical_device_, sw.wd.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

            // Select Present Mode
            VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
            sw.wd.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physical_device_, sw.wd.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

            auto geom = sw.window->getGeometry();
            ImGui_ImplVulkanH_CreateOrResizeWindow(instance_, physical_device_, device_, &sw.wd, queue_family_, nullptr, geom.pixelSize.x, geom.pixelSize.y, 2, 0);
        } else {
            return;
        }
    }

    // Simple clear-only frame render
    ImGui_ImplVulkanH_Window* wd = &sw.wd;
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(device_, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        sw.swapchain_rebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(device_, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
        check_vk_result(err);

        err = vkResetFences(device_, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(device_, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        
        wd->ClearValue.color.float32[0] = sw.clear_color.x * sw.clear_color.w;
        wd->ClearValue.color.float32[1] = sw.clear_color.y * sw.clear_color.w;
        wd->ClearValue.color.float32[2] = sw.clear_color.z * sw.clear_color.w;
        wd->ClearValue.color.float32[3] = sw.clear_color.w;

        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // No ImGui rendering here, just clear.

    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(queue_, 1, &info, fd->Fence);
        check_vk_result(err);
    }

    // Present
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    err = vkQueuePresentKHR(queue_, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        sw.swapchain_rebuild = true;
        return;
    }
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;
}

void WindowModule::cleanupSecondaryWindow(SecondaryWindow &sw) {
    vkDeviceWaitIdle(device_);
    ImGui_ImplVulkanH_DestroyWindow(instance_, device_, &sw.wd, nullptr);
    sw.window.reset();
}
