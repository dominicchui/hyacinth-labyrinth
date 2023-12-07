#include "Sphere.h"

Sphere::Sphere(
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
    m_param1(max_tess_params.first),
    m_param2(max_tess_params.second)
{
    setBoundingCoords(ctm);
    makeSphere();
}

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    makeSphere();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!
    glm::vec3 br_norm = glm::normalize(bottomRight);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);

    glm::vec3 tr_norm = glm::normalize(topRight);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, tr_norm);

    glm::vec3 tl_norm = glm::normalize(topLeft);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);

    glm::vec3 bl_norm = glm::normalize(bottomLeft);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bl_norm);
}

void Sphere::makeWedge(float currentTheta, float nextTheta, int32_t param1, int32_t param2) {
    // Create a single wedge of the sphere using the
    // makeTile() function
    // Note: think about how param 1 comes into play here!

    float half_sin_cur_theta = 0.5f * glm::sin(currentTheta);
    float half_cos_cur_theta = 0.5f * glm::cos(currentTheta);
    float half_sin_nex_theta = 0.5f * glm::sin(nextTheta);
    float half_cos_nex_theta = 0.5f * glm::cos(nextTheta);

    float phi_step = M_PI / float(param1);
    for (int32_t i = 0; i < param1; i++) {
        float phi_curr = float(i) * phi_step;
        float phi_next = float(i+1) * phi_step;

        float top_y = 0.5 * glm::cos(phi_curr);
        float bot_y = 0.5 * glm::cos(phi_next);

        float x_tl = half_sin_cur_theta * glm::sin(phi_curr);
        float z_tl = half_cos_cur_theta * glm::sin(phi_curr);

        float x_tr = half_sin_nex_theta * glm::sin(phi_curr);
        float z_tr = half_cos_nex_theta * glm::sin(phi_curr);

        float x_bl = half_sin_cur_theta * glm::sin(phi_next);
        float z_bl = half_cos_cur_theta * glm::sin(phi_next);

        float x_br = half_sin_nex_theta * glm::sin(phi_next);
        float z_br = half_cos_nex_theta * glm::sin(phi_next);


        glm::vec3 tl = glm::vec4(x_tl, top_y, z_tl, 1.f);
        glm::vec3 tr = glm::vec4(x_tr, top_y, z_tr, 1.f);
        glm::vec3 bl = glm::vec4(x_bl, bot_y, z_bl, 1.f);
        glm::vec3 br = glm::vec4(x_br, bot_y, z_br, 1.f);

        makeTile(tl, tr, bl, br);
    }
}

void Sphere::makeSphere() {
    // Generate n_lod different spheres
    // The current algorithm is brute-force-ish...
    // simply regenerates all the vertices for every level
    // For sphere, the minimum params are (2, 3)

    float param1_step = float(m_param1 - 2) / n_lod;
    float param2_step = float(m_param2 - 3) / n_lod;

    int32_t max_param1 = std::max(2, m_param1);
    int32_t max_param2 = std::max(3, m_param2);

    int32_t cur_param1;
    int32_t cur_param2;

    size_t cur_vertex_data_start = 0U;
    size_t cur_vertex_data_size  = 0U;

    for (int32_t lod = 0; lod < n_lod; lod++) {
        cur_param1 = std::ceil(float(max_param1) - (lod * param1_step));
        cur_param2 = std::ceil(float(max_param2) - (lod * param2_step));

        // By design, cur_param* shouldn't go below the minimum

        float thetaStep = glm::radians(360.f / cur_param2);
        for (int32_t i = 0; i < cur_param2; i++) {
            float currentTheta = i * thetaStep;
            float nextTheta = (i+1) * thetaStep;
            makeWedge(currentTheta, nextTheta, cur_param1, cur_param2);
        }

        // Book keeping for LoD stuff
        lod_shape_start[lod] = cur_vertex_data_start;
        cur_vertex_data_size = m_vertexData.size() - cur_vertex_data_start;
        cur_vertex_data_start = m_vertexData.size();
        lod_shape_size[lod] = cur_vertex_data_size;
    }
}

void Sphere::copyShapeIntoBuffer(std::vector<float>& buffer) {
    abs_shape_start = buffer.size();
    buffer.insert(buffer.end(), m_vertexData.begin(), m_vertexData.end());
}

const SceneMaterial& Sphere::getMaterial() const {
    return m_material;
}

const glm::mat4* Sphere::getModelMat() const {
    return &m_ctm;
}
const glm::mat3* Sphere::getNormMat() const {
    return &m_norm_trans;
}
