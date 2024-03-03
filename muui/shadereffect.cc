#include "shadereffect.h"

#include "framebuffer.h"
#include "item.h"
#include "painter.h"

namespace muui
{
ShaderEffect::ShaderEffect(int padding)
    : m_padding{padding}
{
}

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
            const auto w = static_cast<int>(std::ceil(m_source->width())) + 2 * m_padding;
            const auto h = static_cast<int>(std::ceil(m_source->height())) + 2 * m_padding;
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

        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);

        m_painter.begin();
        m_source->doRender(&m_painter, glm::vec2{m_padding, m_padding}, 0);
        m_painter.end();
    }

    applyEffect(painter, pos, depth);
}

} // namespace muui
