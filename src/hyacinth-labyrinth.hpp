#pragma once

#include "vulkan/vulkan-descriptors.hpp"
#include "vulkan/vulkan-device.hpp"
#include "game/lve_game_object.hpp"
#include "vulkan/vulkan-renderer.hpp"
#include "window/glfw-window.hpp"
#include "game/maze.h"

// std
#include <memory>
#include <vector>

class HyacinthLabyrinth {
 public:
<<<<<<< HEAD
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 720;
=======
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;
>>>>>>> lsystems2

  HyacinthLabyrinth();
  ~HyacinthLabyrinth();

  HyacinthLabyrinth(const HyacinthLabyrinth &) = delete;
  HyacinthLabyrinth &operator=(const HyacinthLabyrinth &) = delete;

  void run();

 private:
  void loadGameObjects();
  void generateMazeFromBoolVec(std::vector<std::vector<bool>>& map);
  
  GameMaze m_maze;
  GlfwWindow m_window;
  VKDeviceManager m_device;
  VKRenderer m_renderer;
  id_t m_ball_id;
  id_t m_ball_light_id;

  // note: order of declarations matters
  std::unique_ptr<VK_DP_Mgr> globalPool{};
  LveGameObject::Map gameObjects;
};
