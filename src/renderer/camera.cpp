#include <stdexcept>
#include <glm/gtx/transform.hpp>

#include "camera.h"
#include "utils/debug.h"
#include "utils/utils.h"

Camera::Camera(CameraProjectionType proj_type) :
    pos(0.f),
    look(0.f, 0.f, -1.f),
    up(0.f, 1.f, 0.f),
    right(1.f, 0.f, 0.f),
    theta_pitch(0.f),
    heightAngle(glm::radians(45.f)),
    aperture(0),
    focalLength(0),
    width(0),
    height(0),
    proj_type(proj_type)
{}

void Camera::initScene(
    const SceneCameraData& cam_data,
    size_t scene_width,
    size_t scene_height,
    float nearPlane,
    float farPlane
) {
    pos  = cam_data.pos;

    look  = glm::normalize(glm::vec3(cam_data.look));
    up    = glm::normalize(glm::vec3(cam_data.up));
    right = glm::cross(-glm::vec3(look), glm::vec3(up)); // Guaranteed to be unit

    // Initialize rotation angles
    theta_pitch = std::asin(-look.y);

    heightAngle = cam_data.heightAngle;
    aperture    = cam_data.aperture;
    focalLength = cam_data.focalLength;

    width      = float(scene_width);
    height     = float(scene_height);
    near_plane = nearPlane;
    far_plane  = farPlane;
}

bool Camera::translate(bool f, bool b, bool l, bool r, bool u, bool d, float deltaTime) {
    // Translate cam-pos based on currently-pressed keys.

    glm::vec3 total_mov(0.f);
    if (f) {
        total_mov += look;
    }
    if (b) {
        total_mov -= look;
    }
    if (l) {
        // Right is a bit of a misnomer here
        total_mov += right;
    }
    if (r) {
        total_mov -= right;
    }
    if (u) {
        total_mov += up;
    }
    if (d) {
        total_mov -= up;
    }

    if (glm::length(total_mov) > 0.f) {
        total_mov = glm::normalize(total_mov) * mov_speed * deltaTime;
        glm::vec4 mov4 = glm::vec4(total_mov, 0.f);
        pos += mov4;

        return true;
    }
    return false;
}

static glm::mat3 getRotationMatrixRodrigues(float angle, glm::vec3 axis) {
    float s_t = glm::sin(angle);
    float c_t = glm::cos(angle);

    // Useful aliases...
    float t_c = 1.f - c_t;
    float u   = axis.x;
    float v   = axis.y;
    float w   = axis.z;
    float u2  = u*u;
    float v2  = v*v;
    float w2  = w*w;
    float uv  = u*v;
    float vw  = v*w;
    float uw  = u*w;

    return glm::mat3(
        c_t + u2*t_c  , uv*t_c + w*s_t, uw*t_c - v*s_t,
        uv*t_c - w*s_t, c_t + v2*t_c  , vw*t_c + u*s_t,
        uw*t_c + v*s_t, vw*t_c - u*s_t, c_t + w2*t_c
    );
}

static glm::mat3 getRotationMatrixUp(float angle) {
    float s_t = glm::sin(angle);
    float c_t = glm::cos(angle);

    return glm::mat3(c_t,0.f,-s_t,
                     0.f,1.0,0.f,
                     s_t,0.f,c_t
                     );
}

bool Camera::rotate(float delta_x, float delta_y, float deltaTime) {
    if (delta_x == 0.f && delta_y == 0.f) {
        return false;
    }
    bool rotated = false;

    // Convert distances to angles (this is a jank conversion...)
    float theta_x = -delta_x * mouse_sensitivity;
    float theta_y = delta_y * mouse_sensitivity;

    if (theta_x) {
        glm::mat3 rot_up = getRotationMatrixUp(theta_x);
        look = rot_up * look;
        right = glm::cross(-look, up);
        rotated = true;
    }

    // Rotation about "right" (delta_y)
    float new_theta_pitch = theta_pitch + theta_y;
    if ((theta_y > 0.f && new_theta_pitch < theta_pitch_max) ||
        (theta_y < 0.f && new_theta_pitch > theta_pitch_min)
    ) {
        glm::mat3 rot_right = glm::rotate(theta_y, right);
        look = rot_right * look;

        theta_pitch = new_theta_pitch;
        rotated = true;
    }

    look = glm::normalize(look);

    return rotated;
}

void Camera::recomputeMatrices() {
    // View matrix.
    glm::vec3 pos3 = glm::vec3(pos);

    view_mat = glm::lookAt(pos3, pos3 + look, up);

    //printMat4(view_mat);
    view_mat_inv = glm::inverse(view_mat);

    // Projection matrix
    float aspect_ratio = float(width) / float(height);
    float t_h = heightAngle;

    switch (proj_type) {
    case CAM_PROJ_ORTHO:
        proj_mat = glm::ortho(-width/2.f, width/2.f, -height/2.f, height/2.f, near_plane, far_plane);
        break;
    case CAM_PROJ_PERSP:
        proj_mat = glm::perspective(t_h, aspect_ratio, near_plane, far_plane);
        break;
    default:
        throw std::runtime_error("Invalid Camera Projection Type" + std::to_string(proj_type));
    }

    proj_mat_inv = glm::inverse(proj_mat);
}

const glm::vec4& Camera::getPosition() const {
    return pos;
}

float Camera::getAspectRatio() const {
    // Optional TODO: implement the getter or make your own design
    throw std::runtime_error("not implemented");
}

float Camera::getHeightAngle() const {
    return heightAngle;
}

float Camera::getFocalLength() const {
    // Optional TODO: implement the getter or make your own design
    return focalLength;
}

float Camera::getAperture() const {
    // Optional TODO: implement the getter or make your own design
    return aperture;
}
