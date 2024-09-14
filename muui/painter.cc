#include "painter.h"

#include "font.h"
#include "gradienttexture.h"
#include "spritebatcher.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

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
    m_spriteBatcher->setMvp(mvp);
}

void Painter::begin()
{
    m_font = nullptr;
    m_backgroundBrush.reset();
    m_foregroundBrush.reset();
    m_outlineBrush.reset();
    m_spriteBatcher->begin();
    m_clipRect.reset();
}

void Painter::end()
{
    m_spriteBatcher->flush();
}

void Painter::pushTransform()
{
    m_transformStack.push({m_spriteBatcher->transform(), m_clipRect});
}

void Painter::popTransform()
{
    const auto &[transform, clipRect] = m_transformStack.top();
    m_spriteBatcher->setTransform(transform);
    m_clipRect = clipRect;
    m_transformStack.pop();
}

const Transform &Painter::transform() const
{
    return m_spriteBatcher->transform();
}

void Painter::translate(const glm::vec2 &pos)
{
    if (pos == glm::vec2(0.0f))
        return;
    if (m_clipRect)
    {
        m_clipRect->min -= pos;
        m_clipRect->max -= pos;
    }
    m_spriteBatcher->translate(pos);
}

void Painter::rotate(float angle)
{
    if (angle == 0.0f)
        return;
    if (m_clipRect)
    {
        // TODO: ???
        assert(0);
    }
    m_spriteBatcher->rotate(angle);
}

void Painter::scale(const glm::vec2 &v)
{
    if (v == glm::vec2(1.0f))
        return;
    if (m_clipRect)
    {
        // TODO: ???
        assert(0);
    }
    m_spriteBatcher->scale(v);
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

void Painter::setClipRect(const std::optional<RectF> &rect)
{
    m_clipRect = rect;
}

void Painter::drawRect(const RectF &rect, int depth)
{
    if (!m_clipRect || m_clipRect->intersects(rect))
    {
        assert(m_backgroundBrush);
        std::visit([this](const auto &brush) { setRectProgram(brush); }, *m_backgroundBrush);
        const auto topLeftVertex = Vertex{.position = rect.min};
        const auto bottomRightVertex = Vertex{.position = rect.max};
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](const auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   *m_backgroundBrush);
    }
}

