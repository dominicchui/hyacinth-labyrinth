#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "utils/settings.h"
#include <glm/gtx/string_cast.hpp>

#include "scene/shaderloader.h"

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
    m_camera = new Camera(CAM_PROJ_PERSP);
    m_scene = new Scene(m_camera);
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    delete m_scene;
    glDeleteProgram(m_phong_shader);
    glDeleteProgram(m_texture_shader);

    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);

    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    this->doneCurrent();
}

void Realtime::makeFBO(){
    // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);

    // Set background color
//    auto r = glm::vec4(128, 64, 25, 255) / 255.f;
//    glClearColor(r.r, r.g, r.b, 1);
    glClearColor(0, 0, 0, 1);

    // Setup shader
    m_phong_shader = ShaderLoader::createShaderProgram(
        ":/resources/proj6_shaders/phong.vert",
        ":/resources/proj6_shaders/phong.frag");
    m_texture_shader = ShaderLoader::createShaderProgram(
        ":/resources/proj6_shaders/texture.vert",
        ":/resources/proj6_shaders/texture.frag");

    // Initialize scene data
    m_scene->init();

    glm::vec4 cyan = {0,1,1,1};
    glm::mat4 flctm = {{9.000000, 0.000000, 0.000000, 0.000000}, {0.000000, 0.100000, 0.000000, 0.000000}, {0.000000, 0.000000, 9.000000, 0.000000}, {0.000000, -0.900000, 0.000000, 1.000000}};
    RenderShapeData fl_data = RenderShapeData{
        ScenePrimitive{
            PrimitiveType::PRIMITIVE_CUBE,
            SceneMaterial{cyan,cyan,cyan,2},
            ""}, flctm};
    Cube *flr = new Cube(fl_data);
    m_scene->add(flr);

    glm::vec4 yellow = {1,1,0,1};
    std::vector<std::vector<bool>> map = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    std::vector<Cube *> walls;
    glm::vec3 coord = {-4, -0.5, -4};
    for (auto row : map) {
        for (auto cell : row) {
            if (cell) {
                RenderShapeData wl_data{
                    ScenePrimitive{
                        PrimitiveType::PRIMITIVE_CUBE,
                        SceneMaterial{yellow,yellow,yellow,2},
                        ""}, glm::translate(glm::mat4(1), coord)};

                Cube *wall = new Cube(wl_data);
                m_scene->add(wall);
                walls.push_back(wall);
            }
            coord.z++;
        }
        coord.x++;
        coord.z = -4;
    }

    glm::vec4 purp = {1,0,1,1};
    RenderShapeData sp_data{
        ScenePrimitive{
            PrimitiveType::PRIMITIVE_SPHERE,
            SceneMaterial{purp,purp,purp,2},
            ""}, glm::mat4(1)};
    ball = new Sphere(sp_data);
    ball->setBall(-0.5, walls);
    m_scene->add(ball);

    // Set the texture.frag uniform for our texture
    glUseProgram(m_texture_shader);
    glUniform1i(glGetUniformLocation(m_texture_shader, "texture"), 0);
    glUseProgram(0);

    std::vector<GLfloat> fullscreen_quad_data = {
        -1.f,  1.f, 0.f, //tl
        0.f,  1.f,
        -1.f, -1.f, 0.f, //bl
        0.f,  0.f,
        1.f, -1.f, 0.f, //br
        1.f,  0.f,
        1.f,  1.f, 0.f, //tr
        1.f,  1.f,
        -1.f,  1.f, 0.f, //tl
        0.f,  1.f,
        1.f, -1.f, 0.f, //br
        1.f,  0.f
    };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here

    // Bind the scene framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the scene to the scene
    m_scene->draw(m_phong_shader);

    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set postprocessing parameters
    glUseProgram(m_texture_shader);
    glUniform1f(glGetUniformLocation(m_texture_shader, "width"), m_screen_width);
    glUniform1f(glGetUniformLocation(m_texture_shader, "height"), m_screen_height);
    glUniform1i(glGetUniformLocation(m_texture_shader, "invert"), settings.perPixelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "gray"), settings.extraCredit1);
    glUniform1i(glGetUniformLocation(m_texture_shader, "toneMap"), settings.extraCredit3);
    glUniform1f(glGetUniformLocation(m_texture_shader, "gamma"), settings.ecGamma);
    glUniform1i(glGetUniformLocation(m_texture_shader, "blur"), settings.kernelBasedFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "sharp"), settings.extraCredit2);
    glUniform1i(glGetUniformLocation(m_texture_shader, "laplacian"), settings.extraCredit4);

    // Draw the scene to the screen
    glBindVertexArray(m_fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Unbinding
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Delete Texture, Renderbuffer, and Framebuffer memory
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    // Update camera and FBO parameters
   // m_camera->resize(w, h);
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Regenerate FBOs
    makeFBO();

    update();
}

void Realtime::sceneChanged() {
    m_scene->loadScene();
    update();
}

void Realtime::settingsChanged() {
    m_scene->update();
    update();
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
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        //m_camera->rotate(-deltaX, -deltaY);

        update();
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
//     int elapsedms   = m_elapsedTimer.elapsed();
//     float deltaTime = elapsedms * 0.001f;
//     m_elapsedTimer.restart();

//     // Use deltaTime and m_keyMap here to move around
//     float dist = 5.f * deltaTime;

//     if (m_scene->loaded) {
//         if (m_keyMap[Qt::Key_W]) m_camera->moveLook(dist);
//         if (m_keyMap[Qt::Key_S]) m_camera->moveLook(-dist);

//         if (m_keyMap[Qt::Key_A]) m_camera->moveSide(-dist);
//         if (m_keyMap[Qt::Key_D]) m_camera->moveSide(dist);
//     } else {
//         auto forward = m_camera->getLook();
//         forward.y = 0;
//         auto side = m_camera->getSide();
//         side.y = 0;

// //        float delta = 0;
//         if (m_keyMap[Qt::Key_W]) ball->move(dist, forward);
//         if (m_keyMap[Qt::Key_S]) ball->move(dist, -forward);

// //        if (m_keyMap[Qt::Key_W]) incVel(0.02);
// //        if (m_keyMap[Qt::Key_S]) incVel(-0.02);
// //        ball->move(dist, vel * m_camera->getLook());

// //        delta = 0;
//         if (m_keyMap[Qt::Key_A]) ball->move(dist, side);
//         if (m_keyMap[Qt::Key_D]) ball->move(dist, -side);
// //        if (m_keyMap[Qt::Key_A]) incVel(0.02);
// //        if (m_keyMap[Qt::Key_D]) incVel(-0.02);
// //        ball->move(dist, vel * m_camera->getSide());

//         if (m_keyMap[Qt::Key_Space]) ball->move(dist, m_camera->getUp());

//         // gravity
//         if (!ball->onFloor()) ball->move(dist, -m_camera->getUp() / 2.f);
// //        zeroVel(0.015);

// //        if (m_keyMap[Qt::Key_Space])  m_camera->moveVertical(dist);
// //        if (m_keyMap[Qt::Key_Control]) m_camera->moveVertical(-dist);
//     }

//     update();
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
