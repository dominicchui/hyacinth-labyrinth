#ifndef SHAPE_H
#define SHAPE_H

#include <glm/glm.hpp>
#include <vector>
#include "utils/scenedata.h"
#include "utils/utils.h"

typedef std::pair<glm::vec4, glm::vec4> BoundingCoords;

// Convenience function
static BoundingCoords getBoundingCubeForUnitObjects(const glm::mat4& ctm) {
    static constexpr glm::vec4 corners[] = {
        glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),
        glm::vec4(-0.5f, -0.5f,  0.5f, 1.0f),
        glm::vec4(-0.5f,  0.5f, -0.5f, 1.0f),
        glm::vec4(-0.5f,  0.5f,  0.5f, 1.0f),
        glm::vec4(0.5f,  -0.5f, -0.5f, 1.0f),
        glm::vec4(0.5f,  -0.5f,  0.5f, 1.0f),
        glm::vec4(0.5f,   0.5f, -0.5f, 1.0f),
        glm::vec4(0.5f,   0.5f,  0.5f, 1.0f)
    };

    glm::vec4 lo = inf_pt;
    glm::vec4 hi = neg_inf_pt;

    for (auto corner : corners) {
        glm::vec4 transformed_corner = ctm * corner;
        lo = glm::min(lo, transformed_corner);
        hi = glm::max(hi, transformed_corner);
    }
    return  BoundingCoords{lo, hi};
}

class Shape {
public:
    // Shape common constants
    static constexpr int32_t n_lod = 4;

protected:
    // The relative index at which each lod-level starts
    size_t lod_shape_start[n_lod] = {0};

    // The size (in total array elements) of each lod-level
    size_t lod_shape_size[n_lod] = {0};

    // The absolute index (in the overall buffer) at which a shape starts
    size_t abs_shape_start = 0;
public:
    virtual ~Shape() {}
    std::vector<float> getData() { return m_vertexData; }

    virtual void insertVec3(std::vector<float>& buffer, glm::vec3 v) {
        buffer.push_back(v.x);
        buffer.push_back(v.y);
        buffer.push_back(v.z);
    }

    virtual void updateParams(int param1, int param2) = 0;
    virtual void copyShapeIntoBuffer(std::vector<float>& buffer) = 0;
    virtual const SceneMaterial& getMaterial() const = 0;
    virtual const glm::mat4* getModelMat() const = 0;
    virtual const glm::mat3* getNormMat()  const = 0;

    std::vector<float> m_vertexData;
    float m_radius = 0.5;

    // LoD stuff
    size_t getNumVertices(int32_t lod) const {
        return lod_shape_size[lod] / 2;
    }
    size_t getRelShapeStart(int32_t lod) const {
        return lod_shape_start[lod];
    }
    size_t getAbsShapeStart(void) const {
        return abs_shape_start;
    }

    // bounding coordinates and centroid (for LoD)
    BoundingCoords b_coords;
    glm::vec3 centroid;
    void setBoundingCoords(const glm::mat4& ctm) {
        b_coords = getBoundingCubeForUnitObjects(ctm);
        centroid = (b_coords.second + b_coords.first) / 2.f;
    }
};

#endif // SHAPE_H
