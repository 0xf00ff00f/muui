#include "shadereffect.h"

#include "framebuffer.h"
#include "item.h"
#include "painter.h"
#include "spritebatcher.h"

namespace muui
{
ShaderEffect::ShaderEffect() = default;

ShaderEffect::~ShaderEffect()
{
    m_resizedConnection.disconnect();
}

void ShaderEffect::setSource(Item *source)
{
    if (source == m_source)
        return;

    m_resizedConnection.disconnect();

    m_source = source;

    if (m_source)
    {
        auto resize = [this] {
            assert(m_source);
            const auto w = static_cast<int>(std::ceil(m_source->width()));
            const auto h = static_cast<int>(std::ceil(m_source->height()));
            m_painter.setWindowSize(w, h);
        };
        resize();
        m_resizedConnection = source->resizedSignal.connect([&resize](Size) { resize(); });
    }
}

void ShaderEffect::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!m_source)
        return;

    const auto w = m_painter.windowWidth();
    const auto h = m_painter.windowHeight();
    if (!m_framebuffer || (m_framebuffer->width() != w || m_framebuffer->height() != h))
        m_framebuffer = std::make_unique<gl::Framebuffer>(w, h);

    {
        gl::FramebufferBinder binder(*m_framebuffer);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_painter.begin();
        m_source->doRender(&m_painter, glm::vec2{0, 0}, 0);
        m_painter.end();
    }

    applyEffect(painter->spriteBatcher(), pos, depth);
}

} // namespace muui
