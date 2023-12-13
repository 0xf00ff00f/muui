#include "shadereffect.h"

#include "framebuffer.h"
#include "item.h"
#include "painter.h"
#include "spritebatcher.h"

namespace muui
{
ShaderEffect::~ShaderEffect() = default;

void ShaderEffect::resize(int width, int height)
{
    if (!m_framebuffer || (m_framebuffer->width() != width || m_framebuffer->height() != height))
        m_framebuffer = std::make_unique<gl::Framebuffer>(width, height);
    m_painter.setWindowSize(width, height);
}

void ShaderEffect::render(Item &item, Painter *painter, const glm::vec2 &pos, int depth)
{
    assert(m_framebuffer);

    {
        gl::FramebufferBinder binder(*m_framebuffer);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_painter.begin();
        item.doRender(&m_painter, glm::vec2{0, 0}, 0);
        m_painter.end();
    }

    applyEffect(painter->spriteBatcher(), pos, depth);
}

} // namespace muui
