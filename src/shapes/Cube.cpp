#include "Cube.h"

Cube::Cube(
    const SceneMaterial& smat,
    const glm::mat4 ctm,
    Texture* tex,
    std::pair<int32_t, int32_t> max_tess_params
) :
    m_material(smat),
    m_ctm(ctm),
    m_inv_ctm(glm::inverse(ctm)),
    m_norm_trans(glm::inverse(glm::transpose(glm::mat3(ctm)))),
    rep_U(smat.textureMap.repeatU),
    rep_V(smat.textureMap.repeatV),
    tex(tex),
    m_vertexData(),
    m_param1(max_tess_params.first)
{
    setBoundingCoords(ctm);
    makeCube();
}

void Cube::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    makeCube();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Draw a face: 2 triangles, counter clockwise

    glm::vec3 br_norm = glm::normalize(glm::cross(topRight - bottomRight, topLeft - bottomRight));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);
    //insertVec3(m_vertexData, m_material.cDiffuse);

    glm::vec3 tr_norm = glm::normalize(glm::cross(topLeft - topRight, bottomRight - topRight));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, tr_norm);
    //insertVec3(m_vertexData, m_material.cDiffuse);

    glm::vec3 tl_norm = glm::normalize(glm::cross(bottomRight - topLeft, topRight - topLeft));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);
    //insertVec3(m_vertexData, m_material.cDiffuse);


    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);
    //insertVec3(m_vertexData, m_material.cDiffuse);


    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);
    //insertVec3(m_vertexData, m_material.cDiffuse);


    glm::vec3 bl_norm = glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bl_norm);
    //insertVec3(m_vertexData, m_material.cDiffuse);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    int32_t param1) {

    glm::vec3 tile_width  = (topRight - topLeft) / float(param1);
    glm::vec3 tile_height = (bottomLeft - topLeft) / float(param1);

    for (int32_t i = 0; i < param1; i++) {
        for (int32_t j = 0; j < param1; j++) {
            glm::vec3 tl = topLeft + float(i) * tile_height + float(j) * tile_width;
            glm::vec3 tr = tl + tile_width;
            glm::vec3 bl = tl + tile_height;
            glm::vec3 br = tr + tile_height;
            makeTile(tl, tr, bl, br);
        }
    }
}

void Cube::makeCube(void) {
    // Draw cube
    static constexpr glm::vec4 obj_vertices[8] ={
        glm::vec4{-0.5f, -0.5f, -0.5f, 1.f},
        glm::vec4{-0.5f, -0.5f,  0.5f, 1.f},
        glm::vec4{ 0.5f, -0.5f,  0.5f, 1.f},
        glm::vec4{ 0.5f, -0.5f, -0.5f, 1.f},
        glm::vec4{-0.5f,  0.5f, -0.5f, 1.f},
        glm::vec4{-0.5f,  0.5f,  0.5f, 1.f},
        glm::vec4{ 0.5f,  0.5f,  0.5f, 1.f},
        glm::vec4{ 0.5f,  0.5f, -0.5f, 1.f}
    };

    // Generate n_lod different cube
    // The current algorithm is brute-force-ish...
    // simply regenerates all the vertices for every level
    // For cube, the minimum params are (1, x)

    float param1_step = float(m_param1 - 1) / n_lod;
    int32_t max_param1 = std::max(1, m_param1);
    int32_t cur_param1;

    size_t cur_vertex_data_start = 0U;
    size_t cur_vertex_data_size  = 0U;

    for (int32_t lod = 0; lod < n_lod; lod++) {
        cur_param1 = std::ceil(float(max_param1) - (lod * param1_step));

        // By design, cur_param1 shouldn't go below the minimum

        // Bottom
        makeFace(obj_vertices[1],
                 obj_vertices[2],
                 obj_vertices[0],
                 obj_vertices[3],
                 cur_param1);


        // Front
        makeFace(obj_vertices[5],
                 obj_vertices[6],
                 obj_vertices[1],
                 obj_vertices[2],
                 cur_param1);

        // Top
        makeFace(obj_vertices[4],
                 obj_vertices[7],
                 obj_vertices[5],
                 obj_vertices[6],
                 cur_param1);

        // Back
        makeFace(obj_vertices[0],
                 obj_vertices[3],
                 obj_vertices[4],
                 obj_vertices[7],
                 cur_param1);

        // Right
        makeFace(obj_vertices[6],
                 obj_vertices[7],
                 obj_vertices[2],
                 obj_vertices[3],
                 cur_param1);

        // Left
        makeFace(obj_vertices[1],
                 obj_vertices[0],
                 obj_vertices[5],
                 obj_vertices[4],
                 cur_param1);

        // Book keeping for LoD stuff
        lod_shape_start[lod] = cur_vertex_data_start;
        cur_vertex_data_size = m_vertexData.size() - cur_vertex_data_start;
        cur_vertex_data_start = m_vertexData.size();
        lod_shape_size[lod] = cur_vertex_data_size;
    }
}

void Cube::copyShapeIntoBuffer(std::vector<float>& buffer) {
    abs_shape_start = buffer.size();
    buffer.insert(buffer.end(), m_vertexData.begin(), m_vertexData.end());
}

const SceneMaterial& Cube::getMaterial() const {
    return m_material;
}

const glm::mat4* Cube::getModelMat() const {
    return &m_ctm;
}
const glm::mat3* Cube::getNormMat() const {
    return &m_norm_trans;
}
