#pragma once

#include "utils/scenedata.h"
#include "texture.h"
#include "shape.h"

#include <vector>
#include <glm/glm.hpp>

class Cylinder : public Shape {
    SceneMaterial m_material;
    glm::mat4 m_ctm;
    glm::mat4 m_inv_ctm;
    glm::mat3 m_norm_trans;

    int32_t rep_U, rep_V;  // The number of times textures repeat in each dimension
    Texture* tex;

public:
    Cylinder(
        const SceneMaterial& smat,
        const glm::mat4 ctm,
        Texture* tex,
        std::pair<int32_t, int32_t> max_tess_params
    );
    void updateParams(int param1, int param2) override;
    std::vector<float> generateShape() { return m_vertexData; }

    void copyShapeIntoBuffer(std::vector<float>& buffer) override;
    const SceneMaterial& getMaterial() const override;
    const glm::mat4* getModelMat() const override;
    const glm::mat3* getNormMat() const override;

private:

    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeCapsTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight,
                      glm::vec3 norm);
    void makeTri(glm::vec3 a,
                 glm::vec3 b,
                 glm::vec3 c,
                 glm::vec3 norm);
    void makeSlice(float currentTheta,
                   float nextTheta,
                   float top_y,
                   float bot_y);

    void makeCaps(float currentTheta,
                  float nextTheta,
                  int32_t param1);

    void makeCylinder(void);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
};
