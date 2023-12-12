#pragma once

#include "maze/maze.h"
#include "game/maze.h"
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
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  HyacinthLabyrinth();
  ~HyacinthLabyrinth();

  HyacinthLabyrinth(const HyacinthLabyrinth &) = delete;
  HyacinthLabyrinth &operator=(const HyacinthLabyrinth &) = delete;

  void run();

 private:
  void loadGameObjects();
  void generateMazeFromBoolVec(std::vector<std::vector<bool>>& map);
  
  GameMaze m_maze;
  Maze maze;
  GlfwWindow m_window;
  VKDeviceManager m_device;
  VKRenderer m_renderer;
  id_t m_ball_id;
  id_t m_ball_light_id;

  // note: order of declarations matters
  std::unique_ptr<VK_DP_Mgr> globalPool{};
  LveGameObject::Map gameObjects;

  float maze_center_x = 0.f;
  float maze_center_z = 0.f;
};
