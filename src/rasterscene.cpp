#include "rasterscene.h"
#include "utils/sceneparser.h"
#include "utils/scenedata.h"
#include "shapes/shape.h"
#include "shapes/Cone.h"
#include "shapes/Cube.h"
#include "shapes/Cylinder.h"
#include "shapes/Sphere.h"
#include "shapes/Triangle.h"
#include "utils/timer.h"
#include "texture.h"
#include "utils/debug.h"
#include "camera/camera.h"

#include <stdexcept>
#include <string>
#include <iostream>

RasterScene::RasterScene(
    int width,
    int height,
    const RenderData &metaData,
    GLuint shader_id,
    GLuint vbo_id,
    GLuint vao_id,
    std::pair<int32_t, int32_t> tess_params
) :
    m_primitives(),
    tex_file_map({}),
    m_width(width),
    m_height(height),
    sgd(metaData.globalData),
    obj_buffer(),
    lights(metaData.lights),
    m_shader(shader_id),
    m_vbo(vbo_id),
    m_vao(vao_id),
    m_tess_params(tess_params)
{
    mytimer t;

    // Loop through the primitives
    m_primitives.reserve(2000);  // TODO: come up with a better rubric
    obj_buffer.reserve(6000);

    for (const RenderShapeData& shape : metaData.shapes) {
        // See if the shape has a texture
        const SceneFileMap& tex_file_info = shape.primitive.material.textureMap;
        Texture* tex = nullptr;

//      For Project 6
//        if (tex_file_info.isUsed) {
//            auto tex_iter = tex_file_map.find(tex_file_info.filename);
//            if (tex_iter == tex_file_map.end()) {
//                // Texture not found. Create it.
//                tex = new Texture(tex_file_info.filename);
//                tex_file_map[tex_file_info.filename] = tex;
//            } else {
//                // Texture was found.
//                tex = tex_iter->second;
//            }
//        }

        switch (shape.primitive.type) {
        case PrimitiveType::PRIMITIVE_CUBE:
            m_primitives.push_back(new Cube(shape.primitive.material, shape.ctm, tex, tess_params));
            break;
        case PrimitiveType::PRIMITIVE_CONE:
            m_primitives.push_back(new Cone(shape.primitive.material, shape.ctm, tex, tess_params));
            break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
            m_primitives.push_back(new Cylinder(shape.primitive.material, shape.ctm, tex, tess_params));
            break;
        case PrimitiveType::PRIMITIVE_SPHERE:
            m_primitives.push_back(new Sphere(shape.primitive.material, shape.ctm, tex, tess_params));
            break;
        case PrimitiveType::PRIMITIVE_MESH:
            m_primitives.push_back(new TriangleMesh(shape.primitive.meshfile, shape.primitive.material, shape.ctm, tex));
            break;
        default:
            std::runtime_error("Unknown primitive type: " +
                               std::to_string(int32_t(shape.primitive.type)));
        }
    }

    // Set up OpenGL VAOs

    // TODO move to a separate function
    glUseProgram(m_shader);

    // Fill VBO
    getDataAndShipItToGPU();

    // VAO stuff
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindVertexArray(m_vao);
    glErrorCheck();

    glEnableVertexAttribArray(0);
    glErrorCheck();

    static constexpr GLint sizeofTri = 3;
    static constexpr GLint strideTri = 6;
    GLint tri_pos_offset = 0;

    glVertexAttribPointer(0,
                          sizeofTri,
                          GL_FLOAT,
                          GL_FALSE,
                          strideTri * sizeof(GLfloat),
                          reinterpret_cast<void*>(tri_pos_offset));
    glErrorCheck();

    // Normal
    glEnableVertexAttribArray(1);
    glErrorCheck();

    GLint tri_norm_offset = tri_pos_offset + sizeofTri * sizeof(GLfloat);

    glVertexAttribPointer(1,
                          sizeofTri,
                          GL_FLOAT,
                          GL_FALSE,
                          strideTri * sizeof(GLfloat),
                          reinterpret_cast<void*>(tri_norm_offset));

    glErrorCheck();

    if (obj_buffer.size() < 10240) {
        verifyVAO(obj_buffer,
                  0,
                  sizeofTri,
                  strideTri * sizeof(GLfloat),
                  reinterpret_cast<void*>(tri_pos_offset));
        verifyVAO(obj_buffer,
                  1,
                  sizeofTri,
                  strideTri * sizeof(GLfloat),
                  reinterpret_cast<void*>(tri_norm_offset));
    }

    // Global scene data

    // Ambient
    GLint k_a_loc = myGlGetUniformLocation(m_shader, "k_a");
    glUniform1f(k_a_loc, sgd.ka);

    // Diffuse
    GLint k_d_loc = myGlGetUniformLocation(m_shader, "k_d");
    glUniform1f(k_d_loc, sgd.kd);

    // Specular
    GLint k_s_loc = myGlGetUniformLocation(m_shader, "k_s");
    glUniform1f(k_s_loc, sgd.ks);

    // Set up the lights
    assert(lights.size() < 8);
    int32_t light_i = 0;

    for (SceneLightData& light : lights) {
        const std::string pre   = "light_info[";
        std::string str_light_i = std::to_string(light_i++);
        const std::string post  = "].";

        const std::string base = pre + str_light_i + post;

        // Type
        std::string str_type = base + "type";
        GLint type_loc = myGlGetUniformLocation(m_shader, str_type.c_str());
        glUniform1ui(type_loc, GLuint(light.type));

        // Color
        std::string str_color = base + "color";
        GLint color_loc = myGlGetUniformLocation(m_shader, str_color.c_str());
        glUniform4fv(color_loc, 1, &light.color[0]);

        // Function
        std::string str_func = base + "function";
        GLint func_loc = myGlGetUniformLocation(m_shader, str_func.c_str());
        glUniform3fv(func_loc, 1, &light.function[0]);
        glErrorCheck();

        // Position
        std::string str_pos = base + "pos";
        GLint pos_loc = myGlGetUniformLocation(m_shader, str_pos.c_str());
        glUniform4fv(pos_loc, 1, &light.pos[0]);

        // Direction
        std::string str_dir = base + "dir";
        GLint dir_loc = myGlGetUniformLocation(m_shader, str_dir.c_str());
        glUniform4fv(dir_loc, 1, &light.dir[0]);

        // Penumbra
        std::string str_pen = base + "penumbra";
        GLint pen_loc = myGlGetUniformLocation(m_shader, str_pen.c_str());
        glUniform1f(pen_loc, light.penumbra);

        // Angle
        std::string str_angle = base + "angle";
        GLint angle_loc = myGlGetUniformLocation(m_shader, str_angle.c_str());
        glUniform1f(angle_loc, light.angle);
    }
    glErrorCheck();

    // Num lights
    GLint valid_lights_loc = myGlGetUniformLocation(m_shader, "valid_lights");
    glUniform1ui(valid_lights_loc, GLuint(light_i));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    t.checkpoint();
    std::cout << "Initialized " << m_primitives.size() << " shapes in " <<
        t.to_string() << std::endl;
    t.checkpoint();
}

