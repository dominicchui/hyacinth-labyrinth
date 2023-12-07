#pragma once

#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include "shapes/shape.h"
#include "camera/camera.h"
#include "texture.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>

#include <vector>
#include <unordered_map>
#include <set>


// A class representing a scene to be rasterized

class RasterScene
{
private:
    // A vector of all primitives
    std::vector<Shape*> m_primitives;
    // A map between filenames and texture objects.
    std::unordered_map<std::string, Texture*> tex_file_map;

    int32_t m_width, m_height;

    SceneGlobalData sgd;    // Not to be confused with stochastic gradient descent.

    std::vector<float> obj_buffer;
    std::vector<SceneLightData> lights;

    GLuint m_shader;
    GLuint m_vbo;
    GLuint m_vao;

    // Tesselation params
    std::pair<int32_t, int32_t> m_tess_params;

public:
    static constexpr glm::vec4 bg_clr{0,0,0,1};

    /////// METHODS ////////

    RasterScene(
        int width,
        int height,
        const RenderData &metaData,
        GLuint shader_id,
        GLuint vbo_id,
        GLuint vao_id,
        std::pair<int32_t, int32_t> tess_params
    );
    ~RasterScene(void);

    // Get a reference to the vector of primitives
//    const std::vector<const Shape*>& getPrimitives(void) const {
//        return m_primitives;
//    }
    void draw(const Camera& cam) const;

    std::vector<float>* getObjBuffer(void);

    void getDataAndShipItToGPU();

    // The getter of the width of the scene
    const int& width() const;

    // The getter of the height of the scene
    const int& height() const;

    // The getter of the global data of the scene
    const SceneGlobalData& getGlobalData() const;

    // Getter for the light vector.
    const std::vector<SceneLightData>& getLights() const;

    // Updates tesselation params
    void updateTessParams(std::pair<int32_t, int32_t>& tess_params);
};
