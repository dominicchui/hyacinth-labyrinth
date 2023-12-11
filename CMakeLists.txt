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
  # src
  src/main.cpp
#  src/debug.h
  src/gl_kernel.h
  src/hyacinth-labyrinth.hpp                  src/hyacinth-labyrinth.cpp

  # external files
  src/extern/tiny_obj_loader.h

  # game
  src/game/keyboard_movement_controller.hpp   src/game/keyboard_movement_controller.cpp
  src/game/lve_game_object.hpp                src/game/lve_game_object.cpp
  src/game/lve_camera.hpp                     src/game/lve_camera.cpp
  src/game/maze.h

  # realtime renderer
  src/renderer/camera.h                       src/renderer/camera.cpp
  src/renderer/realtime.h                     src/renderer/realtime.cpp

  # scene
  src/scene/scene.h                           src/scene/scene.cpp
  src/scene/scenedata.h
  src/scene/scenefilereader.h                 src/scene/scenefilereader.cpp
  src/scene/sceneparser.h                     src/scene/sceneparser.cpp
  src/scene/shaderloader.h

  src/scene/objloader.h                       src/scene/objloader.cpp

  # shapes / objects
                                              src/shapes/cone.cpp
                                              src/shapes/cube.cpp
                                              src/shapes/cylinder.cpp
                                              src/shapes/mesh.cpp
  src/shapes/shapes.h                         src/shapes/shapes.cpp
                                              src/shapes/sphere.cpp

  # systems (? faisal give a better name pls)
  src/systems/point_light_system.hpp          src/systems/point_light_system.cpp
  src/systems/simple_render_system.hpp        src/systems/simple_render_system.cpp

  # utils
  src/utils/settings.h                        src/utils/settings.cpp
  src/utils/texture.h                         src/utils/texture.cpp
  src/utils/timer.h

  # vulkan files
  src/vulkan/vulkan-buffer.hpp                src/vulkan/vulkan-buffer.cpp
  src/vulkan/vulkan-descriptors.hpp           src/vulkan/vulkan-descriptors.cpp
  src/vulkan/vulkan-device.hpp                src/vulkan/vulkan-device.cpp
  src/vulkan/vulkan-frame-info.hpp
  src/vulkan/vulkan-model.hpp                 src/vulkan/vulkan-model.cpp
  src/vulkan/vulkan-pipeline.hpp              src/vulkan/vulkan-pipeline.cpp
  src/vulkan/vulkan-renderer.hpp              src/vulkan/vulkan-renderer.cpp
  src/vulkan/vulkan-swapchain.hpp             src/vulkan/vulkan-swapchain.cpp

  # windowing
  src/window/aspectratiowidget.hpp
  src/window/glfw-window.hpp                  src/window/glfw-window.cpp
  src/window/mainwindow.h                     src/window/mainwindow.cpp
  src/utils/utils.h
  src/utils/debug.h

  # maze
  src/maze/maze.h
  src/maze/maze.cpp
  src/maze/mazeblock.h
  src/maze/mazeblock.cpp
  src/maze/cell.h
  src/maze/cell.cpp

)
add_executable(MazeTest
    src/maze/maze.h
    src/maze/maze.cpp
    src/maze/mazeblock.h
    src/maze/mazeblock.cpp
    src/maze/cell.h
    src/maze/cell.cpp
    src/maze/mazetest.cpp
)

# VULKAN: Tested on VulkanSDK v1.3.268.1
#find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
#if (NOT Vulkan_FOUND)
#    message(FATAL_ERROR "Could not find Vulkan library!")
#else()
#    message(STATUS "Using vulkan headers at: ${Vulkan_INCLUDE_DIRS}")
#    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
#endif()

set(Vulkan_LIBRARIES "/usr/local/lib")
set(Vulkan_INCLUDE_DIRS "/usr/local/include")

# GLFW
if (WIN32)
  set(GLFW_PATH "C:/Program Files/glfw-3.3.8")
  set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
  set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")
else ()
  find_package(glfw3 3.3 REQUIRED)
  set(GLFW_LIB glfw)
  if (NOT glfw3_FOUND)
      message(FATAL_ERROR "Could not find glfw library! ${GLFW_LIB}")
  else()
      message(STATUS "Using glfw at: ${glfw3_DIR}")
  endif()
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
    ${Vulkan_LIBRARIES}/libvulkan.1.dylib
)
if (WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw3)
else ()
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif ()

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES

    resources/models/bunny.obj
    resources/models/colored_cube.obj
    resources/models/cube.obj
    resources/models/dragon.obj
    resources/models/flat_vase.obj
    resources/models/quad.obj
    resources/models/smooth_vase.obj

    resources/proj6_shaders/phong.frag
    resources/proj6_shaders/phong.vert
    resources/proj6_shaders/texture.frag
    resources/proj6_shaders/texture.vert

    resources/shaders/point_light.frag
    resources/shaders/point_light.vert
    resources/shaders/simple_shader.frag
    resources/shaders/simple_shader.vert
)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )

  target_include_directories(${PROJECT_NAME} PUBLIC
    ${GLFW_INCLUDE_DIRS}
  )

  target_link_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    ${GLFW_LIB}
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

###### Build Sahders
# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
  /usr/bin
  /usr/local/bin
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/
  $ENV{VULKAN_SDK}/Bin32/
)

set (SHADER_DIR ${PROJECT_SOURCE_DIR}/resources/shaders)
# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${SHADER_DIR}/*.frag"
  "${SHADER_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${SHADER_DIR}/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)
