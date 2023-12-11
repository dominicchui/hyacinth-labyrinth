// Code lifted largely from https://github.com/blurrypiano/littleVulkanEngine

#include "glfw-window.hpp"

// std
#include <stdexcept>

GlfwWindow::GlfwWindow(int w, int h, std::string name) :
    width(w),
    height(h),
    windowName(name) {
  initWindow();
}

GlfwWindow::~GlfwWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void GlfwWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void GlfwWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to craete window surface");
  }
}

void GlfwWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  auto lveWindow = reinterpret_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
  lveWindow->framebufferResized = true;
  lveWindow->width = width;
  lveWindow->height = height;
}
