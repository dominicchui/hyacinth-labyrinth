#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include <string>
//#include <filesystem>

#include "utils/rgba.h"

class Texture {
private:
    std::vector<RGBA> data;
    int32_t width;
    int32_t height;
public:
    Texture(const std::string& filename);
    //Texture(const std::filesystem::path& path);
    ~Texture(void);

    RGBA* getData(void);
    RGBA at(float u, float v);
};

#endif // TEXTURE_H
