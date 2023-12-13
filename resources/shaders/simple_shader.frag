#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

// JANKTEX

layout(set = 0, binding = 1) uniform sampler2D texSampler0;
layout(set = 0, binding = 2) uniform sampler2D texSampler1;
layout(set = 0, binding = 3) uniform sampler2D texSampler2;
layout(set = 0, binding = 4) uniform sampler2D texSampler3;
layout(set = 0, binding = 5) uniform sampler2D texSampler4;
layout(set = 0, binding = 6) uniform sampler2D texSampler5;
layout(set = 0, binding = 7) uniform sampler2D texSampler6;
layout(set = 0, binding = 8) uniform sampler2D texSampler7;


layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
  int tex_id;
} push;


vec3 read_tex_clr(vec2 frag_uv) {
    frag_uv = vec2(frag_uv[0], 1.f - frag_uv[1]);
    if (push.tex_id == 0) {
        return vec3(texture(texSampler0, frag_uv));
    } else if (push.tex_id == 1) {
        return vec3(texture(texSampler1, frag_uv));
    } else if (push.tex_id == 2) {
        return vec3(texture(texSampler2, frag_uv));
    } else if (push.tex_id == 3) {
        return vec3(texture(texSampler3, frag_uv));
    } else if (push.tex_id == 4) {
        return vec3(texture(texSampler4, frag_uv));
    } else if (push.tex_id == 5) {
        return vec3(texture(texSampler5, frag_uv));
    } else if (push.tex_id == 6) {
        return vec3(texture(texSampler6, frag_uv));
    } else if (push.tex_id == 7) {
        return vec3(texture(texSampler7, frag_uv));
    }
    return vec3(1.f, 1.f, 1.f);
}

vec4 nlerp(vec4 a, vec4 b, float t) {
    float easeFactor;
    if (t < 0.5) {
        return a;
    } else {
        t = (t - 0.5)*2;
        easeFactor = (t == 0) ? 0 : pow(2, 10 * t - 10);
        return a + easeFactor*(b-a);
    }
}

void main() {
  vec3 tex_clr = read_tex_clr(fragUV);

  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld);

  vec3 cameraPosWorld = ubo.invView[3].xyz;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
    directionToLight = normalize(directionToLight);

    float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 intensity = tex_clr * light.color.xyz * light.color.w * attenuation;

    diffuseLight += intensity * cosAngIncidence;

    // specular lighting
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
    specularLight += intensity * blinnTerm;
  }

  vec4 camPos4 = ubo.invView * vec4(0.f,0.f,0.f,1.f);
  vec3 camPos = vec3(camPos4[0], camPos4[1], camPos4[2]);
  outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);

  //outColor = vec4(fragUV[0], fragUV[1], 0.f, 1.f);
  //outColor = texture(texSampler, fragUV);

  float dist = clamp(distance(camPos, fragPosWorld) / 20.f, 0.f, 1.f);
  outColor = nlerp(outColor, vec4(255.f, 255.f, 255.f, 255) / 255.f, dist);

}
