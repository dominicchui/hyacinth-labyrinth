#pragma once

#include "vulkan-device.hpp"
#include "vulkan-swapchain.hpp"
#include "glfw-window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

class VKRenderer {
 public:
  VKRenderer(GlfwWindow& window, VKDeviceManager& device);
  ~VKRenderer();

  VKRenderer(const VKRenderer&) = delete;
  VKRenderer &operator=(const VKRenderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const {
      return m_swapChain->getRenderPass();
  }
  float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
    return m_commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  GlfwWindow& m_window;
  VKDeviceManager& m_device;
  std::unique_ptr<VKSwapChain> m_swapChain;
  std::vector<VkCommandBuffer> m_commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};
