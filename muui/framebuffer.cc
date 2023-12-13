#include "framebuffer.h"

#include "log.h"

namespace muui::gl
{

Framebuffer::Framebuffer(int width, int height)
    : m_texture(width, height, PixelType::RGBA)
{
    glGenFramebuffers(1, &m_fboId);
    glGenRenderbuffers(1, &m_rboId);

    FramebufferBinder binder(*this);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.id(), 0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboId);
}

Framebuffer::~Framebuffer()
{
    if (m_fboId)
        glDeleteFramebuffers(1, &m_fboId);
    if (m_rboId)
        glDeleteRenderbuffers(1, &m_rboId);
}

Framebuffer::Framebuffer(Framebuffer &&other)
    : m_fboId(other.m_fboId)
    , m_rboId(other.m_rboId)
    , m_texture(std::move(other.m_texture))
{
    other.m_fboId = 0;
    other.m_rboId = 0;
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other)
{
    m_fboId = other.m_fboId;
    m_rboId = other.m_rboId;
    m_texture = std::move(other.m_texture);

    other.m_fboId = 0;
    other.m_rboId = 0;

    return *this;
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rboId);
}

FramebufferBinder::FramebufferBinder(const Framebuffer &framebuffer)
{
    glGetIntegerv(GL_VIEWPORT, m_prevViewport.data());
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_prevFboId);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &m_prevRboId);
    framebuffer.bind();
    glViewport(0, 0, framebuffer.width(), framebuffer.height());
}

FramebufferBinder::~FramebufferBinder()
{
    glBindRenderbuffer(GL_RENDERBUFFER, m_prevRboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_prevFboId);
    glViewport(m_prevViewport[0], m_prevViewport[1], m_prevViewport[2], m_prevViewport[3]);
}

} // namespace muui::gl
