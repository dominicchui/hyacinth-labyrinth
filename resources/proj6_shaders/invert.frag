#version 330 core
in vec2 uv_frag;

uniform sampler2D my_non_kitten_texture;

out vec4 fragColor;

void main() {
    fragColor = texture(my_non_kitten_texture, uv_frag);
    fragColor = 1.0f - fragColor;
}
