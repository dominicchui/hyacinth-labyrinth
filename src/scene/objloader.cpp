#include "objloader.h"

#include <fstream>
#include <iostream>

// Contains the data representing the face of a triangle
struct Face {
    // TRUE is the normIdx array is populated
    bool hasNorm;
    // indices of the vertexes for this face
    int vertIdx[3];
    // indicies of the normals for this face
    int normIdx[3];
};

// Splits a string into a vector of strings based on a token delimiter
std::vector<std::string> split(std::string &line, char splitToken) {
    std::vector<std::string> words;
    std::string word = "";
    for (char c : line) {
        if (c == splitToken) {
            words.push_back(word);
            word = "";
        } else {
            word = word + c;
        }
    }
    words.push_back(word);
    return words;
}

// Counts the number of times a char appears in a word
int countChar(std::string &word, char token) {
    int count = 0;
    for (char c : word) if (c == token) count++;
    return count;
}

// Turns three strings (containing floats) as a vec3 using std::stof
glm::vec3 vecFromStrings(std::string &x, std::string &y, std::string &z) {
    return glm::vec3(std::stof(x), std::stof(y), std::stof(z));
}

// Populates the array `arr` with three strings converted into integer indices
// Indices in the OBJ file are 1-indexed, so subtracts one from each given string
void popArrFromStrings(int *arr, std::string &x, std::string &y, std::string &z) {
    arr[0] = std::stoi(x) - 1;
    arr[1] = std::stoi(y) - 1;
    arr[2] = std::stoi(z) - 1;
}

// Generates a face given the index data
Face makeFace(std::string &d1, std::string &d2, std::string &d3) {
    Face face = Face();
    if (countChar(d1, '/') == 0) {
        // face P
        face.hasNorm = false;
        popArrFromStrings(face.vertIdx, d1, d2, d3);
    } else {
        // face (P//N)
        std::vector<std::string> v1, v2, v3;
        v1 = split(d1, '/');
        v2 = split(d2, '/');
        v3 = split(d3, '/');
        face.hasNorm = true;
        popArrFromStrings(face.vertIdx, v1[0], v2[0], v3[0]);
        popArrFromStrings(face.normIdx, v1[2], v2[2], v3[2]);
    }
    return face;
}

void ObjLoader::load(std::string &filepath) {
    vertices.clear();
    normals.clear();

    std::vector<glm::vec3> vdata;
    std::vector<glm::vec3> ndata;
    std::vector<Face> fdata;

    std::ifstream file(filepath);
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> words = split(line, ' ');
        if (words[0] == "#" or std::all_of(words[0].begin(), words[0].end(), isspace)) {
            // comment or whitespace
            continue;
        } else if (words[0] == "v") { // vertex
            vdata.push_back(vecFromStrings(words[1], words[2], words[3]));
        } else if (words[0] == "vn") { // vertex normal
            ndata.push_back(normalize(vecFromStrings(words[1], words[2], words[3])));
        } else if (words[0] == "f") { // face
            fdata.push_back(makeFace(words[1], words[2], words[3]));
        } else {
            std::cerr << "Error: cannot parse file " << filepath
                      << ": token '" << words[0]
                      << "' is unsupported."  << std::endl;
        }
    }

    for (Face face : fdata) {
        if (face.hasNorm) {
            vertices.push_back(vdata[face.vertIdx[0]]);
            vertices.push_back(vdata[face.vertIdx[1]]);
            vertices.push_back(vdata[face.vertIdx[2]]);
            normals.push_back(ndata[face.normIdx[0]]);
            normals.push_back(ndata[face.normIdx[1]]);
            normals.push_back(ndata[face.normIdx[2]]);
        } else {
            vertices.push_back(vdata[face.vertIdx[0]]);
            vertices.push_back(vdata[face.vertIdx[1]]);
            vertices.push_back(vdata[face.vertIdx[2]]);

            glm::vec3 A = vdata[face.vertIdx[0]] - vdata[face.vertIdx[1]];
            glm::vec3 B = vdata[face.vertIdx[2]] - vdata[face.vertIdx[1]];
            glm::vec3 normal = normalize(cross(A, B));

            normals.push_back(-normal);
            normals.push_back(-normal);
            normals.push_back(-normal);
        }
    }
}

std::vector<glm::vec3> ObjLoader::getData() {
    std::vector<glm::vec3> zip;
    for (int i = 0; i < vertices.size(); i++) {
        zip.push_back(vertices[i]);
        zip.push_back(normals[i]);
    }
    return zip;
}
