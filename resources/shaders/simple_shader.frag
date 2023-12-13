#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

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

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;



vec4 lerp(vec4 a, vec4 b, float t) {
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
    vec3 intensity = light.color.xyz * light.color.w * attenuation;

    diffuseLight += intensity * cosAngIncidence;

    // specular lighting
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 5.0); // higher values -> sharper highlight
    specularLight += intensity * blinnTerm;
  }

  vec4 camPos4 = ubo.invView * vec4(0.f,0.f,0.f,1.f);
  vec3 camPos = vec3(camPos4[0], camPos4[1], camPos4[2]);
  outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
  float dist = clamp(distance(camPos, fragPosWorld) / 20.f, 0.f, 1.f);
  outColor = lerp(outColor, vec4(255.f, 255.f, 255.f, 255) / 255.f, dist);
}
