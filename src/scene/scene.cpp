#include "scene.h"

#include "utils/settings.h"

Scene::Scene(Camera *camera) {
    cam = camera;
}

Scene::~Scene() {
    // delete all shapes
    for (Shape *shape : shapes) delete shape;
    delete cam;
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Scene::init() {
    // Generate and bing VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Generate and bind a VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Add position and normal attributes to VAO
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Scene::loadScene() {
    bool success = SceneParser::parse(settings.sceneFilePath, data);
    if (!success) {
        std::cerr
            << "Error loading scene: \""
            << settings.sceneFilePath << "\""
            << std::endl;
        loaded = false;
        return;
    }

    shapes.clear();
    // Generate shapes
    transform(
        data.shapes.begin(), data.shapes.end(),
        back_inserter(shapes),
        getShape);

    //cam->init(data.cameraData);
    loaded = true;

    update();
}

void Scene::update() {
    if (!loaded and !manual_add) return;

    // Clear data and re-generate verticies for each object
    vertexData.clear();
    int offset = 0;
    for (Shape *shape : shapes) {
        int numNewVertex = shape->updateData(offset, vertexData);
        offset += numNewVertex;
    }

    // Setup VBO with the new data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //cam->update();
}

void setLightData(GLuint shader, std::vector<SceneLightData> lights) {
    int numLights = fmin(lights.size(), 8);
    glUniform1i(glGetUniformLocation(shader, "numLights"), numLights);

    for (int i = 0; i < numLights; i++) {
        SceneLightData light = lights[i];
        auto base = "lights[" + std::to_string(i) + "].";

        auto type = base + "type";
        glUniform1i(glGetUniformLocation(shader, type.c_str()),
                    static_cast<int>(light.type));

        auto color = base + "color";
        glUniform4f(glGetUniformLocation(shader, color.c_str()),
                    light.color[0], light.color[1], light.color[2], light.color[3]);

        auto dir = base + "dir";;
        glUniform3f(glGetUniformLocation(shader, dir.c_str()),
                    light.dir[0], light.dir[1], light.dir[2]);

        auto pos = base + "pos";;
        glUniform3f(glGetUniformLocation(shader, pos.c_str()),
                    light.pos[0], light.pos[1], light.pos[2]);

        auto atten = base + "atten";;
        glUniform3f(glGetUniformLocation(shader, atten.c_str()),
                    light.function[0], light.function[1], light.function[2]);

        auto penumbra = base + "penumbra";;
        glUniform1f(glGetUniformLocation(shader, penumbra.c_str()), light.penumbra);

        auto angle = base + "angle";;
        glUniform1f(glGetUniformLocation(shader, angle.c_str()), light.angle);
    }
}

void Scene::draw(GLuint shader) {
    if (!loaded and !manual_add) return;
    // Bind the shader and VAO
    glUseProgram(shader);
    glBindVertexArray(vao);

    // Set view matrix
    // glUniformMatrix4fv(
    //     glGetUniformLocation(shader, "view"),
    //     1, GL_FALSE, &cam->getView()[0][0]);

    // // Set projection matrix
    // glUniformMatrix4fv(
    //     glGetUniformLocation(shader, "proj"),
    //     1, GL_FALSE, &cam->getProj()[0][0]);

    // // Set camera position
    // glm::vec3 camPos = cam->getPos();
    // glUniform3f(glGetUniformLocation(shader, "camPos"),
    //             camPos[0], camPos[1], camPos[2]);

    // Set light data
    setLightData(shader, data.lights);

    // Set global lighting terms
    glUniform1f(glGetUniformLocation(shader, "ka"), data.globalData.ka);
    glUniform1f(glGetUniformLocation(shader, "kd"), data.globalData.kd);
    glUniform1f(glGetUniformLocation(shader, "ks"), data.globalData.ks);

    // Draw all shapes in the scene
    for (Shape *shape : shapes) shape->draw(shader);

    // Unbind VAO and shader
    glBindVertexArray(0);
    glUseProgram(0);
}
