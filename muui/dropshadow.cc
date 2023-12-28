#include "dropshadow.h"

#include "framebuffer.h"
#include "shadermanager.h"
#include "spritebatcher.h"
#include "system.h"

namespace muui
{

DropShadow::DropShadow()
    : ShaderEffect{8}
{
    static const std::array<Vertex, 4> verts = {
        Vertex{{-1, -1}, {0, 0}},
        Vertex{{-1, 1}, {0, 1}},
        Vertex{{1, -1}, {1, 0}},
        Vertex{{1, 1}, {1, 1}},
    };
    m_quad.setData(verts);
}

void DropShadow::applyEffect(Painter *painter, const glm::vec2 &pos, int depth)
{
    auto *shaderManager = getShaderManager();
    shaderManager->useProgram(ShaderManager::ProgramGaussianBlur);
    shaderManager->setUniform("baseColorTexture", 0);

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);

    auto applyBlur = [this, shaderManager](const gl::Texture *source, std::unique_ptr<gl::Framebuffer> &dest,
                                           bool horizontal) {
        const auto w = width();
        const auto h = height();
        if (!dest || dest->width() != w || dest->height() != h)
            dest = std::make_unique<gl::Framebuffer>(w, h);
        shaderManager->setUniform("horizontal", horizontal);
        source->bind(0);
        {
            gl::FramebufferBinder binder(*dest);
            m_quad.render(gl::Primitive::TriangleStrip);
        }
    };
    bool first = true;
    for (int i = 0; i < 4; ++i)
    {
        applyBlur(first ? m_framebuffer->texture() : m_pingPongBuffers[1]->texture(), m_pingPongBuffers[0], true);
        applyBlur(m_pingPongBuffers[0]->texture(), m_pingPongBuffers[1], false);
        first = false;
    }

    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);

    auto *spriteBatcher = painter->spriteBatcher();
    auto blitResult = [this, spriteBatcher, &pos](const gl::Texture *source, const glm::vec2 &offset,
                                                  const glm::vec4 &color, int depth) {
        struct Vertex
        {
            glm::vec2 position;
            glm::vec2 texCoord;
        };
        const auto size = glm::vec2{width(), height()};
        spriteBatcher->setBatchTexture(source);
        const Vertex topLeftVertex{.position = pos - glm::vec2(m_padding) + offset, .texCoord = {0, 1}};
        const Vertex bottomRightVertex{.position = topLeftVertex.position + size, .texCoord = {1, 0}};
        spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, color, depth);
    };

    spriteBatcher->setBatchProgram(ShaderManager::ProgramDecal);
    blitResult(m_pingPongBuffers[1]->texture(), offset, color, depth);

    spriteBatcher->setBatchProgram(ShaderManager::ProgramCopy);
    blitResult(m_framebuffer->texture(), glm::vec2(0), glm::vec4(1), depth + 1);
}

} // namespace muui
