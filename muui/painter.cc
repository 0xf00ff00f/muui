#include "painter.h"

#include "font.h"
#include "gradienttexture.h"
#include "spritebatcher.h"

#include "gl.h"

#include <glm/gtc/matrix_transform.hpp>

namespace muui
{

namespace
{

struct Vertex
{
    glm::vec2 position;
};

struct VertexUV
{
    glm::vec2 position;
    glm::vec2 texCoord;
};

} // namespace

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
    m_backgroundBrush.reset();
    m_foregroundBrush.reset();
    m_outlineBrush.reset();
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

void Painter::setBackgroundBrush(const std::optional<Brush> &brush)
{
    m_backgroundBrush = brush;
}

void Painter::setForegroundBrush(const std::optional<Brush> &brush)
{
    m_foregroundBrush = brush;
}

void Painter::setOutlineBrush(const std::optional<Brush> &brush)
{
    m_outlineBrush = brush;
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

void Painter::drawRect(const RectF &rect, int depth)
{
    assert(m_backgroundBrush);
    if (m_clipRect.intersects(rect))
    {
        std::visit([this](auto &brush) { setRectProgram(brush); }, *m_backgroundBrush);
        const auto topLeftVertex = Vertex{.position = rect.min};
        const auto bottomRightVertex = Vertex{.position = rect.max};
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   *m_backgroundBrush);
    }
}

void Painter::drawPixmap(const PackedPixmap &pixmap, const RectF &rect, int depth)
{
    assert(m_foregroundBrush);
    if (m_clipRect.intersects(rect))
    {
        std::visit([this](auto &brush) { setDecalProgram(brush); }, *m_foregroundBrush);
        struct Vertex
        {
            glm::vec2 position;
            glm::vec2 texCoord;
        };
        const Vertex topLeftVertex = {.position = rect.min, .texCoord = pixmap.texCoord.min};
        const Vertex bottomRightVertex = {.position = rect.max, .texCoord = pixmap.texCoord.max};
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   *m_foregroundBrush);
    }
}

void Painter::drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const RectF &clipRect, int depth)
{
    assert(m_foregroundBrush);
    if (m_clipRect.intersects(rect) && m_clipRect.intersects(rect))
    {
        std::visit([this](auto &brush) { setDecalProgram(brush); }, *m_foregroundBrush);
        const auto texPos = [&rect, &texCoord = pixmap.texCoord](const glm::vec2 &p) {
            const float x =
                (p.x - rect.min.x) * (texCoord.max.x - texCoord.min.x) / (rect.max.x - rect.min.x) + texCoord.min.x;
            const float y =
                (p.y - rect.min.y) * (texCoord.max.y - texCoord.min.y) / (rect.max.y - rect.min.y) + texCoord.min.y;
            return glm::vec2(x, y);
        };
        const auto spriteRect = rect.intersected(clipRect);
        const VertexUV topLeftVertex = {.position = spriteRect.min, .texCoord = texPos(spriteRect.min)};
        const VertexUV bottomRightVertex = {.position = spriteRect.max, .texCoord = texPos(spriteRect.max)};
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   *m_foregroundBrush);
    }
}

void Painter::drawText(std::u32string_view text, const glm::vec2 &pos, int depth)
{
    assert(m_font);
    if (m_font->outlineSize() > 0)
    {
        drawText(text, pos, true, depth);
        drawText(text, pos, false, depth + 1);
    }
    else
    {
        drawText(text, pos, false, depth);
    }
}

void Painter::drawText(std::u32string_view text, const glm::vec2 &pos, bool outline, int depth)
{
    assert(m_font);
    auto basePos = glm::vec2(pos.x, pos.y + m_font->ascent());
    for (auto ch : text)
    {
        if (const auto *g = m_font->glyph(ch))
        {
            drawGlyph(g, basePos, outline, depth);
            basePos.x += g->advanceWidth;
        }
    }
}

