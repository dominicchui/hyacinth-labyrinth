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
#include <iostream>

#include "rasterscene.h"
#include "camera/camera.h"
#include "gl_kernel.h"

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    ~Realtime(void);
    void finish();                                      // Called on program exit
    void sceneChanged(const RenderData& metadata);
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void translate(void);

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Binds the FBO for the next valid filter that comes after cur_idx
    // Pass -1 for geometry rendering.
    // Binds the default FBO if cur_idx points to the last valid texture
    void bindNextFBO(int32_t cur_idx);

    void recomputeMatrices(void);

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    // OpenGL-specific stuff

    GLuint m_shader;
    GLuint m_vbo;
    GLuint m_vao;

    // Filter stuff
    GLuint m_quad_vbo;
    GLuint m_quad_vao;
    GLuint m_default_fbo;

    std::vector<GlConv2DFilter*> m_pp_filters;
    // Only 4 filters are supported now
    // with a very strict order:
    // 1. invert, 2. grayscale, 3. blur, 4. sobel
    bool m_active_filters[4];

    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;

    // Scene data
    RasterScene* m_scene;
    std::vector<float> m_shape_buffer;
    size_t buf_size;

    // Camera info
    Camera cam;
};
