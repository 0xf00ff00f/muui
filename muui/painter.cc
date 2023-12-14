#include "painter.h"

#include "font.h"
#include "spritebatcher.h"

#include "gl.h"

#include <glm/gtc/matrix_transform.hpp>

namespace muui
{

Painter::Painter()
    : m_spriteBatcher(std::make_unique<SpriteBatcher>())
{
    updateTransformMatrix();
}

Painter::~Painter() = default;

void Painter::setWindowSize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
    updateTransformMatrix();
}

void Painter::updateTransformMatrix()
{
    const auto mvp = glm::ortho(0.0f, static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), 0.0f);
    m_spriteBatcher->setTransformMatrix(mvp);
}

void Painter::begin()
{
    m_font = nullptr;
    m_spriteBatcher->begin();
    setClipRect({{0, 0}, {m_windowWidth, m_windowHeight}});
}

void Painter::end()
{
    m_spriteBatcher->flush();
}

void Painter::setFont(Font *font)
{
    m_font = font;
}

void Painter::setClipRect(const RectF &rect)
{
    m_clipRect = rect;
    const auto x = static_cast<GLint>(rect.min.x);
    const auto y = static_cast<GLint>(rect.min.y);
    const auto w = static_cast<GLint>(rect.width());
    const auto h = static_cast<GLint>(rect.height());
    m_spriteBatcher->setBatchScissorBox({.position = {x, m_windowHeight - (y + h)}, .size = {w, h}});
}

void Painter::drawRect(const RectF &rect, const glm::vec4 &color, int depth)
{
    if (m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::ProgramFlat);
        struct Vertex
        {
            glm::vec2 position;
        };
        const auto topLeftVertex = Vertex{.position = rect.min};
        const auto bottomRightVertex = Vertex{.position = rect.max};
        m_spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, color, depth);
    }
}

void Painter::drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &color, int depth)
{
    if (m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::ProgramDecal);
        struct Vertex
        {
            glm::vec2 position;
            glm::vec2 texCoord;
        };
        const Vertex topLeftVertex = {.position = rect.min, .texCoord = pixmap.texCoord.min};
        const Vertex bottomRightVertex = {.position = rect.max, .texCoord = pixmap.texCoord.max};
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        m_spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, color, depth);
    }
}

void Painter::drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const RectF &clipRect, const glm::vec4 &color,
                         int depth)
{
    if (m_clipRect.intersects(rect) && m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::ProgramDecal);
        const auto texPos = [&rect, &texCoord = pixmap.texCoord](const glm::vec2 &p) {
            const float x =
                (p.x - rect.min.x) * (texCoord.max.x - texCoord.min.x) / (rect.max.x - rect.min.x) + texCoord.min.x;
            const float y =
                (p.y - rect.min.y) * (texCoord.max.y - texCoord.min.y) / (rect.max.y - rect.min.y) + texCoord.min.y;
            return glm::vec2(x, y);
        };
        const auto spriteRect = rect.intersected(clipRect);
        struct Vertex
        {
            glm::vec2 position;
            glm::vec2 texCoord;
        };
        const Vertex topLeftVertex = {.position = spriteRect.min, .texCoord = texPos(spriteRect.min)};
        const Vertex bottomRightVertex = {.position = spriteRect.max, .texCoord = texPos(spriteRect.max)};
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        m_spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, color, depth);
    }
}

void Painter::drawText(std::u32string_view text, const glm::vec2 &pos, const glm::vec4 &color, int depth)
{
    assert(m_font);
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramText);

    auto basePos = glm::vec2(pos.x, pos.y + m_font->ascent());
    for (auto ch : text)
    {
        if (const auto *g = m_font->glyph(ch); g)
        {
            const auto topLeft = basePos + glm::vec2(g->boundingBox.min);
            const auto bottomRight = topLeft + glm::vec2(g->boundingBox.max - g->boundingBox.min);
            const auto rect = RectF{topLeft, bottomRight};
            if (m_clipRect.intersects(rect))
            {
                const auto &pixmap = g->pixmap;
                m_spriteBatcher->setBatchTexture(pixmap.texture);
                struct Vertex
                {
                    glm::vec2 position;
                    glm::vec2 texCoord;
                };
                const auto topLeftVertex = Vertex{.position = topLeft, .texCoord = pixmap.texCoord.min};
                const auto bottomRightVertex = Vertex{.position = bottomRight, .texCoord = pixmap.texCoord.max};
                m_spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, color, depth);
            }
            basePos.x += g->advanceWidth;
        }
    }
}

void Painter::drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int depth)
{
    const auto topLeft = center - glm::vec2(radius, radius);
    const auto bottomRight = center + glm::vec2(radius, radius);
    const auto rect = RectF{topLeft, bottomRight};
    if (m_clipRect.intersects(rect))
    {
        m_spriteBatcher->setBatchProgram(ShaderManager::ProgramCircle);
        struct Vertex
        {
            glm::vec2 position;
            glm::vec2 texCoord;
        };
        const auto topLeftVertex = Vertex{.position = topLeft, .texCoord = {0, 0}};
        const auto bottomRightVertex = Vertex{.position = bottomRight, .texCoord = {1, 1}};
        m_spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, color, depth);
    }
}

void Painter::drawCapsule(const RectF &rect, const glm::vec4 &color, int depth)
{
    const auto cornerRadius = std::min(0.5f * rect.height(), 0.5f * rect.width());
    drawRoundedRect(rect, cornerRadius, color, depth);
}

void Painter::drawRoundedRect(const RectF &rect, float cornerRadius, const glm::vec4 &color, int depth)
{
    if (!m_clipRect.intersects(rect))
        return;
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramRoundedRect);
    const auto radius = std::min(std::min(cornerRadius, 0.5f * rect.width()), 0.5f * rect.height());
    const glm::vec2 size(rect.width(), rect.height());
    const RectF texCoords{-0.5f * size, 0.5f * size};
    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
    };
    const auto topLeft = Vertex{.position = rect.min, .texCoord = -0.5f * size};
    const auto bottomRight = Vertex{.position = rect.max, .texCoord = 0.5f * size};
    m_spriteBatcher->addSprite(topLeft, bottomRight, color, glm::vec4(size, radius, 0), depth);
}

} // namespace muui
