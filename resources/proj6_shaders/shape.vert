#version 330 core

//layout(location = 0) in vec3 vert_color;
layout(location = 0) in vec3 vert_pos_obj;
layout(location = 1) in vec3 vert_norm_obj;

out vec4 frag_pos_world;
out vec3 frag_norm_world;
//out vec3 frag_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat3 norm_world;

void main() {
    // Task 8: compute the world-space position and normal, then pass them to
    //         the fragment shader using the variables created in task 5
    frag_pos_world = model * vec4(vert_pos_obj, 1.f);

    //mat3 m_inv_t = inverse(transpose(mat3(model)));
    frag_norm_world = normalize(norm_world * vert_norm_obj);

    // Recall that transforming normals requires obtaining the inverse-transpose of the model matrix!
    // In projects 5 and 6, consider the performance implications of performing this here.

    // set gl_Position to the object space position transformed to clip space
    gl_Position = proj * view * frag_pos_world;
}
