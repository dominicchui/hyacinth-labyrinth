
#version 330 core
in vec2 uv_frag;

uniform sampler2D my_non_kitten_texture;

out vec4 fragColor;

// Need to know the distance between each adjacent pixel.
uniform vec2 delta_dims;

// No actual need to declare it lol
//float blur_kernel[25] = float[](
//    0.04, 0.04, 0.04, 0.04, 0.04,
//    0.04, 0.04, 0.04, 0.04, 0.04,
//    0.04, 0.04, 0.04, 0.04, 0.04,
//    0.04, 0.04, 0.04, 0.04, 0.04,
//    0.04, 0.04, 0.04, 0.04, 0.04
//);

void main() {
    fragColor = vec4(0.f);

    // Convolve by a 5x5 blur filter (1/25 all around)
    for (int y = -2; y < 3; y++) {
        float v = uv_frag[1] + y * delta_dims[1];
        for (int x = -2; x < 3; x++) {
            float u = uv_frag[0] + x * delta_dims[0];
            fragColor += texture(my_non_kitten_texture, vec2(u, v)) * 0.04;
        }
    }
    fragColor[3] = 1.f;
    // return fragColor
}
