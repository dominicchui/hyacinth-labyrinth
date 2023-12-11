#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "scene/scene.h"

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    float m_devicePixelRatio;

    // FBO data
    GLuint m_defaultFBO = 2;
    int m_fbo_width;
    int m_fbo_height;
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    void makeFBO();

    // Screen data
    int m_screen_width;
    int m_screen_height;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    // Shader Loader
    GLuint m_phong_shader;
    GLuint m_texture_shader;

    // Camera data
    Camera *m_camera;

    // Scene data
    Scene *m_scene;

    Sphere *ball;
    float vel = 0;

    void incVel(float amount) {
        vel = fmax(fmin(vel + amount, 1), -1);
    }

    inline int sgn(float val) {
        return (0 < val) - (val < 0);
    }

    void zeroVel(float amount) {
        vel = fmax(vel - sgn(vel) * amount, 0);
    }

};
