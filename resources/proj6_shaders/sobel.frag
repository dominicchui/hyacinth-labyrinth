
#version 330 core
in vec2 uv_frag;

uniform sampler2D my_non_kitten_texture;

out vec4 fragColor;

// Need to know the distance between each adjacent pixel.
uniform vec2 delta_dims;

// No actual need to declare it lol
const vec3 sobel[3] = vec3[3](
    vec3(1.f, 0.f, -1.f),
    vec3(2.f, 0.f, -2.f),
    vec3(1.f, 0.f, -1.f)
);

float rgba_to_gray_luma(vec4 frag) {
    float intensity = 0.299 * frag[0] +
                      0.587 * frag[1] +
                      0.114 * frag[2];

    return intensity;
}

float sensitivity = 0.5f;

void main() {
    float x_out = 0;
    float y_out = 0;

    // Convolve by a sobel filter

    // X and Y directions combined
    for (int y = -1; y < 2; y++) {
        float v = uv_frag[1] + y * delta_dims[1];

        for (int x = -1; x < 2; x++) {
            float u = uv_frag[0] + x * delta_dims[0];

            float k_val_x = sobel[y][x];
            float k_val_y = sobel[x][y];

            float tex_gray =
                rgba_to_gray_luma(texture(my_non_kitten_texture, vec2(u, v)));

            x_out += tex_gray * k_val_x;
            y_out += tex_gray * k_val_y;
        }
    }

    float out_clr = sqrt(x_out*x_out + y_out*y_out) * sensitivity;
    //float out_clr = x_out * sensitivity;
    fragColor = vec4(out_clr, out_clr, out_clr, 1.f);

    // return fragColor
}
