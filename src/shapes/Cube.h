#pragma once

#include "utils/scenedata.h"
#include "texture.h"
#include "shape.h"

#include <vector>
#include <glm/glm.hpp>

// TODO remove
class Texture;

class Cube : public Shape {
    SceneMaterial m_material;
    glm::mat4 m_ctm;
    glm::mat4 m_inv_ctm;
    glm::mat3 m_norm_trans;

    int32_t rep_U, rep_V;  // The number of times textures repeat in each dimension
    Texture* tex;

public:
    Cube(
        const SceneMaterial& smat,
        const glm::mat4 ctm,
        Texture* tex,
        std::pair<int32_t, int32_t> max_tess_params
    );

    ~Cube(void) {}
    void updateParams(int param1, int param2) override;
    std::vector<float> getData() { return m_vertexData; }

    void copyShapeIntoBuffer(std::vector<float>& buffer) override;
    const SceneMaterial& getMaterial() const override;
    const glm::mat4* getModelMat() const override;
    const glm::mat3* getNormMat() const override;

private:
    void makeCube(void);
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeFace(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight,
                  int32_t param1);

    std::vector<float> m_vertexData;
    int m_param1;
};
