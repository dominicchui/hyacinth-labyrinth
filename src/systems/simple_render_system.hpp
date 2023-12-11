#pragma once

#include "game/lve_camera.hpp"
#include "game/lve_game_object.hpp"

#include "vulkan/vulkan-device.hpp"
#include "vulkan/vulkan-frame-info.hpp"
#include "vulkan/vulkan-pipeline.hpp"

// std
#include <memory>
#include <vector>

class SimpleRenderSystem {
 public:
  SimpleRenderSystem(
      VKDeviceManager& device,
      VkRenderPass renderPass,
      VkDescriptorSetLayout globalSetLayout
  );
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(FrameInfo &frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  VKDeviceManager& m_device;

  std::unique_ptr<VulkanPipeline> m_pipeline;
  VkPipelineLayout pipelineLayout;
};
