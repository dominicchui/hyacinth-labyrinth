#ifndef TEXTURE_H
#define TEXTURE_H

#include <glm/glm.hpp>
#include <vector>
#include <string>

class Texture {
private:
    std::vector<glm::vec4> data;
    int32_t width;
    int32_t height;
public:
    Texture(const std::string& filename);
    //Texture(const std::filesystem::path& path);
    ~Texture(void);

    glm::vec4* getData(void);
    glm::vec4 at(float u, float v);
};

#endif // TEXTURE_H
