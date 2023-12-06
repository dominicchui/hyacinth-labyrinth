#pragma once

// #include "lve_descriptors.hpp"
#include "vulkan-device.hpp"
// #include "lve_game_object.hpp"
// #include "lve_renderer.hpp"
#include "glfw-window.hpp"

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

  // GlfwWindow m_window(WIDTH, HEIGHT, "Hyacinth Labyrinth");
  // VKDeviceManager m_device(m_window);
  // LveRenderer m_renderer{lveWindow, lveDevice};

  // // note: order of declarations matters
  // std::unique_ptr<LveDescriptorPool> globalPool{};
  // LveGameObject::Map gameObjects;
};
