#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUV;
layout (location = 4) in vec4 lightSpacePos;

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
  mat4 lightProj;
  mat4 lightView;
} ubo;

// JANKTEX

layout(set = 0, binding = 1) uniform sampler2D texSampler0;
layout(set = 0, binding = 2) uniform sampler2D texSampler1;
layout(set = 0, binding = 3) uniform sampler2D texSampler2;
layout(set = 0, binding = 4) uniform sampler2D texSampler3;
layout(set = 0, binding = 5) uniform sampler2D texSampler4;
// layout(set = 0, binding = 6) uniform sampler2D texSampler5;
// layout(set = 0, binding = 7) uniform sampler2D texSampler6;
// layout(set = 0, binding = 8) uniform sampler2D texSampler7;


layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
  int tex_id;
} push;


vec3 read_tex_clr() {
    if (push.tex_id == 0) {
        return vec3(texture(texSampler0, fragUV));
    } else if (push.tex_id == 1) {
        return vec3(texture(texSampler1, fragUV));
    } else if (push.tex_id == 2) {
        return vec3(texture(texSampler2, fragUV));
    } else if (push.tex_id == 3) {
        return vec3(texture(texSampler3, fragUV));
    } else if (push.tex_id == 4) {
        return vec3(texture(texSampler4, fragUV));
    }/* else if (push.tex_id == 5) {
        return vec3(texture(texSampler5, fragUV));
    } else if (push.tex_id == 6) {
        return vec3(texture(texSampler6, fragUV));
    } else if (push.tex_id == 7) {
        return vec3(texture(texSampler7, fragUV));
    }*/
    return vec3(1.f, 1.f, 1.f);
}

//blogs.igalia.com/itoral/2017/10/02/working-with-lights-and-shadows-part-iii-rendering-the-shadows/
float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map)
{
   // Convert light space position to NDC
   vec3 light_space_ndc = light_space_pos.xyz; // /= light_space_pos.w;

   // If the fragment is outside the light's projection then it is outside
   // the light's influence, which means it is in the shadow (notice that
   // such sample would be outside the shadow map image)
   // if (abs(light_space_ndc.x) > 1.f ||
   //     abs(light_space_ndc.y) > 1.f ||
   //     abs(light_space_ndc.z) > 1.f)
   //    return 0.0;

   // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
   vec2 shadow_map_coord = light_space_ndc.xy * 0.5 + 0.5;

   // Check if the sample is in the light or in the shadow
   if (light_space_ndc.z > texture(shadow_map, shadow_map_coord.xy).x)
      return 0.0; // In the shadow

   // In the light
   return 1.0;
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
    //loat depth = texture(texSampler0, lightSpacePos.xy).x / 5.f;
    //out_color = vec4(1.0 - (1.0 - depth) * 100.0);
    float shadow = compute_shadow_factor(lightSpacePos, texSampler0);
    //float shadow = textureProj(lightSpacePos / lightSpacePos.w, vec2(0.f));
    outColor = vec4(1.0 - (1.0 - shadow));
    //outColor = vec4(depth, depth, depth, 1.0);
  // vec3 tex_clr = read_tex_clr();

  // vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  // vec3 specularLight = vec3(0.0);
  // vec3 surfaceNormal = normalize(fragNormalWorld);

  // vec3 cameraPosWorld = ubo.invView[3].xyz;
  // vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

  // for (int i = 0; i < ubo.numLights; i++) {
  //   PointLight light = ubo.pointLights[i];
  //   vec3 directionToLight = light.position.xyz - fragPosWorld;
  //   float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
  //   directionToLight = normalize(directionToLight);

  //   float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
  //   vec3 intensity = tex_clr * light.color.xyz * light.color.w * attenuation;

  //   diffuseLight += intensity * cosAngIncidence;

  //   // specular lighting
  //   vec3 halfAngle = normalize(directionToLight + viewDirection);
  //   float blinnTerm = dot(surfaceNormal, halfAngle);
  //   blinnTerm = clamp(blinnTerm, 0, 1);
  //   blinnTerm = pow(blinnTerm, 512.0); // higher values -> sharper highlight
  //   specularLight += intensity * blinnTerm;
  // }

  // vec4 camPos4 = ubo.invView * vec4(0.f,0.f,0.f,1.f);
  // vec3 camPos = vec3(camPos4[0], camPos4[1], camPos4[2]);
  // outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);

  // //outColor = vec4(fragUV[0], fragUV[1], 0.f, 1.f);
  // //outColor = texture(texSampler, fragUV);

  // float dist = clamp(distance(camPos, fragPosWorld) / 20.f, 0.f, 1.f);
  // outColor = nlerp(outColor, vec4(255.f, 255.f, 255.f, 255) / 255.f, dist);
}
