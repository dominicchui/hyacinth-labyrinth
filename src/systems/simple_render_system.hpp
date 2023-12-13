#pragma once

#include "game/lve_camera.hpp"
#include "game/lve_game_object.hpp"

#include "vulkan/vulkan-descriptors.hpp"
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
        VkRenderPass shadowPass,
        VkDescriptorSetLayout globalSetLayout
        );
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem &) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

    void generateShadowMap(FrameInfo &frameInfo);
    void renderGameObjects(FrameInfo &frameInfo);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);
    void createShadowPipeline(VkRenderPass renderPass);


    VKDeviceManager& m_device;

    std::unique_ptr<VulkanPipeline> m_pipeline;
    std::unique_ptr<VulkanPipeline> m_shadow_pipeline;

    VkPipelineLayout pipelineLayout;

    VKDescriptorWriter *m_descriptorWriter;
    VkDescriptorSet m_descriptorSet;
};
