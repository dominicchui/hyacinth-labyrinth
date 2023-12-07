#include "Cylinder.h"

Cylinder::Cylinder(
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
    makeCylinder();
}

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    makeCylinder();
}

void Cylinder::makeTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight) {
    // Create a tile (i.e. 2 triangles) based on 4 given points.
    // The normals will be calculated based on a perfect cylinder

    glm::vec3 br_norm = glm::normalize(glm::vec3(bottomRight.x, 0, bottomRight.z));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);

    glm::vec3 tr_norm = glm::normalize(glm::vec3(topRight.x, 0, topRight.z));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, tr_norm);

    glm::vec3 tl_norm = glm::normalize(glm::vec3(topLeft.x, 0, topLeft.z));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);

    glm::vec3 bl_norm = glm::normalize(glm::vec3(bottomLeft.x, 0, bottomLeft.z));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bl_norm);
}

void Cylinder::makeCapsTile(glm::vec3 topLeft,
                            glm::vec3 topRight,
                            glm::vec3 bottomLeft,
                            glm::vec3 bottomRight,
                            glm::vec3 norm) {
    // Create a tile (i.e. 2 triangles) based on 4 given points.
    // The normals will be calculated based on a perfect cylinder

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, norm);
}

void Cylinder::makeTri(glm::vec3 a,
                       glm::vec3 b,
                       glm::vec3 c,
                       glm::vec3 norm) {
    insertVec3(m_vertexData, a);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, b);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, c);
    insertVec3(m_vertexData, norm);

}

void Cylinder::makeCaps(
    float currentTheta,
    float nextTheta,
    int32_t param1
) {
    // Create both the top and bottom caps
    // Start with the very center and divide it up:
    // param1 divides the radius, param2 divides the angle

    static constexpr glm::vec3 top(0.f,  0.5f, 0.f);
    static constexpr glm::vec3 bot(0.f, -0.5f, 0.f);
    static constexpr glm::vec3 norm_top(0.f,  1.f, 0.f);
    static constexpr glm::vec3 norm_bot(0.f, -1.f, 0.f);

    float edge_len = 0.5f / param1;

    // Special case: very center.
    float x_bl = edge_len * glm::sin(currentTheta);
    float x_br = edge_len * glm::sin(nextTheta);
    float z_bl = edge_len * glm::cos(currentTheta);
    float z_br = edge_len * glm::cos(nextTheta);

    glm::vec3 bl_top(x_bl, top.y, z_bl);
    glm::vec3 br_top(x_br, top.y, z_br);
    glm::vec3 bl_bot(x_bl, bot.y, z_bl);
    glm::vec3 br_bot(x_br, bot.y, z_br);

    makeTri(top, bl_top, br_top, norm_top);
    makeTri(bot, br_bot, bl_bot, norm_bot);

    // The rest:
    float x_tl = x_bl;
    float x_tr = x_br;
    float z_tl = z_bl;
    float z_tr = z_br;

    for (int32_t i = 1; i < param1; i++) {
        x_bl = (i+1) * edge_len * glm::sin(currentTheta);
        x_br = (i+1) * edge_len * glm::sin(nextTheta);
        z_bl = (i+1) * edge_len * glm::cos(currentTheta);
        z_br = (i+1) * edge_len * glm::cos(nextTheta);

        glm::vec3 bl_top(x_bl, top.y, z_bl);
        glm::vec3 br_top(x_br, top.y, z_br);
        glm::vec3 tl_top(x_tl, top.y, z_tl);
        glm::vec3 tr_top(x_tr, top.y, z_tr);

        glm::vec3 bl_bot(x_bl, bot.y, z_bl);
        glm::vec3 br_bot(x_br, bot.y, z_br);
        glm::vec3 tl_bot(x_tl, bot.y, z_tl);
        glm::vec3 tr_bot(x_tr, bot.y, z_tr);

        makeCapsTile(tl_top, tr_top, bl_top, br_top, norm_top);
        makeCapsTile(bl_bot, br_bot, tl_bot, tr_bot, norm_bot);

        x_tl = x_bl;
        x_tr = x_br;
        z_tl = z_bl;
        z_tr = z_br;
    }
}

void Cylinder::makeCylinder(void) {
    // Factored out stuff
    static constexpr float top_y = 0.5f;
    static constexpr float bot_y = -0.5f;

    glm::vec3 top = glm::vec4(0.f, top_y, 0.f, 1.f);
    glm::vec3 bot = glm::vec4(0.f, bot_y, 0.f, 1.f);

    glm::vec3 top_normal = glm::vec3(0.f,  1.f, 0.f);
    glm::vec3 bot_normal = glm::vec3(0.f, -1.f, 0.f);

    // Generate n_lod different cylinders
    // The current algorithm is brute-force-ish...
    // simply regenerates all the vertices for every level
    // For cylinder, the minimum params are (1, 3)

    float param1_step = float(m_param1 - 1) / n_lod;
    float param2_step = float(m_param2 - 3) / n_lod;

    int32_t max_param1 = std::max(1, m_param1);
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
            // Make a slice that contains top, side, and bottom pieces
            float currentTheta = i * thetaStep;
            float nextTheta = (i+1) * thetaStep;

            float half_sin_cur_theta = 0.5f * glm::sin(currentTheta);
            float half_cos_cur_theta = 0.5f * glm::cos(currentTheta);
            float half_sin_nex_theta = 0.5f * glm::sin(nextTheta);
            float half_cos_nex_theta = 0.5f * glm::cos(nextTheta);

            float x_tl = half_sin_cur_theta;
            float z_tl = half_cos_cur_theta;

            float x_tr = half_sin_nex_theta;
            float z_tr = half_cos_nex_theta;

            float x_bl = half_sin_cur_theta;
            float z_bl = half_cos_cur_theta;

            float x_br = half_sin_nex_theta;
            float z_br = half_cos_nex_theta;

            float height = top_y - bot_y;
            float height_segment = height / cur_param1;

            float y0;
            float y1 = top_y;

            // Face
            for (int32_t i = 0; i < cur_param1; i++) {
                y0 = y1;
                y1 = top_y - (i+1)*height_segment;

                glm::vec3 tl = glm::vec3(x_tl, y0, z_tl);
                glm::vec3 tr = glm::vec3(x_tr, y0, z_tr);
                glm::vec3 bl = glm::vec3(x_bl, y1, z_bl);
                glm::vec3 br = glm::vec3(x_br, y1, z_br);

                makeTile(tl, tr, bl, br);
            }
            // Caps
            makeCaps(currentTheta, nextTheta, cur_param1);
        }

        // Book keeping for LoD stuff
        lod_shape_start[lod] = cur_vertex_data_start;
        cur_vertex_data_size = m_vertexData.size() - cur_vertex_data_start;
        cur_vertex_data_start = m_vertexData.size();
        lod_shape_size[lod] = cur_vertex_data_size;
    }
}

void Cylinder::copyShapeIntoBuffer(std::vector<float>& buffer) {
    abs_shape_start = buffer.size();

    buffer.insert(buffer.end(), m_vertexData.begin(), m_vertexData.end());
}

const SceneMaterial& Cylinder::getMaterial() const {
    return m_material;
}

const glm::mat4* Cylinder::getModelMat() const {
    return &m_ctm;
}
const glm::mat3* Cylinder::getNormMat() const {
    return &m_norm_trans;
}
