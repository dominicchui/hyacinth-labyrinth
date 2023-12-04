#version 330 core

in vec4 frag_pos_world;
in vec3 frag_norm_world;
in vec3 frag_color;

// Global scene lighing info
uniform float k_a;      // Ambient
uniform float k_d;      // Diffuse
uniform float k_s;      // Specular

// Light info: keep in sync with scenedata.h
const uint LIGHT_POINT = 0U;
const uint LIGHT_DIRECTIONAL = 1U;
const uint LIGHT_SPOT = 2U;

struct Light {
    uint type;

    vec4 color;
    vec3 function; // Attenuation function

    vec4 pos; // Position with CTM applied (Not applicable to directional lights)
    vec4 dir; // Direction with CTM applied (Not applicable to point lights)

    float penumbra; // Only applicable to spot lights, in RADIANS
    float angle;    // Only applicable to spot lights, in RADIANS
};
uniform Light light_info[8];
uniform uint valid_lights;

// Cam info
uniform vec3  cam_pos;

// Per-object info
//struct obj_color_info {

//};

uniform vec3 obj_c_ambt;
uniform vec3 obj_c_dfus;
uniform vec3 obj_c_spec;
uniform float spec_exp;

// Output color
out vec4 out_color;

float getPointAttenuation(in uint light_idx, in vec3 light_vec) {
    float dist = length(light_vec);
    vec3  att  = light_info[light_idx].function;
    float ret  = 1.f / (att[0] + dist * att[1] + dist*dist * att[2]);

    return min(ret, 1.f);
}

// Same as the point light, but with an angular falloff
float getSpotAttenuation(in uint light_idx, in vec3 light_vec) {
    // Angular falloff

    Light light = light_info[light_idx];

    vec3 norm_light_vec = normalize(light_vec);
    vec3 light_dir      = normalize(vec3(light.dir));
    float cos_theta     = dot(norm_light_vec, light_dir);

    float theta       = acos(cos_theta);
    float theta_inner = light.angle - light.penumbra;

    float ret = 0.f;
    if (theta <= theta_inner) {
        ret = getPointAttenuation(light_idx, light_vec);
    } else if (theta <= light.angle) {
        // Enter gnarly equation
        // TODO: precompute some of these values
        float c = (theta - theta_inner) / (light.angle - theta_inner);
        float falloff = -2 * c*c*c + 3 * c*c;

        ret = (1.f - falloff) * getPointAttenuation(light_idx, light_vec);
    }

    return ret;
}

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized

    vec3 color = obj_c_ambt * k_a;

    for (uint i = 0U; i < valid_lights; i++) {
        vec3 light_dir;
        float f_att = 1.0f;

        if (light_info[i].type == LIGHT_POINT) {
            light_dir = vec3(light_info[i].pos - frag_pos_world);
            f_att = getPointAttenuation(i, -light_dir);
        } else if (light_info[i].type == LIGHT_DIRECTIONAL) {
            light_dir = -vec3(light_info[i].dir);
        } else if (light_info[i].type == LIGHT_SPOT) {
            light_dir = vec3(light_info[i].pos - frag_pos_world);
            f_att = getSpotAttenuation(i, -light_dir);
        } else {
            continue;
        }

        light_dir = normalize(light_dir);

        // Diffuse
        vec3 normalized_frag_norm_world = normalize(frag_norm_world);
        float light_dot_norm = dot(normalized_frag_norm_world, light_dir);

        if (light_dot_norm > 0.f) {
            light_dot_norm = light_dot_norm > 1.f ? 1.f : light_dot_norm;
            vec3 diffuse = f_att * k_d * light_dot_norm * vec3(light_info[i].color) * obj_c_dfus;
            color += diffuse;
        }

        // Specular
        vec3 incident_ray_w = normalize(vec3(frag_pos_world) - cam_pos);
        vec3 reflected_light_w = reflect(light_dir, normalized_frag_norm_world); // should be already normalized

        float refl_dot_cam = dot(reflected_light_w, incident_ray_w);
        if (refl_dot_cam > 0.f && spec_exp > 0) {
            refl_dot_cam = refl_dot_cam > 1.f ? 1.f : refl_dot_cam;
            vec3 specular = f_att * k_s * pow(refl_dot_cam, spec_exp) * vec3(light_info[i].color) * obj_c_spec;
            color += specular;
        }
    }

    out_color = vec4(color, 1.f);
}
