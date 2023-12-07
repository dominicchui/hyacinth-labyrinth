#version 330 core
in vec2 uv_frag;

uniform sampler2D my_non_kitten_texture;

out vec4 fragColor;

vec4 rgba_to_gray_luma(vec4 frag) {
    float intensity = 0.299 * frag[0] +
                      0.587 * frag[1] +
                      0.114 * frag[2];

    return vec4(intensity, intensity, intensity, 1.0f);
}


void main() {
    fragColor = texture(my_non_kitten_texture, uv_frag);
    fragColor = rgba_to_gray_luma(fragColor);
}
