#include "hyacinth-labyrinth.hpp"

#include "game/keyboard_movement_controller.hpp"
#include "maze/maze.h"
#include "game/maze.h"
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

HyacinthLabyrinth::HyacinthLabyrinth()
  : m_window(WIDTH, HEIGHT, "Hyacinth Labrynth"),
    m_device(m_window),
    m_renderer(m_window, m_device
) {
    // JANKTEX
    globalPool =
      VK_DP_Mgr::Builder(m_device)
          .setMaxSets(VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          // .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          // .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)
          // .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VKSwapChain::MAX_FRAMES_IN_FLIGHT)

          .build();
    loadGameObjects();
}

HyacinthLabyrinth::~HyacinthLabyrinth() {}

void HyacinthLabyrinth::run() {
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

  // JANKTEX
  auto globalSetLayout =
      VK_DSL_Mgr::Builder(m_device)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          // .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          // .addBinding(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          // .addBinding(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  auto& ball = gameObjects.at(m_ball_id);

  // HACK
  VkDescriptorImageInfo imageInfos[m_device.cur_texture];
  std::cout << "JANKTEX: we are using: " << m_device.cur_texture << " textures" << std::endl;

  for (int32_t i = 0; i < m_device.cur_texture; i++) {
      imageInfos[i] = VkDescriptorImageInfo{
          m_device.textureSampler[i],
          m_device.textureImageView[i],
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      };
  }

  // JANKTEX
  std::vector<VkDescriptorSet> globalDescriptorSets(VKSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    VKDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imageInfos[0])
        .writeImage(2, &imageInfos[1])
        .writeImage(3, &imageInfos[2])
        .writeImage(4, &imageInfos[3])
        .writeImage(5, &imageInfos[4])
        // .writeImage(6, &imageInfos[5])
        // .writeImage(7, &imageInfos[6])
        // .writeImage(8, &imageInfos[7])
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
//  glm::vec4 cam_pos(-5.f, -12.f, 5.f, 1.f);
  glm::vec4 cam_pos(1.f, -11.f, 7.f, 1.f);
  glm::vec4 focus_at(0.f, 0.f, 0.f, 1.f);
  SceneCameraData scd{
      cam_pos, // pos
      focus_at - cam_pos,  // look
      glm::vec4(0.f, 1.f, 0.f, 0.f),   // up
      M_PI/4.f, // height angle
      0, // DoF
      0  // focal length
  };

  Camera camera(CAM_PROJ_PERSP);
  camera.initScene(scd, WIDTH, HEIGHT, 0.1f, 100.f);
  camera.recomputeMatrices(ball.transform.translation);

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;


  KeyboardMovementController cameraController{};
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
    bool did_move =
        cameraController.moveCameraNoRot(
            m_window.getGLFWwindow(),
            frameTime,
            camera
        );

    camera.recomputeMatrices(ball.transform.translation);

    ballController.moveInPlaneXZ(
        m_window.getGLFWwindow(),
        frameTime,
        gameObjects.at(m_ball_id),
        &m_maze
    );
    // move the lights with the ball
    gameObjects.at(m_ball_light_id).transform.translation = gameObjects.at(m_ball_id).transform.translation;
//    for (int i=0; i<point_light_ids.size(); i++) {
//        auto rotateLight = glm::rotate(
//            glm::mat4(1.f),
//            (i * glm::two_pi<float>()) / point_light_ids.size(),
//            {0.f, 1.f, 0.f});
//        gameObjects.at(point_light_ids[i]).transform.translation = glm::vec3(rotateLight * glm::vec4(gameObjects.at(m_ball_id).transform.translation,1.0f)) + glm::vec3(0.f,-2.f,0.f);
//    }

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

void HyacinthLabyrinth::loadGameObjects() {
    std::shared_ptr<VKModel> model =
        VKModel::createModelFromFile(m_device, "resources/models/ball.obj", true);
    auto ball = LveGameObject::createGameObject();
    ball.model = model;
    ball.transform.translation = {-.5f, 0.5f, 0.f};
//    ball.transform.scale = {0.25f, 0.25f, 0.25f};
    ball.transform.scale = {0.3f, 0.3f, 0.3f};
    ball.transform.update_matrices();
//    ball.phys.radius = ball.transform.scale.x;
    ball.phys.radius = 0.25f;
    m_ball_id = ball.getId();
    gameObjects.emplace(m_ball_id, std::move(ball));


    // add light to ball
    auto ballLight = LveGameObject::makePointLight(0.2f);
    ballLight.transform.scale = {0.5f, 0.5f, 0.5f};
    ballLight.color = glm::vec3(0.8f);
    m_ball_light_id = ballLight.getId();
    gameObjects.emplace(m_ball_light_id, std::move(ballLight));

  model = VKModel::createModelFromFile(m_device,
                                        "resources/models/lsys.obj",
                                         true, glm::vec3(1.f, 0.1f, 0.1f));
  auto smoothVase = LveGameObject::createGameObject();
  smoothVase.model = model;
  smoothVase.transform.translation = {.5f, .5f, 0.f};
  smoothVase.transform.scale = {0.08f, -0.08f, 0.08f};
  smoothVase.transform.update_matrices();
  gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

  model = VKModel::createModelFromFile(m_device,
                                       "resources/models/quad.obj",
                                       true, glm::vec3(1.f, 1.f, 1.f));
  auto floor = LveGameObject::createGameObject();
  floor.model = model;
  floor.transform.translation = {0.f, 1.f, 0.f};
  floor.transform.scale = {50.f, 1.f, 50.f};
  floor.transform.update_matrices();
  gameObjects.emplace(floor.getId(), std::move(floor));

  //// Generate the maze:
  Maze maze = Maze(5,5);
  maze.generate();
  //std::cout << maze.toString() << std::endl;
  std::vector<std::vector<bool>> map = maze.toBoolVector();
//  std::vector<std::vector<bool>> map = {
//      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
//      {1, 0, 0, 1, 0, 1, 1, 1, 1, 1},
//      {1, 0, 0, 1, 1, 1, 0, 0, 0, 1},
//      {1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
//      {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//      {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
//      {1, 0, 0, 0, 0, 0, 0, 0, 1, 1},
//      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
//  };
  m_maze.generateMazeFromBoolVec(m_device, map);
  m_maze.exportMazeVisibleGeometry(m_device, gameObjects);


 // Sun
  auto pointLight = LveGameObject::makePointLight(90.f);
  pointLight.color = glm::vec3(1.f, 1.f, 1.f);
//  pointLight.color = glm::vec3(.98f, .84f, .11f);
  pointLight.transform.translation = glm::vec3(0.f, -20.f, 0.f);
  pointLight.transform.update_matrices();
  gameObjects.emplace(pointLight.getId(), std::move(pointLight));
}
