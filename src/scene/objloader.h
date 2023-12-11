#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

// Reads and loads data from an OBJ file
class ObjLoader {
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
public:
    // Loads data from the given filepath and stores the vertex and normal
    // data, which can be retreived later
    void load(std::string &filepath);
    // Gets the zipped vertex and normal data, in the form of <v, n, v, n, ...>
    // for all vertices and normals in the OBJ file
    std::vector<glm::vec3> getData();
};
