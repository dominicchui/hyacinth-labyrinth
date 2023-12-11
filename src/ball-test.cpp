#include "ball-test.hpp"

#include "game/keyboard_movement_controller.hpp"
#include "vulkan/vulkan-buffer.hpp"
#include "renderer/camera.h"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"
#include "utils/utils.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

BallTest::BallTest()
  : m_window(WIDTH, HEIGHT, "Hyacinth Labrynth"),
    m_device(m_window),
    m_renderer(m_window, m_device)
{
  globalPool =
      VK_DP_Mgr::Builder(m_device)
          .setMaxSets(VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build();
  loadGameObjects();
}

BallTest::~BallTest() {}

void BallTest::run() {
  std::vector<std::unique_ptr<VKBufferMgr>> uboBuffers(VKSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<VKBufferMgr>(
        m_device,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }

  auto globalSetLayout =
      VK_DSL_Mgr::Builder(m_device)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
          .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(VKSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    VKDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem simpleRenderSystem{
      m_device,
      m_renderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()
  };

  PointLightSystem pointLightSystem{
      m_device,
      m_renderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()
  };

  // Create camera
  SceneCameraData scd{
      glm::vec4(0.f, 0.f, 4.f, 1.f),   // pos
      glm::vec4(-0.f, 0.f, -1.f, 0.f),  // look
      glm::vec4(0.f, 1.f, 0.f, 0.f),    // up
      M_PI/4.f, // height angle
      0, // DoF
      0  // focal length
  };

  Camera camera(CAM_PROJ_PERSP);
  camera.initScene(scd, WIDTH, HEIGHT, 0.1f, 100.f);
  camera.recomputeMatrices(glm::vec3(0.f));


  auto viewerObject = LveGameObject::createGameObject();
  KeyboardMovementController ballController{};

  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!m_window.shouldClose()) {
    bool do_update = false;
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    // TODO: Update the frame only when something changes
    ballController.moveInPlaneXZ(
        m_window.getGLFWwindow(),
        frameTime,
        gameObjects.at(0)
    );

    //camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    if (auto commandBuffer = m_renderer.beginFrame()) {
      int frameIndex = m_renderer.getFrameIndex();
      FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          gameObjects};

      // update
      GlobalUbo ubo{};
      ubo.projection = camera.proj_mat;
      ubo.view = camera.view_mat;
      ubo.inverseView = camera.view_mat_inv;
      pointLightSystem.update(frameInfo, ubo);
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      m_renderer.beginSwapChainRenderPass(commandBuffer);

      // order here matters
      simpleRenderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);

      m_renderer.endSwapChainRenderPass(commandBuffer);
      m_renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(m_device.device());
}

void BallTest::loadGameObjects() {
  std::shared_ptr<VKModel> model =
        VKModel::createModelFromFile(m_device, "resources/models/cube.obj");
  auto floor = LveGameObject::createGameObject();
  floor.model = model;
  floor.transform.translation = {0.f, 0.f, 0.f};
  floor.transform.scale = {0.5f, 0.5f, 0.5f};
  floor.transform.update_matrices();
  gameObjects.emplace(floor.getId(), std::move(floor));

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f},
      {.1f, .1f, 1.f},
      {.1f, 1.f, .1f},
      {1.f, 1.f, .1f},
      {.1f, 1.f, 1.f},
      {1.f, 1.f, 1.f}  //
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto pointLight = LveGameObject::makePointLight(0.2f);
    pointLight.color = lightColors[i];
    auto rotateLight = glm::rotate(
        glm::mat4(1.f),
        (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, 1.f, 0.f});
    pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
    pointLight.transform.update_matrices();
    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
}