RasterScene::~RasterScene(void) {
    for (auto primitive : m_primitives) {
        delete primitive;
    }
}

void RasterScene::draw(const Camera& cam) const {
    // Bind Vertex Data
    glBindVertexArray(m_vao);
    glErrorCheck();

    glUseProgram(m_shader);
    glErrorCheck();

    // TODO: Most of these uniforms shouldn't be set each draw call...

    glm::vec4 camera_world = cam.getPosition();
    GLint cam_pos_loc = myGlGetUniformLocation(m_shader, "cam_pos");
    // Deliberately casting vec4 to vec3
    glUniform3fv(cam_pos_loc, 1, &camera_world[0]);

    // for each extra 1000 objects in the scene, drop the LoD by 1
    int32_t lod_count_modifier = (m_primitives.size() / 1000);

    GLint start_idx = 0;
    for (const Shape* shape : m_primitives) {
        // Set model
        const glm::mat4* model = shape->getModelMat();
        GLint model_loc = myGlGetUniformLocation(m_shader, "model");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, &model[0][0][0]);

        const glm::mat3* norm_trans = shape->getNormMat();
        GLint norm_loc = myGlGetUniformLocation(m_shader, "norm_world");
        glUniformMatrix3fv(norm_loc, 1, GL_FALSE, &norm_trans[0][0][0]);

        // Set per-shape data
        const SceneMaterial& smat = shape->getMaterial();
        // Ambient
        GLint k_a_loc = myGlGetUniformLocation(m_shader, "obj_c_ambt");
        glUniform3f(k_a_loc, smat.cAmbient[0], smat.cAmbient[1], smat.cAmbient[2]);

        // Diffuse
        GLint k_d_loc = myGlGetUniformLocation(m_shader, "obj_c_dfus");
        glUniform3f(k_d_loc, smat.cDiffuse[0], smat.cDiffuse[1], smat.cDiffuse[2]);

        // Specular
        GLint k_s_loc = myGlGetUniformLocation(m_shader, "obj_c_spec");
        glUniform3f(k_s_loc, smat.cSpecular[0], smat.cSpecular[1], smat.cSpecular[2]);

        GLint spec_exp_loc = myGlGetUniformLocation(m_shader, "spec_exp");
        glUniform1f(spec_exp_loc, smat.shininess);
        glErrorCheck();

        // Calculate LoD real quick
        // TODO: incorporate scale info
        float dist_to_cam =
            glm::length(glm::vec3(camera_world) - shape->centroid);

        // Highly arbitrary rubric
        int32_t lod = int32_t(dist_to_cam / 10.f);
        lod += lod_count_modifier;
        lod = std::clamp(lod, 0, Shape::n_lod - 1);

        // Get start and size of data
        size_t obj_start      = shape->getAbsShapeStart();
        size_t lod_offset     = shape->getRelShapeStart(lod);
        size_t obj_n_vertices = shape->getNumVertices(lod);
        size_t abs_obj_start  = obj_start + lod_offset;

        // Draw Command
        glDrawArrays(GL_TRIANGLES, abs_obj_start / 6, obj_n_vertices / 3);

        glErrorCheck();
    }

    // Unbind Vertex Array
    glBindVertexArray(0);

    glUseProgram(0);
}

void RasterScene::getDataAndShipItToGPU(void) {
    // Generate the object buffer
    obj_buffer.clear();
    for (Shape* s : m_primitives) {
        s->copyShapeIntoBuffer(obj_buffer);
    }

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 obj_buffer.size() * sizeof(GLfloat),
                 obj_buffer.data(),
                 GL_STATIC_DRAW);
    glErrorCheck();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

std::vector<float>* RasterScene::getObjBuffer(void) {
    return &obj_buffer;
}

const int& RasterScene::width() const {
    return m_width;
}

const int& RasterScene::height() const {
    return m_height;
}

const SceneGlobalData& RasterScene::getGlobalData() const {
    return sgd;
}

const std::vector<SceneLightData>& RasterScene::getLights() const {
    return lights;
}

void RasterScene::updateTessParams(std::pair<int32_t, int32_t>& tess_params) {
    // Update all the shapes
    for (Shape* s : m_primitives) {
        s->updateParams(tess_params.first, tess_params.second);
    }
    getDataAndShipItToGPU();
}
