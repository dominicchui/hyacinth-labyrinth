#include "texture.h"
#include "utils/rgba.h"

#include <QString>
#include <QByteArray>
#include <QImage>

#include <iostream>
#include <stdexcept>
#include <cmath>

Texture::Texture(const std::string& filename) :
    data(),
    width(0),
    height(0)
{
    // Load texture from file

    QString qfname = QString::fromStdString(filename);
    QImage myTexture;

    if (!myTexture.load(qfname)) {
        throw std::runtime_error("Failed to load in image: " + filename);
    }
    myTexture = myTexture.convertToFormat(QImage::Format_RGBX8888);
    width = myTexture.width();
    height = myTexture.height();

    data.reserve(width*height);
    QByteArray arr = QByteArray::fromRawData((const char*) myTexture.bits(), myTexture.sizeInBytes());

    for (int i = 0; i < arr.size() / 4.f; i++){
        data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
}

Texture::~Texture() {}

RGBA* Texture::getData(void) {
    return data.data();
}

RGBA Texture::at(float u, float v) {
    // UV -> XY
    int32_t x = int32_t(std::round(u * (width-1))) % width;
    int32_t y = int32_t(std::round((1.f - v) * (height-1))) % height;

    return data.at(y * width + x);

}
