#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>

#include "settings.h"
#include "utils/debug.h"
#include "utils/shaderloader.h"

// ================== Project 5: Lights, Camera

// Needed for the post-processing kernels.
static const std::vector<GLfloat> fullscreen_quad_data =
{   // x, y, z, u, v    //
    -1.0f,  1.0f, 0.0f,
    0.0f,  1.0f,
    -1.0f, -1.0f, 0.0f,
    0.0f,  0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,
    -1.0f,  1.0f, 0.0f,
    0.0f,  1.0f
};

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent),
    m_shader(0),
    m_vbo(0),
    m_vao(0),
    m_quad_vbo(0),
    m_quad_vao(0),
    m_default_fbo(1),
    m_pp_filters(),
    m_active_filters(),
    m_scene(nullptr),
    m_shape_buffer(),
    buf_size(),
    cam()
{
    m_prev_mouse_pos = glm::vec2(size().width()/2.f, size().height()/2.f);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this

    m_active_filters[0] = settings.invertFilter;
    m_active_filters[1] = settings.grayscaleFilter;
    m_active_filters[2] = settings.blurFilter;
    m_active_filters[3] = settings.sobelFilter;
}

Realtime::~Realtime() {
    delete m_scene;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    this->doneCurrent();
}

void Realtime::initializeGL() {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    m_devicePixelRatio = this->devicePixelRatio();

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL: OpenGL global state goes here.

    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, m_screen_width, m_screen_height);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/shape.vert",
        ":/resources/shaders/shape.frag"
    );
    glErrorCheck();

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glErrorCheck();

    static std::vector<float> dummy_buffer({0.f,0.f,0.f,0.f,0.f,0.f});
    std::vector<float>* obj_buffer;

    // Generate the scene
    if (!m_scene) {
        // No scene loaded
        obj_buffer = &dummy_buffer;
    } else {
        obj_buffer = m_scene->getObjBuffer();
    }

    buf_size = obj_buffer->size();

    glBufferData(GL_ARRAY_BUFFER,
                 obj_buffer->size() * sizeof(GLfloat),
                 obj_buffer->data(),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          3 * sizeof(GLfloat),
                          reinterpret_cast<void*>(0));

    // Set up post-processing filters
    glGenBuffers(1, &m_quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glErrorCheck();

    glBufferData(
        GL_ARRAY_BUFFER,
        fullscreen_quad_data.size()*sizeof(GLfloat),
        fullscreen_quad_data.data(),
        GL_STATIC_DRAW
    );
    glErrorCheck();

    glGenVertexArrays(1, &m_quad_vao);
    glBindVertexArray(m_quad_vao);
    glErrorCheck();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glErrorCheck();

    // HARDWARE TEXTURE UNIT ALLOCATION HAPPENS HERE!
    // For each filter we have, bump the default fbo by 1
    m_pp_filters.push_back(
        new GlConv2DFilter(
            m_fbo_width,
            m_fbo_height,
            GL_TEXTURE0,
            0,
            PP_INVERT
        )
    );
    m_default_fbo++;

    m_pp_filters.push_back(
        new GlConv2DFilter(
            m_fbo_width,
            m_fbo_height,
            GL_TEXTURE1,
            1,
            PP_GRAYSCALE
        )
    );
    m_default_fbo++;

    m_pp_filters.push_back(
        new GlConv2DFilter(
            m_fbo_width,
            m_fbo_height,
            GL_TEXTURE2,
            2,
            PP_BLUR
        )
    );
    m_default_fbo++;

    m_pp_filters.push_back(
        new GlConv2DFilter(
            m_fbo_width,
            m_fbo_height,
            GL_TEXTURE3,
            3,
            PP_SOBEL
        )
    );
    m_default_fbo++;

    recomputeMatrices();

    glBindFramebuffer(GL_FRAMEBUFFER, m_default_fbo);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Realtime::bindNextFBO(int32_t cur_idx) {
    for (int32_t i = cur_idx+1; i < m_pp_filters.size(); i++) {
        if (m_active_filters[i]) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_pp_filters[i]->m_fbo);
            glViewport(0, 0, m_pp_filters[i]->m_width, m_pp_filters[i]->m_height);
            return;
        }
    }
    // No more active filters
    glBindFramebuffer(GL_FRAMEBUFFER, m_default_fbo);
    glViewport(0, 0, m_screen_width, m_screen_height);
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here

    if (m_scene != nullptr) {
        bindNextFBO(-1);

        // Clear screen color and depth before painting
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Main draw call
        m_scene->draw(cam);

        // Post-processing
        for (int32_t i = 0; i < m_pp_filters.size(); i++) {
            if (!m_active_filters[i]) {
                continue;
            }
            bindNextFBO(i);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_pp_filters[i]->render(m_quad_vao);
        }
    }
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // FIXME m_conv2d_filter might not be initialized
    for (GlConv2DFilter* filt : m_pp_filters) {
        filt->reinitializeGL(m_fbo_width, m_fbo_height);
    }
    // Students: anything requiring OpenGL calls when the program starts should be done here
}

