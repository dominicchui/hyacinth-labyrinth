#pragma once

#include "renderer/camera.h"
#include "shapes/shapes.h"
#include "scene/scenedata.h"

// Container for a renderable scene
class Scene {
private:
    RenderData data;
    Camera *cam;
    std::vector<Shape *> shapes;
    GLuint vao;
    GLuint vbo;
    std::vector<GLfloat> vertexData = {};

public:
    // Creates a scene object.
    Scene(Camera *camera);
    ~Scene();

    // Initialized the data for the scene. Assumes GL has been initialized
    void init();

    // Loads a scenefile, using the global settings object. Initializes the
    // data associated with the scene
    void loadScene();
    bool loaded = false;
    bool manual_add = false;

    // Updates the loaded data based on the global settings object
    void update();

    // Draws the scene using the given shader
    void draw(GLuint shader);

    void add(Shape *s) {
        shapes.push_back(s);
        manual_add = true;
        SceneCameraData c {
            glm::vec4(5,5,0,1),
            glm::vec4(-1,-1,0,0),
            glm::vec4(0,1,0,0),
            M_PI / 2.f,
            0.f,
            0.f
        };
        cam->init(c);

        data.lights = {SceneLightData{0, LightType::LIGHT_DIRECTIONAL, {1,1,1,1}, glm::vec3(0), glm::vec4(0), {0,-1,0,0}}};
        data.globalData = {0.5,0.5,0.5,1};

        update();

    }
};
