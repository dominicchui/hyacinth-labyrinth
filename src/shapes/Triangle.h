#pragma once

#include "utils/scenedata.h"
#include "texture.h"
#include "shape.h"

#include <vector>
#include <glm/glm.hpp>

class TriangleMesh : public Shape {
    SceneMaterial m_material;
    glm::mat4 m_ctm;
    glm::mat4 m_inv_ctm;
    glm::mat3 m_norm_trans;

    Texture* tex;

public:
    TriangleMesh(
        const std::string& mesh_file,
        const SceneMaterial& smat,
        const glm::mat4 ctm,
        Texture* tex
    );

    ~TriangleMesh(void) {}
    void updateParams(int param1, int param2) override {/* Not applicable */}
    std::vector<float> getData() {return m_vertexData;}

    void copyShapeIntoBuffer(std::vector<float>& buffer) override;
    const SceneMaterial& getMaterial() const override;
    const glm::mat4* getModelMat() const override;
    const glm::mat3* getNormMat() const override;

private:
    void readMesh(const std::string& filename);

    std::vector<float> m_vertexData;
};