void Painter::drawPixmap(const PackedPixmap &pixmap, const RectF &rect, int depth)
{
    if (!m_clipRect || m_clipRect->intersects(rect))
    {
        assert(m_foregroundBrush);
        std::visit([this](const auto &brush) { setDecalProgram(brush); }, *m_foregroundBrush);
        const VertexUV topLeftVertex = {.position = rect.min, .texCoord = pixmap.texCoord.min};
        const VertexUV bottomRightVertex = {.position = rect.max, .texCoord = pixmap.texCoord.max};
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](const auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
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
    const auto topLeft = pos + glm::vec2(glyph->boundingBox.min);
    const auto bottomRight = topLeft + glm::vec2(glyph->boundingBox.max - glyph->boundingBox.min);
    const auto rect = RectF{topLeft, bottomRight};
    if (!m_clipRect || m_clipRect->intersects(rect))
    {
        const auto &brush = outline ? m_outlineBrush : m_foregroundBrush;
        assert(brush);
        std::visit([this, outline](const auto &brush) { setTextProgram(brush, outline); }, *brush);
        const auto &pixmap = glyph->pixmap;
        m_spriteBatcher->setBatchTexture(pixmap.texture);
        const auto topLeftVertex = VertexUV{.position = topLeft, .texCoord = pixmap.texCoord.min};
        const auto bottomRightVertex = VertexUV{.position = bottomRight, .texCoord = pixmap.texCoord.max};
        std::visit([this, &topLeftVertex, &bottomRightVertex,
                    depth](const auto &brush) { addSprite(topLeftVertex, bottomRightVertex, brush, depth); },
                   *brush);
    }
}

void Painter::drawCircle(const glm::vec2 &center, float radius, int depth)
{
    const auto topLeft = center - glm::vec2(radius, radius);
    const auto bottomRight = center + glm::vec2(radius, radius);
    const auto rect = RectF{topLeft, bottomRight};
    if (!m_clipRect || m_clipRect->intersects(rect))
    {
        assert(m_backgroundBrush);
        std::visit([this](const auto &brush) { setCircleProgram(brush); }, *m_backgroundBrush);
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
    if (!m_clipRect || m_clipRect->intersects(rect))
    {
        assert(m_backgroundBrush);
        std::visit([this](const auto &brush) { setRoundedRectProgram(brush); }, *m_backgroundBrush);
        const auto radius = std::min(std::min(cornerRadius, 0.5f * rect.width()), 0.5f * rect.height());
        const glm::vec2 size(rect.width(), rect.height());
        const RectF texCoords{-0.5f * size, 0.5f * size};
        const auto topLeft = VertexUV{.position = rect.min, .texCoord = -0.5f * size};
        const auto bottomRight = VertexUV{.position = rect.max, .texCoord = 0.5f * size};
        std::visit(
            [this, &topLeft, &bottomRight, &size, cornerRadius, depth](const auto &brush) {
                addRoundedRectSprite(topLeft, bottomRight, brush, size, cornerRadius, depth);
            },
            *m_backgroundBrush);
    }
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
    addSprite(topLeft, bottomRight, color, {}, depth);
}

template<typename VertexT>
void Painter::addSprite(const VertexT &topLeft, const VertexT &bottomRight, const LinearGradient &gradient, int depth)
{
    addSprite(topLeft, bottomRight, glm::vec4(gradient.start.x, gradient.start.y, gradient.end.x, gradient.end.y), {},
              depth);
}

template<typename VertexT>
void Painter::addRoundedRectSprite(const VertexT &topLeft, const VertexT &bottomRight, const Color &color,
                                   const glm::vec2 &size, float radius, int depth)
{
    addSprite(topLeft, bottomRight, color, glm::vec4(size, radius, 0), depth);
}

template<typename VertexT>
void Painter::addRoundedRectSprite(const VertexT &topLeft, const VertexT &bottomRight, const LinearGradient &gradient,
                                   const glm::vec2 &size, float radius, int depth)
{
    addSprite(topLeft, bottomRight, glm::vec4(gradient.start.x, gradient.start.y, gradient.end.x, gradient.end.y),
              glm::vec4(size, radius, 0), depth);
}

void Painter::addSprite(const Vertex &topLeft, const Vertex &bottomRight, const glm::vec4 &fgColor,
                        const glm::vec4 &bgColor, int depth)
{
    const auto rect = RectF{topLeft.position, bottomRight.position};
    if (!m_clipRect || m_clipRect->contains(rect))
    {
        m_spriteBatcher->addSprite(topLeft, bottomRight, fgColor, bgColor, depth);
    }
    else
    {
        assert(m_clipRect.has_value());
        const auto clippedRect = rect.intersected(*m_clipRect);
        if (!clippedRect.isNull())
            m_spriteBatcher->addSprite(Vertex{clippedRect.min}, Vertex{clippedRect.max}, fgColor, bgColor, depth);
    }
}

void Painter::addSprite(const VertexUV &topLeft, const VertexUV &bottomRight, const glm::vec4 &fgColor,
                        const glm::vec4 &bgColor, int depth)
{
    const auto rect = RectF{topLeft.position, bottomRight.position};
    if (!m_clipRect || m_clipRect->contains(rect))
    {
        m_spriteBatcher->addSprite(topLeft, bottomRight, fgColor, bgColor, depth);
    }
    else
    {
        assert(m_clipRect.has_value());
        const auto clippedRect = rect.intersected(*m_clipRect);
        if (!clippedRect.isNull())
        {
            const auto texCoord = [&topLeft, &bottomRight](const glm::vec2 &p) {
                const float x = (p.x - topLeft.position.x) * (bottomRight.texCoord.x - topLeft.texCoord.x) /
                                    (bottomRight.position.x - topLeft.position.x) +
                                topLeft.texCoord.x;
                const float y = (p.y - topLeft.position.y) * (bottomRight.texCoord.y - topLeft.texCoord.y) /
                                    (bottomRight.position.y - topLeft.position.y) +
                                topLeft.texCoord.y;
                return glm::vec2(x, y);
            };
            m_spriteBatcher->addSprite(VertexUV{clippedRect.min, texCoord(clippedRect.min)},
                                       VertexUV{clippedRect.max, texCoord(clippedRect.max)}, fgColor, bgColor, depth);
        }
    }
}

} // namespace muui
