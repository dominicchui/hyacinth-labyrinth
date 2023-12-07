#include <stdexcept>
#include <sstream>

#include "Triangle.h"

// #define TINYOBJLOADER_IMPLEMENTATION
// #include "extern/tiny_obj_loader.h"

TriangleMesh::TriangleMesh(
    const std::string& mesh_file,
    const SceneMaterial& smat,
    const glm::mat4 ctm,
    Texture* tex
) :
    m_material(smat),
    m_ctm(ctm),
    m_inv_ctm(glm::inverse(ctm)),
    m_norm_trans(glm::inverse(glm::transpose(glm::mat3(ctm)))),
    tex(tex),
    m_vertexData()
{
    setBoundingCoords(ctm);
    readMesh(mesh_file);
}

#include <fstream>

void TriangleMesh::readMesh(const std::string& mesh_file) {
    std::ifstream infile(mesh_file);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> faces;
    std::vector<glm::vec3> normals_in;

    std::string line;
    while (std::getline(infile, line))
    {
        if (line.starts_with('#')) {
            continue;
        }

        std::istringstream iss(line);
        char type;
        float x,y,z;

        if (!(iss >> type)) { break; } // Done

        // Process line
        if (type == 'v') {

            if (!(iss >> x >> y >> z)) {
                // error
                throw std::runtime_error("obj file v line invalid");
            }

//            std::cout << "vert = (" << x << ", " << y << ", " << z << ")\n";
            vertices.push_back(glm::vec3(x,y,z));
       // } else if (type == 'vn') {
        } else if (type == 'f') {
            if (!(iss >> x >> y >> z)) {
                // error
                throw std::runtime_error("obj file f line invalid");
            }

            int32_t x_i = int32_t(x - 1);
            int32_t y_i = int32_t(y - 1);
            int32_t z_i = int32_t(z - 1);

//            std::cout << "face = (" << x_i << ", " << y_i << ", " << z_i << ")\n";
            faces.push_back(glm::vec3(x_i, y_i, z_i));
        }
    }

    std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.f));
    if (normals_in.empty()) {
        // Compute normals per vertex
        std::vector<int32_t> num_normal_contribs(vertices.size(), 0);

        for (const glm::vec3& face : faces) {
            glm::vec3 edge_xy = vertices[face.y] - vertices[face.x];
            glm::vec3 edge_xz = vertices[face.z] - vertices[face.x];
            glm::vec3 edge_yz = vertices[face.z] - vertices[face.y];

            normals[face.x] += glm::normalize(glm::cross(edge_xy, edge_xz));
            num_normal_contribs[face.x] += 1;
            normals[face.y] += glm::normalize(glm::cross(edge_yz, -edge_xy));
            num_normal_contribs[face.y] += 1;
            normals[face.z] += glm::normalize(glm::cross(-edge_xz, -edge_yz));
            num_normal_contribs[face.z] += 1;
        }

        for (int32_t i = 0; i < normals.size(); i++) {
            normals[i] /= num_normal_contribs[i];
        }
    } else if (normals_in.size() != vertices.size()) {
        throw std::runtime_error("Obj file has a different number of normals than vertices");
    } else {
        std::memcpy(normals.data(), normals_in.data(), normals_in.size() * sizeof(glm::vec3));
    }

    // Fill vertex data
    for (int32_t i = 0; i < faces.size(); i++) {
        const glm::vec3& face = faces[i];
        insertVec3(m_vertexData, vertices[face.x]);
        insertVec3(m_vertexData, normals[face.x]);
        //insertVec3(m_vertexData, glm::vec3(0.f, 1.f, 0.f));

        insertVec3(m_vertexData, vertices[face.y]);
        insertVec3(m_vertexData, normals[face.y]);
        //insertVec3(m_vertexData, glm::vec3(1.f, 0.f, 0.f));


        insertVec3(m_vertexData, vertices[face.z]);
        insertVec3(m_vertexData, normals[face.z]);
        //insertVec3(m_vertexData, glm::vec3(0.f, 0.f, 1.f));



    }

    // Quick hack for LoD
    for (int32_t i = 0; i < n_lod; i++) {
        lod_shape_size[i] = m_vertexData.size();
    }
}

//void TriangleMesh::readMesh(const std::string& mesh_file) {
//    tinyobj::ObjReader reader;

//    if (!reader.ParseFromFile(mesh_file)) {
//        if (!reader.Error().empty()) {
//            throw std::runtime_error("TinyObjReader: " + reader.Error());
//        }
//    }

//    if (!reader.Warning().empty()) {
//        std::cerr << "TinyObjReader: " << reader.Warning();
//    }

//    auto& attrib = reader.GetAttrib();
//    auto& shapes = reader.GetShapes();

//    // Loop over shapes
//    for (size_t s = 0; s < shapes.size(); s++) {
//        // Loop over faces(polygon)
//        size_t index_offset = 0;
//        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
//            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

//            // Loop over vertices in the face.
//            for (size_t v = 0; v < fv; v++) {
//                // access to vertex
//                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
//                tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
//                tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
//                tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

//                // Check if `normal_index` is zero or positive. negative = no normal data
//                if (idx.normal_index >= 0) {
//                    tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
//                    tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
//                    tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
//                }

//                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
//                if (idx.texcoord_index >= 0) {
//                    tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
//                    tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
//                }

//                // Optional: vertex colors
//                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
//                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
//                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
//            }
//            index_offset += fv;

//            // per-face material
//            shapes[s].mesh.material_ids[f];
//        }
//    }
//}

void TriangleMesh::copyShapeIntoBuffer(std::vector<float>& buffer) {
    abs_shape_start = buffer.size();
    buffer.insert(buffer.end(), m_vertexData.begin(), m_vertexData.end());
}

const SceneMaterial& TriangleMesh::getMaterial() const {
    return m_material;
}

const glm::mat4* TriangleMesh::getModelMat() const {
    return &m_ctm;
}
const glm::mat3* TriangleMesh::getNormMat() const {
    return &m_norm_trans;
}