void Realtime::sceneChanged(const RenderData& meta_data) {
    delete m_scene;
    m_scene = new RasterScene(
        size().width(),
        size().height(),
        meta_data,
        m_shader,
        m_vbo,
        m_vao,
        std::make_pair(settings.shapeParameter1, settings.shapeParameter2)
    );

    cam.initScene(
        meta_data.cameraData,
        size().width(),
        size().height(),
        settings.nearPlane,
        settings.farPlane
    );
    recomputeMatrices();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    std::pair<int32_t, int32_t> tess_params(settings.shapeParameter1, settings.shapeParameter2);

    if (m_scene) {
        m_scene->updateTessParams(tess_params);
    }
    cam.near = settings.nearPlane;
    cam.far  = settings.farPlane;
    recomputeMatrices();

    m_active_filters[0] = settings.invertFilter;
    m_active_filters[1] = settings.grayscaleFilter;
    m_active_filters[2] = settings.blurFilter;
    m_active_filters[3] = settings.sobelFilter;

    update(); // asks for a PaintGL() call to occur
}

void Realtime::recomputeMatrices(void) {
    if (m_scene != nullptr) {
        // Update view matrix by rotating eye vector based on x and y angles
        glUseProgram(m_shader);
        glErrorCheck();

        cam.recomputeMatrices();
        glm::mat4 m_view = cam.view_mat;
        glm::mat4 m_proj = cam.proj_mat;

        // Pass in m_view and m_proj to shader
        GLint view_loc = glGetUniformLocation(m_shader, "view");
        if (view_loc < 0) {
            std::cerr << "glGetUniformLocation(\"view\") returned " << view_loc << std::endl;
        }
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, &m_view[0][0]);
        glErrorCheck();

        GLint proj_loc = glGetUniformLocation(m_shader, "proj");
        if (proj_loc < 0) {
            std::cerr << "glGetUniformLocation(\"proj\") returned " << proj_loc << std::endl;
        }
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, &m_proj[0][0]);
        glErrorCheck();

        update();
    }
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        float posX = event->position().x();
        float posY = event->position().y();
        float deltaX = posX - m_prev_mouse_pos.x;
        float deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        int elapsedms   = m_elapsedTimer.elapsed();
        float deltaTime = elapsedms * 0.001f;

        // Use deltaX and deltaY here to rotate
        //std::cout << "DeltaX: " << deltaX << ", DeltaY: " << deltaY << std::endl;
        if (cam.rotate(deltaX, deltaY, deltaTime)) {
            recomputeMatrices();
        }
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    bool fwd = m_keyMap.at(Qt::Key_W);
    bool bwd = m_keyMap.at(Qt::Key_S);
    bool lft = m_keyMap.at(Qt::Key_A);
    bool rgt = m_keyMap.at(Qt::Key_D);
    bool upw = m_keyMap.at(Qt::Key_Space);
    bool dwn = m_keyMap.at(Qt::Key_Control);

    if (cam.translate(fwd, bwd, lft, rgt, upw, dwn, deltaTime)) {
        recomputeMatrices();
    }

    // Use deltaTime and m_keyMap here to move around

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
