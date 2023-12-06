cmake_minimum_required(VERSION 3.16)

# Sets project name
project(hyacinth-labyrinth LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp

    src/mainwindow.h
    src/realtime.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp
    src/utils/timer.h
    src/camera/camera.cpp src/camera/camera.h
    src/utils/debug.h
    src/rasterscene.cpp src/rasterscene.h
    src/shapes/Cone.cpp src/shapes/Cone.h src/shapes/Cube.cpp src/shapes/Cube.h src/shapes/Cylinder.cpp src/shapes/Cylinder.h src/shapes/Sphere.cpp src/shapes/Sphere.h   src/shapes/Triangle.cpp src/shapes/Triangle.h
    src/texture.cpp src/texture.h
    src/shapes/shape.h
    src/utils/rgba.h
    src/utils/shaderloader.h
    src/utils/utils.h
    src/gl_kernel.h
    src/extern/tiny_obj_loader.h
    src/vulkan-device.cpp
    src/vulkan-device.hpp
    src/hyacinth-labyrinth.hpp src/vulkan-pipeline.hpp src/glfw-window.hpp
    src/hyacinth-labyrinth.cpp src/vulkan-pipeline.cpp src/glfw-window.cpp
    src/vulkan-buffer.hpp src/vulkan-descriptors.hpp src/vulkan-renderer.hpp
    src/vulkan-buffer.cpp src/vulkan-descriptors.cpp src/vulkan-renderer.cpp
    src/vulkan-swapchain.hpp
    src/vulkan-swapchain.cpp
    src/vulkan-model.cpp
    src/vulkan-model.hpp

)

# VULKAN: Tested on VulkanSDK v1.3.268.1
find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan headers at: ${Vulkan_INCLUDE_DIRS}")
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

# GLFW
find_package(glfw3 3.3 REQUIRED)
set(GLFW_LIB glfw)
if (NOT glfw3_FOUND)
    message(FATAL_ERROR "Could not find glfw library!")
else()
    message(STATUS "Using glfw at: ${glfw3_DIR}")
endif()

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c)
include_directories(${PROJECT_NAME} PRIVATE
    glew/include
    ${Vulkan_INCLUDE_DIRS}
    
)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
    ${Vulkan_LIBRARIES}
    glfw
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
    resources/shaders/shape.frag
    resources/shaders/shape.vert
    resources/shaders/invert.frag
    resources/shaders/invert.vert
    resources/shaders/grayscale.frag
    resources/shaders/grayscale.vert
    resources/shaders/blur.frag
    resources/shaders/blur.vert
    resources/shaders/sobel.vert
    resources/shaders/sobel.frag
)


# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
