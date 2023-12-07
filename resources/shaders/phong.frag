#version 330 core

// Vertex position in world space
in vec3 vertexWorld;
// Vertex normal in world space
in vec3 normalWorld;

// Output fragment color
out vec4 fragColor;

// Camera and light positions
uniform vec3 camPos;
struct LightData {
  int type; // 0: Point, 1: Directional, 2: Spot
  vec4 color;

  vec3 dir;       // not used by point
  vec3 pos;       // not used by directional
  vec3 atten;     // not used by directional, attenuation coefficents
  float penumbra; // only for spot
  float angle;    // only for spot
};
uniform int numLights;
uniform LightData lights[8];

// Ambient lighting
uniform vec4 cAmb;
uniform float ka;

// Diffuse lighting
uniform vec4 cDiff;
uniform float kd;

// Specular lighting
uniform vec4 cSpec;
uniform float ks;
uniform float shininess;

// Clamps a float between 0 and 1
float clamp(float val) {
  return min(max(val, 0), 1);
}

// Gets the attenuation factor of the given light
float getAttenuation(vec3 lightPos, vec3 pos, vec3 coef, bool directional) {
    float fatt = 1.f;
    if (!directional) {
        float dist = distance(lightPos, pos);
        fatt = 1.f / (coef[0] +
                      (dist * coef[1]) +
                      (dist * dist * coef[2]));
        if (fatt >= 1) fatt = 1.f;
    }
    return fatt;
}

// Finds the color of a light. If not a spot light, this is `base`
vec4 getLightColor(vec4 base, bool spot, float angle, float thetaInner, float thetaOuter) {
    vec4 color = base;
    if (spot && (angle > thetaInner)) {
        // in the outer cone
        float term = (angle - thetaInner) / (thetaOuter - thetaInner);
        float falloff = (-2 * term * term * term) + (3 * term * term);
        color *= 1.f - falloff;
    }
    return color;
}

vec4 lerp(vec4 a, vec4 b, float t) {
  return a + t * (b - a);
}

void main() {
  vec3 norm = normalize(normalWorld);
  vec3 dirToCam = normalize(camPos - vertexWorld);

  // Ambient component
  vec4 amb = cAmb * ka;

  vec4 diffspec = vec4(0);
  for (int i = 0; i < numLights; i++) {
    LightData light = lights[i];

    vec3 dirToLight = (light.type == 1) ? -normalize(light.dir) : normalize(light.pos - vertexWorld);

    float fatt = getAttenuation(light.pos, vertexWorld, light.atten, light.type == 1);

    // Diffuse component
    float diffTerm = kd * clamp(dot(norm, dirToLight));
    vec4 diff = cDiff * diffTerm;

    // Specular component
    vec3 reflection = normalize(reflect(-dirToLight, norm));
    float specTerm = shininess <= 0 ? 1 : ks * pow(clamp(dot(dirToCam, reflection)), shininess);
    vec4 spec = cSpec * specTerm;

    // Calculate light color
    float angle = 0;
    if (light.type == 2) {
      angle = acos(dot(dirToLight, -normalize(light.dir)));
      if (angle > light.angle) continue;
    }
    vec4 lightColor = getLightColor(light.color, light.type == 2, angle, light.angle - light.penumbra, light.angle);

    diffspec += fatt * lightColor * (diff + spec);
  }

  // Phong illumination
  fragColor = amb + diffspec;

//  float con = clamp(distance(camPos, vertexWorld) / 20.f);
//  fragColor = lerp(fragColor, vec4(128, 64, 25, 255) / 255.f, con);
}
