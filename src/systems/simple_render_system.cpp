#include "simple_render_system.hpp"
#include "vulkan/vulkan-swapchain.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <iostream>

struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
    int32_t tex_id;
};

SimpleRenderSystem::SimpleRenderSystem(
    VKDeviceManager& device,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout
    ) : m_device(device), m_descriptorSet()
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
    vkDestroyPipelineLayout(m_device.device(), pipelineLayout, nullptr);
    delete m_descriptorWriter;
}

void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // auto layout = VK_DSL_Mgr::Builder(m_device)
    //                   .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    //                   .build();
    // auto pool = VK_DP_Mgr::Builder(m_device)
    //                 .setMaxSets(VKSwapChain::MAX_FRAMES_IN_FLIGHT)
    //                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
    //                 .build();
    // m_descriptorWriter = new VKDescriptorWriter(*layout, *pool);
    // //m_descriptorWriter->writeImage(1, nullptr);
    // m_descriptorWriter->build(m_descriptorSet);
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");


  PipelineConfigInfo pipelineConfig{};
  VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  m_pipeline = std::make_unique<VulkanPipeline>(
      m_device,
      "simple_shader.vert.spv",
      "simple_shader.frag.spv",
      pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
    m_pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    for (auto& kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.model == nullptr) continue;
        SimplePushConstantData push{};
        push.modelMatrix = obj.transform.mat4;
        push.normalMatrix = obj.transform.normalMatrix;
        push.tex_id = obj.model->texture_id;

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);

        // // for image:
        // VkDescriptorImageInfo imageInfo{
        //     obj.model->textureSampler,
        //     obj.model->textureImageView,
        //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        // };
        // m_descriptorWriter->writeImage(1, &imageInfo /* image pointer */);
        // m_descriptorWriter->build(m_descriptorSet);

        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
    }
}