void Painter::drawGlyph(const Font::Glyph *glyph, const glm::vec2 &pos, bool outline, int depth)
{
    const Brush brush = [this, outline]() -> Brush {
        if (outline)
        {
            assert(m_outlineBrush);
            return *m_outlineBrush;
        }
        else
        {
            assert(m_foregroundBrush);
            return *m_foregroundBrush;
        }
    }();
    const auto topLeft = pos + glm::vec2(glyph->boundingBox.min);
    const auto bottomRight = topLeft + glm::vec2(glyph->boundingBox.max - glyph->boundingBox.min);
    const auto rect = RectF{topLeft, bottomRight};
    if (m_clipRect.intersects(rect))
    {
        std::visit([this, outline](auto &brush) { setTextProgram(brush, outline); }, brush);
        const auto &pixmap = glyph->pixmap;
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        const auto topLeftVertex = VertexUV{.position = topLeft, .texCoord = pixmap.texCoord.min};
        const auto bottomRightVertex = VertexUV{.position = bottomRight, .texCoord = pixmap.texCoord.max};
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](const auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   brush);
    }
}

void Painter::drawGlyph(const Font::Glyph *glyph, const glm::vec2 &pos, const RectF &clipRect, bool outline, int depth)
{
    const Brush brush = [this, outline]() -> Brush {
        if (outline)
        {
            assert(m_outlineBrush);
            return *m_outlineBrush;
        }
        else
        {
            assert(m_foregroundBrush);
            return *m_foregroundBrush;
        }
    }();
    const auto topLeft = pos + glm::vec2(glyph->boundingBox.min);
    const auto bottomRight = topLeft + glm::vec2(glyph->boundingBox.max - glyph->boundingBox.min);
    const auto rect = RectF{topLeft, bottomRight};
    if (m_clipRect.intersects(rect))
    {
        std::visit([this, outline](auto &brush) { setTextProgram(brush, outline); }, brush);
        const auto &pixmap = glyph->pixmap;
        const auto texPos = [&rect, &texCoord = pixmap.texCoord](const glm::vec2 &p) {
            const float x =
                (p.x - rect.min.x) * (texCoord.max.x - texCoord.min.x) / (rect.max.x - rect.min.x) + texCoord.min.x;
            const float y =
                (p.y - rect.min.y) * (texCoord.max.y - texCoord.min.y) / (rect.max.y - rect.min.y) + texCoord.min.y;
            return glm::vec2(x, y);
        };
        const auto spriteRect = rect.intersected(clipRect);
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        const auto topLeftVertex = VertexUV{.position = spriteRect.min, .texCoord = texPos(spriteRect.min)};
        const auto bottomRightVertex = VertexUV{.position = spriteRect.max, .texCoord = texPos(spriteRect.max)};
        std::visit([this, &topLeftVertex, bottomRightVertex,
                    depth](const auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   brush);
    }
}

void Painter::drawCircle(const glm::vec2 &center, float radius, int depth)
{
    assert(m_backgroundBrush);
    const auto topLeft = center - glm::vec2(radius, radius);
    const auto bottomRight = center + glm::vec2(radius, radius);
    const auto rect = RectF{topLeft, bottomRight};
    if (m_clipRect.intersects(rect))
    {
        std::visit([this](auto &brush) { setCircleProgram(brush); }, *m_backgroundBrush);
        const auto topLeftVertex = VertexUV{.position = topLeft, .texCoord = {0, 0}};
        const auto bottomRightVertex = VertexUV{.position = bottomRight, .texCoord = {1, 1}};
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](const auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   *m_backgroundBrush);
    }
}

void Painter::drawCapsule(const RectF &rect, int depth)
{
    const auto cornerRadius = std::min(0.5f * rect.height(), 0.5f * rect.width());
    drawRoundedRect(rect, cornerRadius, depth);
}

