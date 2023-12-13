#pragma once

#include "vulkan-device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <vector>

class VKSwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
  static constexpr int SHADOW_MAP_WIDTH = 2048;
  static constexpr int SHADOW_MAP_HEIGHT = 2048;

  VKSwapChain(VKDeviceManager& deviceRef, VkExtent2D windowExtent);
  VKSwapChain(
      VKDeviceManager &deviceRef,
      VkExtent2D windowExtent,
      std::shared_ptr<VKSwapChain> previous
  );

  ~VKSwapChain();

  VKSwapChain(const VKSwapChain&) = delete;
  VKSwapChain &operator=(const VKSwapChain&) = delete;

  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
  VkRenderPass getRenderPass() { return renderPass; }

  VkFramebuffer getShadowFrameBuffer(int index) { return shadowMapFramebuffers[index]; }
  VkRenderPass getShadowRenderPass() { return shadowRenderPass; }

  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }

  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  VkExtent2D getShadowSwapChainExtent() { return {SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT}; }

  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  bool compareSwapFormats(const VKSwapChain& swapChain) const {
    return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
           swapChain.swapChainImageFormat == swapChainImageFormat;
  }

 private:
  void init();
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createShadowResources();
  void createRenderPass();
  void createShadowMapRenderPass();
  void createFramebuffers();
  void createShadowFramebuffers();
  void createSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat;
  VkFormat swapChainDepthFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  std::vector<VkFramebuffer> shadowMapFramebuffers;

  VkRenderPass renderPass;
  VkRenderPass shadowRenderPass;

  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;

  std::vector<VkImage> shadowImages;
  std::vector<VkDeviceMemory> shadowImageMemorys;
  // Moved to device for hacky reasons
  //std::vector<VkImageView> shadowImageViews;

  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  VKDeviceManager& m_device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<VKSwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};
