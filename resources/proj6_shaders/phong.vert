#version 330 core

// Vertex position in object space
layout(location = 0) in vec3 vertexObj;
layout(location = 1) in vec3 normalObj;

// Vertex position in world space
out vec3 vertexWorld;

// Vertex normal in world space
out vec3 normalWorld;

// MVP matricies
uniform mat4 model;
uniform mat3 invTransModel;
uniform mat4 view;
uniform mat4 proj;

void main() {
  // Compute the world space outputs
  vertexWorld = vec3(model * vec4(vertexObj, 1));
  normalWorld = invTransModel * normalObj;

  // Set gl_Position to the clip space position
  gl_Position = proj * view * model * vec4(vertexObj, 1);
}