void Painter::drawRoundedRect(const RectF &rect, float cornerRadius, int depth)
{
    if (!m_clipRect.intersects(rect))
        return;
    assert(m_backgroundBrush);
    std::visit([this](auto &brush) { setRoundedRectProgram(brush); }, *m_backgroundBrush);
    const auto radius = std::min(std::min(cornerRadius, 0.5f * rect.width()), 0.5f * rect.height());
    const glm::vec2 size(rect.width(), rect.height());
    const RectF texCoords{-0.5f * size, 0.5f * size};
    const auto topLeft = VertexUV{.position = rect.min, .texCoord = -0.5f * size};
    const auto bottomRight = VertexUV{.position = rect.max, .texCoord = 0.5f * size};
    std::visit([this, &topLeft, &bottomRight, &size, cornerRadius, depth](
                   const auto &brush) { addRoundedRectSprite(topLeft, bottomRight, brush, size, cornerRadius, depth); },
               *m_backgroundBrush);
}

void Painter::setRectProgram(const Color &)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::Flat);
}

void Painter::setRectProgram(const LinearGradient &gradient)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::Gradient);
    m_spriteBatcher->setBatchGradientTexture(gradient.texture);
}

void Painter::setDecalProgram(const Color &)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::Decal);
}

void Painter::setDecalProgram(const LinearGradient &gradient)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::DecalGradient);
    m_spriteBatcher->setBatchGradientTexture(gradient.texture);
}

void Painter::setTextProgram(const Color &, bool outline)
{
    const auto program = outline ? ShaderManager::ProgramHandle::TextOutline : ShaderManager::ProgramHandle::Text;
    m_spriteBatcher->setBatchProgram(program);
}

void Painter::setTextProgram(const LinearGradient &gradient, bool outline)
{
    const auto program =
        outline ? ShaderManager::ProgramHandle::TextGradientOutline : ShaderManager::ProgramHandle::TextGradient;
    m_spriteBatcher->setBatchProgram(program);
    m_spriteBatcher->setBatchGradientTexture(gradient.texture);
}

void Painter::setCircleProgram(const Color &)
{
    const auto program = ShaderManager::ProgramHandle::Text;
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::Circle);
}

void Painter::setCircleProgram(const LinearGradient &gradient)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::CircleGradient);
    m_spriteBatcher->setBatchGradientTexture(gradient.texture);
}

void Painter::setRoundedRectProgram(const Color &)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::RoundedRect);
}

void Painter::setRoundedRectProgram(const LinearGradient &gradient)
{
    m_spriteBatcher->setBatchProgram(ShaderManager::ProgramHandle::RoundedRectGradient);
    m_spriteBatcher->setBatchGradientTexture(gradient.texture);
}

template<typename VertexT>
void Painter::addSprite(const VertexT &topLeft, const VertexT &bottomRight, const Color &color, int depth)
{
    m_spriteBatcher->addSprite(topLeft, bottomRight, color, depth);
}

template<typename VertexT>
void Painter::addSprite(const VertexT &topLeft, const VertexT &bottomRight, const LinearGradient &gradient, int depth)
{
    m_spriteBatcher->addSprite(topLeft, bottomRight,
                               glm::vec4(gradient.start.x, gradient.start.y, gradient.end.x, gradient.end.y), depth);
}

template<typename VertexT>
void Painter::addRoundedRectSprite(const VertexT &topLeft, const VertexT &bottomRight, const Color &color,
                                   const glm::vec2 &size, float radius, int depth)
{
    m_spriteBatcher->addSprite(topLeft, bottomRight, color, glm::vec4(size, radius, 0), depth);
}

template<typename VertexT>
void Painter::addRoundedRectSprite(const VertexT &topLeft, const VertexT &bottomRight, const LinearGradient &gradient,
                                   const glm::vec2 &size, float radius, int depth)
{
    m_spriteBatcher->addSprite(topLeft, bottomRight,
                               glm::vec4(gradient.start.x, gradient.start.y, gradient.end.x, gradient.end.y),
                               glm::vec4(size, radius, 0), depth);
}

} // namespace muui
