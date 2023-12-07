#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

#include "vulkan-device.hpp"
#include "vulkan-frame-info.hpp"
#include "vulkan-pipeline.hpp"

// std
#include <memory>
#include <vector>

class PointLightSystem {
 public:
  PointLightSystem(
      VKDeviceManager& device,
      VkRenderPass renderPass,
      VkDescriptorSetLayout globalSetLayout
  );
  ~PointLightSystem();

  PointLightSystem(const PointLightSystem &) = delete;
  PointLightSystem &operator=(const PointLightSystem &) = delete;

  void update(FrameInfo &frameInfo, GlobalUbo &ubo);
  void render(FrameInfo &frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  VKDeviceManager& m_device;

  std::unique_ptr<VulkanPipeline> m_pipeline;
  VkPipelineLayout pipelineLayout;
};
