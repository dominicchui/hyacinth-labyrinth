#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include <glm/glm.hpp>

struct RGBA {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a = 255;

    bool operator!=(const RGBA& other) {
        return (r != other.r ||
                g != other.g ||
                b != other.b ||
                a != other.a);
    }
};

static constexpr RGBA zero_clr = RGBA{0,0,0,0};

static float u8_to_f32(uint8_t in) {
    return float(in) / 255.f;
}

static uint8_t f32_to_u8(float in) {
    return uint8_t(std::clamp(std::round(in * 255.f), 0.f, 255.f));
}

// Helper function to convert illumination to RGBA, applying some form of tone-mapping (e.g. clamping) in the process
static RGBA vec4ToRGBA(const glm::vec4& illumination) {
    static constexpr glm::vec4 lo(0.f, 0.f, 0.f, 0.f);
    static constexpr glm::vec4 hi(1.f, 1.f, 1.f, 1.f);

    glm::vec4 clamped = glm::clamp(illumination, lo, hi);
    glm::u8vec4 u8vec = clamped * 255.f;

    return RGBA{u8vec[0], u8vec[1], u8vec[2], u8vec[3]};
}

// The reverse of the above function
static glm::vec4 RGBAToVec4(const RGBA& rgba) {
    return glm::vec4{rgba.r/255.f, rgba.g/255.f, rgba.b/255.f, rgba.a/255.f};
}

static constexpr float eps = 1e-4;

#define LUMA 1

static float rgba_to_gray(const RGBA &pixel) {
    // Task 3: Calculate and return the gray value of the
    //         pixel using its RGB components.
    // Note: We are returning an std::uint8_t, because it is 1 byte
    //       and can store values from 0-255!

#if LUMA == 1
    float intensity = 0.299 * u8_to_f32(pixel.r) +
                      0.587 * u8_to_f32(pixel.g) +
                      0.114 * u8_to_f32(pixel.b);
#else // Use lightness method
    uint8_t max = std::max(pixel.r, std::max(pixel.g, pixel.b));
    uint8_t min = std::min(pixel.r, std::min(pixel.g, pixel.b));
    float   intensity = (float(max) + float(min))/ 2;
#endif
    return intensity;
}
