#pragma once

#include "gl.h"
#include "texture.h"

#include <array>

namespace muui::gl
{

class Framebuffer
{
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer &) = delete;
    Framebuffer &operator=(const Framebuffer &) = delete;

    Framebuffer(Framebuffer &&other);
    Framebuffer &operator=(Framebuffer &&other);

    int width() const { return m_texture.width(); }
    int height() const { return m_texture.height(); }
    const Texture *texture() const { return &m_texture; }

private:
    void bind() const;

    GLuint m_fboId{0};
    GLuint m_rboId{0};
    Texture m_texture;

    friend class FramebufferBinder;
};

class FramebufferBinder
{
public:
    FramebufferBinder(const Framebuffer &framebuffer);
    ~FramebufferBinder();

    FramebufferBinder(FramebufferBinder &) = delete;
    FramebufferBinder &operator=(const FramebufferBinder &) = delete;

    FramebufferBinder(FramebufferBinder &&) = delete;
    FramebufferBinder &operator=(const FramebufferBinder &&) = delete;

private:
    GLint m_prevFboId{0};
    GLint m_prevRboId{0};
    std::array<GLint, 4> m_prevViewport;
};

} // namespace muui::gl
