#include "Cone.h"

Cone::Cone(
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
    makeCone();
}

void Cone::makeTriTile(glm::vec3 top,
                       glm::vec3 bottomLeft,
                       glm::vec3 bottomRight,
                       glm::vec3 midway) {
    // Create a tile (i.e. 2 triangles) based on 4 given points.
    // The normals will be calculated based on a perfect cylinder

    glm::vec3 norm1(bottomRight.x * 2.f, -(bottomRight.y * 0.5f) + 0.25f, bottomRight.z * 2.f);
    glm::vec3 norm3(bottomLeft.x * 2.f, -(bottomLeft.y * 0.5f) + 0.25f, bottomLeft.z * 2.f);
    norm1 = glm::normalize(norm1);
    norm3 = glm::normalize(norm3);

    glm::vec3 norm2(midway.x * 2.f, -(midway.y * 0.5f) + 0.25f, midway.z * 2.f);

    norm2 = glm::normalize(norm2);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, norm1);

    //glm::vec3 norm2(top.x * 2.f, -(top.y * 0.5f) + 0.25f, top.z * 2.f);
    insertVec3(m_vertexData, top);
    insertVec3(m_vertexData, norm2);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, norm3);
}

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    makeCone();
}

void Cone::makeQuadTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight) {
    // Create a tile (i.e. 2 triangles) based on 4 given points.
    // The normals will be calculated based on a perfect cylinder

    glm::vec3 br_norm(bottomRight.x * 2.f, -(bottomRight.y * 0.5f) + 0.25f, bottomRight.z * 2.f);
    br_norm = normalize(br_norm);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);

    glm::vec3 tr_norm(topRight.x * 2.f, -(topRight.y * 0.5f) + 0.25f, topRight.z * 2.f);
    tr_norm = normalize(tr_norm);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, tr_norm);

    glm::vec3 tl_norm(topLeft.x * 2.f, -(topLeft.y * 0.5f) + 0.25f, topLeft.z * 2.f);
    tl_norm = normalize(tl_norm);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, br_norm);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, tl_norm);

    glm::vec3 bl_norm(bottomLeft.x * 2.f, -(bottomLeft.y * 0.5f) + 0.25f, bottomLeft.z * 2.f);
    bl_norm = normalize(bl_norm);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bl_norm);
}

