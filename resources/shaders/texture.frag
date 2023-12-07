#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uvCoord;

// Task 8: Add a sampler2D uniform
uniform sampler2D texture;

uniform float width;
uniform float height;

// Task 29: Add a bool on whether or not to filter the texture
uniform bool invert;
uniform bool gray;
uniform bool toneMap;
uniform float gamma;
uniform bool blur;
uniform bool sharp;
uniform bool laplacian;

out vec4 fragColor;

float lum(vec4 color) {
  return (0.299 * color.r) + (0.587 * color.g) + (0.114 * color.b);
}

void main()
{
  // Task 17: Set fragColor using the sampler2D at the UV coordinate
//  fragColor = vec4(1);
  fragColor = texture(texture, uvCoord);

  // Task 33: Invert fragColor's r, g, and b color channels if your bool is true
  float wOff = 1.f / width;
  float hOff = 1.f / height;

  if (blur) {
    vec3 acc = vec3(0);
    for (int c = -2; c < 3; c++) {
      for (int r = -2; r < 3; r++) {
        float u = uvCoord[0] + (wOff * c);
        float v = uvCoord[1] + (hOff * r);
        acc += vec3(texture(texture, vec2(u, v)));
      }
    }

    acc /= 25.f;
    fragColor = vec4(acc, 1);

  }

  if (sharp) {
    float ign = -1.f / 9.f;
    mat3 kern = mat3(
      ign, ign, ign,
      ign, 17.f / 9.f, ign,
      ign, ign, ign
    );

    vec3 acc = vec3(0);
    for (int c = 0; c < 3; c++) {
      for (int r = 0; r < 3; r++) {
        float u = uvCoord[0] + (wOff * float(1.f - c));
        float v = uvCoord[1] + (hOff * float(1.f - r));
        vec3 pixel = vec3(texture(texture, vec2(u, v)));
        acc += pixel * kern[c][r];
      }
    }

    fragColor = vec4(acc, 1);
  }

  if (laplacian) {
    mat3 kern = mat3(
      0, 1, 0,
      1, -4, 1,
      0, 1, 0
    );

    vec3 acc = vec3(0);
    for (int c = 0; c < 3; c++) {
      for (int r = 0; r < 3; r++) {
        float u = uvCoord[0] + (wOff * float(1.f - c));
        float v = uvCoord[1] + (hOff * float(1.f - r));
        vec3 pixel = vec3(texture(texture, vec2(u, v)));
        acc += pixel * kern[c][r];
      }
    }

    fragColor = vec4(acc, 1);
  }

  if (toneMap) {
    vec3 c = vec3(fragColor);
    fragColor = vec4(pow(c.r, gamma), pow(c.g, gamma), pow(c.b, gamma), 1);
  }

  if (invert) fragColor = 1.f - fragColor;
  if (gray) fragColor = vec4(lum(fragColor));
}
