#pragma once

#include <glm/glm.hpp>

#include "utils/scenedata.h"


// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
    glm::vec4 pos;

    // Unit vecs
    glm::vec3 look;
    glm::vec3 up;
    glm::vec3 right;

    // Rotation angles
    float theta_pitch;
    static constexpr float theta_pitch_max = M_PI/2.f - 0.00001;
    static constexpr float theta_pitch_min = -M_PI/2.f + 0.00001;

    float heightAngle; // The height angle of the camera in RADIANS

    float aperture;    // Only applicable for depth of field
    float focalLength; // Only applicable for depth of field

    // Scene bounds
    float width;
    float height;

public:
    float near;
    float far;

    // Speed
    static constexpr float mov_speed = 5.0f;
    static constexpr float mouse_sensitivity = 0.01;


public:
    glm::mat4 view_mat;
    glm::mat4 view_mat_inv;

    glm::mat4 proj_mat;
    glm::mat4 proj_mat_inv;

    glm::mat4 vp_mat;

    /////// Methods ////////

    Camera(void);

    void initScene(
        const SceneCameraData& cam_data,
        size_t width,
        size_t height,
        float nearPlane,
        float farPlane
    );

    // translate camera with 6 degrees of freedom
    // Returns true if a translation occurred, false otherwise
    bool translate(bool f, bool b, bool l, bool r, bool u, bool d, float deltaTime);

    // rotates camera with 2 degrees of freedom
    // Returns true if a rotation occurred, false otherwise
    bool rotate(float delta_x, float delta_y, float deltaTime);

    void recomputeMatrices();

    // Returns the camera position in world coords
    const glm::vec4& getPosition() const;

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;
};
