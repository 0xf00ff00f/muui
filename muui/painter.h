#pragma once

#include "noncopyable.h"

#include "brush.h"
#include "font.h"
#include "transform.h"
#include "util.h"

#include <glm/glm.hpp>

#include <memory>
#include <stack>
#include <string>
#include <string_view>

namespace muui
{
class SpriteBatcher;
struct PackedPixmap;

class Painter : private NonCopyable
{
public:
    Painter();
    ~Painter();

    int windowWidth() const { return m_windowWidth; }
    int windowHeight() const { return m_windowHeight; }

    void setWindowSize(int width, int height);

    void begin();
    void end();

    const Transform &transform() const;

    void translate(const glm::vec2 &pos);
    void rotate(float angle);
    void scale(const glm::vec2 &v);

    void pushTransform();
    void popTransform();

    void setFont(Font *font);
    Font *font() const { return m_font; }

    void setBackgroundBrush(const std::optional<Brush> &brush);
    std::optional<Brush> backgroundBrush() const { return m_backgroundBrush; }

    void setForegroundBrush(const std::optional<Brush> &brush);
    std::optional<Brush> foregroundBrush() const { return m_foregroundBrush; }

    void setOutlineBrush(const std::optional<Brush> &brush);
    std::optional<Brush> outlineBrush() const { return m_outlineBrush; }

    void setClipRect(const std::optional<RectF> &rect);
    std::optional<RectF> clipRect() const { return m_clipRect; }

    SpriteBatcher *spriteBatcher() const { return m_spriteBatcher.get(); }

    void drawRect(const RectF &rect, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, int depth);
    void drawText(std::u32string_view text, const glm::vec2 &pos, int depth);
    void drawGlyph(const Font::Glyph *glyph, const glm::vec2 &pos, bool outline, int depth);
    void drawCircle(const glm::vec2 &center, float radius, int depth);
    void drawCapsule(const RectF &rect, int depth);
    void drawRoundedRect(const RectF &rect, float cornerRadius, int depth);

private:
    struct Vertex
    {
        glm::vec2 position;
    };

    struct VertexUV
    {
        glm::vec2 position;
        glm::vec2 texCoord;
    };

    void drawText(std::u32string_view text, const glm::vec2 &pos, bool outline, int depth);
    void setRectProgram(const Color &color);
    void setRectProgram(const LinearGradient &gradient);

    void setDecalProgram(const Color &color);
    void setDecalProgram(const LinearGradient &gradient);

    void setTextProgram(const Color &color, bool outline);
    void setTextProgram(const LinearGradient &gradient, bool outline);

    void setCircleProgram(const Color &color);
    void setCircleProgram(const LinearGradient &gradient);

    void setRoundedRectProgram(const Color &color);
    void setRoundedRectProgram(const LinearGradient &gradient);

    template<typename VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, const Color &color, int depth);

    template<typename VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, const LinearGradient &gradient, int depth);

    template<typename VertexT>
    void addRoundedRectSprite(const VertexT &topLeft, const VertexT &bottomRight, const Color &color,
                              const glm::vec2 &size, float radius, int depth);

    template<typename VertexT>
    void addRoundedRectSprite(const VertexT &topLeft, const VertexT &bottomRight, const LinearGradient &gradient,
                              const glm::vec2 &size, float radius, int depth);

    void addSprite(const Vertex &topLeft, const Vertex &bottomRight, const glm::vec4 &fgColor, const glm::vec4 &bgColor,
                   int depth);
    void addSprite(const VertexUV &topLeft, const VertexUV &bottomRight, const glm::vec4 &fgColor,
                   const glm::vec4 &bgColor, int depth);

    void render();
    void updateTransformMatrix();

    int m_windowWidth{0};
    int m_windowHeight{0};
    std::unique_ptr<SpriteBatcher> m_spriteBatcher;
    Font *m_font{nullptr};
    std::optional<Brush> m_backgroundBrush; // rect, capsule, circle
    std::optional<Brush> m_foregroundBrush; // pixmap, text
    std::optional<Brush> m_outlineBrush;    // text outline
    std::optional<RectF> m_clipRect;
    bool m_clippingEnabled{false};
    struct TransformClipRect
    {
        Transform transform;
        std::optional<RectF> clipRect;
    };
    std::stack<TransformClipRect> m_transformStack;
};

class PainterBrushSaver
{
public:
    explicit PainterBrushSaver(Painter *painter)
        : m_painter(painter)
        , m_backgroundBrush(painter->backgroundBrush())
        , m_foregroundBrush(painter->foregroundBrush())
        , m_outlineBrush(painter->outlineBrush())
    {
    }

    ~PainterBrushSaver()
    {
        m_painter->setBackgroundBrush(m_backgroundBrush);
        m_painter->setForegroundBrush(m_foregroundBrush);
        m_painter->setOutlineBrush(m_outlineBrush);
    }

private:
    Painter *m_painter;
    std::optional<Brush> m_backgroundBrush;
    std::optional<Brush> m_foregroundBrush;
    std::optional<Brush> m_outlineBrush;
};

} // namespace muui
