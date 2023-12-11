// Code lifted largely from https://github.com/blurrypiano/littleVulkanEngine

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class GlfwWindow {
 public:
  GlfwWindow(int w, int h, std::string name);
  ~GlfwWindow();

  // Disable copy constructors
  GlfwWindow(const GlfwWindow &) = delete;
  GlfwWindow& operator=(const GlfwWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }
  GLFWwindow *getGLFWwindow() const { return window; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

 private:
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  void initWindow();

  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;
  GLFWwindow *window;
};
