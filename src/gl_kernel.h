#ifndef GL_KERNEL_H
#define GL_KERNEL_H

#include "GL/glew.h"
#include "utils/shaderloader.h"
#include "utils/debug.h"
#include "utils/utils.h"

#include <vector>
#include <string>

static void makeFBO(
    GLuint*  tex_id,
    GLuint*  rbo_id,
    GLuint*  fbo_id,
    GLuint   GL_TEXTURE_ID,
    GLuint   fbo_width,
    GLuint   fbo_height,
    uint8_t* texture_data) {
    // Shamelessly lifted from lab11
    // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, tex_id);
    glActiveTexture(GL_TEXTURE_ID);

    glBindTexture(GL_TEXTURE_2D, *tex_id);
    glErrorCheck();

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        fbo_width,
        fbo_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        texture_data
    );
    glErrorCheck();

    // Set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glErrorCheck();

    // Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, *rbo_id);
    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        fbo_width,
        fbo_height
    );
    glErrorCheck();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glErrorCheck();

    // Generate and bind an FBO
    glGenFramebuffers(1, fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo_id);
    glErrorCheck();

    // Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        *tex_id,
        0
    );
    glErrorCheck();
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        *rbo_id
    );
    glErrorCheck();

    // Unbind the texture (will unbind the FBO later)
    glBindTexture(GL_TEXTURE_2D, 0);
}

enum PostProcessEffect {
    PP_INVERT,
    PP_GRAYSCALE,
    PP_BLUR,
    PP_SOBEL
};

class GlConv2DFilter {
public:
    GLuint m_shader;
    GLuint m_width;
    GLuint m_height;
    GLuint m_texture_id;
    GLuint m_GL_TEXTURE_ID;
    GLuint m_GL_TEX_ID_AS_A_NUMBER;
    GLuint m_rbo;
    GLuint m_fbo;
    uint8_t* m_buffer;
    PostProcessEffect m_pp_effect;

    GLuint m_tex_loc;
//    GLuint m_vbo_fullscreen_quad;  // Passed in
//    GLuint m_vao_fullscreen_quad;  // Passed in

public:
    GlConv2DFilter(
        size_t width,
        size_t height,
        GLuint GL_TEXTURE_ID,
        GLuint GL_TEX_ID_AS_A_NUMBER,
        PostProcessEffect post_process_effect
    ) :
        m_shader(0),
        m_width(width),
        m_height(height),
        m_texture_id(0),
        m_GL_TEXTURE_ID(GL_TEXTURE_ID),
        m_GL_TEX_ID_AS_A_NUMBER(GL_TEX_ID_AS_A_NUMBER),
        m_rbo(0),
        m_fbo(0),
        m_buffer(nullptr),
        m_tex_loc(0),
        m_pp_effect(post_process_effect)
    {
        m_GL_TEXTURE_ID = GL_TEXTURE_ID;

        switch (m_pp_effect) {
        case PP_INVERT:
            m_shader = ShaderLoader::createShaderProgram(
                ":/resources/shaders/invert.vert",
                ":/resources/shaders/invert.frag"
                );
            break;
        case PP_GRAYSCALE:
            m_shader = ShaderLoader::createShaderProgram(
                ":/resources/shaders/grayscale.vert",
                ":/resources/shaders/grayscale.frag"
                );
            break;
        case PP_BLUR:
            m_shader = ShaderLoader::createShaderProgram(
                  ":/resources/shaders/blur.vert",
                  ":/resources/shaders/blur.frag"
            );
            break;
        case PP_SOBEL:
            m_shader = ShaderLoader::createShaderProgram(
                ":/resources/shaders/sobel.vert",
                ":/resources/shaders/sobel.frag"
                );
            break;
        default:
            throw std::runtime_error("Unkown post-process filter");
        };

        glErrorCheck();

        initGLState();
    }

    ~GlConv2DFilter(void) {
        deleteGLState();
        if (m_shader > 0) {
            glDeleteShader(m_shader);
        }
    }


    void initGLState(void) {
        if (m_buffer != nullptr) {
            free(m_buffer);
            m_buffer = nullptr;
        }

        m_buffer = reinterpret_cast<uint8_t*>(malloc(m_width*m_height * sizeof(int32_t)));
        if (m_buffer == nullptr) {
            throw std::runtime_error(
                std::string(__FILE__) + ": " +
                std::to_string(__LINE__) +
                "Failed to allocate FBO buffer"
            );
        }

        makeFBO(
            &m_texture_id,
            &m_rbo,
            &m_fbo,
            m_GL_TEXTURE_ID,
            m_width,
            m_height,
            m_buffer
        );

        glUseProgram(m_shader);
        m_tex_loc = myGlGetUniformLocation(m_shader, "my_non_kitten_texture");
        glUniform1i(m_tex_loc, m_GL_TEX_ID_AS_A_NUMBER);

        if (m_pp_effect == PP_BLUR    ||
            m_pp_effect == PP_SOBEL
        ) {
            // Delta dims is float(1/dim)

            float delta_x = 1.f / m_width;
            float delta_y = 1.f / m_height;

            GLuint delta_dims_loc = myGlGetUniformLocation(m_shader, "delta_dims");
            glUniform2f(delta_dims_loc, delta_x, delta_y);
        }
        glUseProgram(0);
    }

    void deleteGLState(void) {
        glDeleteTextures(1, &m_texture_id);
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteFramebuffers(1, &m_fbo);
        if (m_buffer != nullptr) {
            free(m_buffer);
            m_buffer = nullptr;
        }
    }

    void reinitializeGL(size_t width, size_t height) {
        if (m_shader != 0) {
            m_width = width;
            m_height = height;

            deleteGLState();
            initGLState();
        }
    }

    void render(GLuint quad_vao) {
        glUseProgram(m_shader);

        glBindVertexArray(quad_vao);

        glActiveTexture(m_GL_TEXTURE_ID);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

#endif // GL_KERNEL_H