void Cone::makeQuadTileBot(glm::vec3 topLeft,
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

void Cone::makeTri(glm::vec3 a,
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

void Cone::makeCap(
    float currentTheta,
    float nextTheta,
    int32_t param1
    ) {
    // Create bottom cap
    // Start with the very center and divide it up:
    // param1 divides the radius, param2 divides the angle

    static constexpr glm::vec3 bot(0.f, -0.5f, 0.f);
    static constexpr glm::vec3 norm_bot(0.f, -1.f, 0.f);

    float edge_len = 0.5f / param1;

    // Special case: very center.
    float x_bl = edge_len * glm::sin(currentTheta);
    float x_br = edge_len * glm::sin(nextTheta);
    float z_bl = edge_len * glm::cos(currentTheta);
    float z_br = edge_len * glm::cos(nextTheta);

    glm::vec3 bl_bot(x_bl, bot.y, z_bl);
    glm::vec3 br_bot(x_br, bot.y, z_br);

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

        glm::vec3 bl_bot(x_bl, bot.y, z_bl);
        glm::vec3 br_bot(x_br, bot.y, z_br);
        glm::vec3 tl_bot(x_tl, bot.y, z_tl);
        glm::vec3 tr_bot(x_tr, bot.y, z_tr);

        makeQuadTileBot(bl_bot, br_bot, tl_bot, tr_bot, norm_bot);

        x_tl = x_bl;
        x_tr = x_br;
        z_tl = z_bl;
        z_tr = z_br;
    }
}

void Cone::makeTip(float currentTheta,
                   float midwayTheta,
                   float nextTheta,
                   float top_y,
                   float bot_y,
                   int32_t param1) {
    float radius = 0.5f / param1;

    float x_bl = radius * glm::sin(currentTheta);
    float z_bl = radius * glm::cos(currentTheta);

    float x_m  = radius * glm::sin(midwayTheta);
    float z_m  = radius * glm::cos(midwayTheta);

    float x_br = radius * glm::sin(nextTheta);
    float z_br = radius * glm::cos(nextTheta);

    //    // Norm calculation
    //    glm::vec2 xz(obj_pos.x, obj_pos.z);

    //    float xz_len = glm::length(xz);
    //    ret.x = obj_pos.x;
    //    ret.y = xz_len / 2.f;
    //    ret.z = obj_pos.z;

    glm::vec3 top = glm::vec3(0.f,  top_y, 0.f);
    glm::vec3 bl  = glm::vec3(x_bl, bot_y, z_bl);
    glm::vec3 br  = glm::vec3(x_br, bot_y, z_br);
    glm::vec3 mid = glm::vec3(x_m, bot_y, z_m);

    makeTriTile(top, bl, br, mid);
}

void Cone::makeFace(float currentTheta,
                    float nextTheta,
                    float top_y,
                    float bot_y,
                    int32_t param1) {

    float seg_height = 1.f / param1;

    float sin_cur_theta = glm::sin(currentTheta);
    float cos_cur_theta = glm::cos(currentTheta);
    float sin_nex_theta = glm::sin(nextTheta);
    float cos_nex_theta = glm::cos(nextTheta);

    for (int32_t i = 0; i < param1 - 1; i++) {
        float seg_top = top_y - seg_height * i;
        float seg_bot = seg_top - seg_height;

        float rad_top = 0.5f * (1.f - (seg_top + 0.5f));
        float rad_bot = 0.5f * (1.f - (seg_bot + 0.5f));

        float x_bl = rad_bot * sin_cur_theta;
        float z_bl = rad_bot * cos_cur_theta;

        float x_br = rad_bot * sin_nex_theta;
        float z_br = rad_bot * cos_nex_theta;

        float x_tl = rad_top * sin_cur_theta;
        float z_tl = rad_top * cos_cur_theta;

        float x_tr = rad_top * sin_nex_theta;
        float z_tr = rad_top * cos_nex_theta;


        //    // Norm calculation
        //    glm::vec2 xz(obj_pos.x, obj_pos.z);

        //    float xz_len = glm::length(xz);
        //    ret.x = obj_pos.x;
        //    ret.y = xz_len / 2.f;
        //    ret.z = obj_pos.z;

        glm::vec3 bl = glm::vec3(x_bl, seg_bot, z_bl);
        glm::vec3 br = glm::vec3(x_br, seg_bot, z_br);
        glm::vec3 tl = glm::vec3(x_tl, seg_top, z_tl);
        glm::vec3 tr = glm::vec3(x_tr, seg_top, z_tr);

        makeQuadTile(tl, tr, bl, br);
    }
}

void Cone::makeCone(void) {
    // Generate n_lod different cone
    // The current algorithm is brute-force-ish...
    // simply regenerates all the vertices for every level
    // For cone, the minimum params are (1, 3)

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
            float currentTheta = i * thetaStep;
            float midwayTheta = (float(i) + 0.5f) * thetaStep;
            float nextTheta = (i+1) * thetaStep;

            // Tip
            float tip_height = 1.f / cur_param1;
            makeTip(currentTheta,
                    midwayTheta,
                    nextTheta,
                    0.5f,
                    0.5f - tip_height,
                    cur_param1);

            // Bottom cap
            makeCap(currentTheta, nextTheta, cur_param1);

            // Rest
            makeFace(currentTheta,
                     nextTheta,
                     0.5f - tip_height,
                     -0.5f,
                     cur_param1);
        }
        // Book keeping for LoD stuff
        lod_shape_start[lod] = cur_vertex_data_start;
        cur_vertex_data_size = m_vertexData.size() - cur_vertex_data_start;
        cur_vertex_data_start = m_vertexData.size();
        lod_shape_size[lod] = cur_vertex_data_size;
    }
}

void Cone::copyShapeIntoBuffer(std::vector<float>& buffer) {
    abs_shape_start = buffer.size();

    buffer.insert(buffer.end(), m_vertexData.begin(), m_vertexData.end());

}

const SceneMaterial& Cone::getMaterial() const {
    return m_material;
}

const glm::mat4* Cone::getModelMat() const {
    return &m_ctm;
}
const glm::mat3* Cone::getNormMat() const {
    return &m_norm_trans;
}
