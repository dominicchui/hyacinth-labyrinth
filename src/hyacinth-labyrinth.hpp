#pragma once

#include "vulkan/vulkan-descriptors.hpp"
#include "vulkan/vulkan-device.hpp"
#include "game/lve_game_object.hpp"
#include "vulkan/vulkan-renderer.hpp"
#include "window/glfw-window.hpp"

// std
#include <memory>
#include <vector>

class HyacinthLabyrinth {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  HyacinthLabyrinth();
  ~HyacinthLabyrinth();

  HyacinthLabyrinth(const HyacinthLabyrinth &) = delete;
  HyacinthLabyrinth &operator=(const HyacinthLabyrinth &) = delete;

  void run();

 private:
  void loadGameObjects();

  GlfwWindow m_window;
  VKDeviceManager m_device;
  VKRenderer m_renderer;

  // note: order of declarations matters
  std::unique_ptr<VK_DP_Mgr> globalPool{};
  LveGameObject::Map gameObjects;
};
