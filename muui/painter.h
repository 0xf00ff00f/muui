#pragma once

#include "noncopyable.h"

#include "brush.h"
#include "font.h"
#include "util.h"

#include <glm/glm.hpp>

#include <memory>
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

    void setFont(Font *font);
    Font *font() const { return m_font; }

    void setBackgroundBrush(const std::optional<Brush> &brush);
    std::optional<Brush> backgroundBrush() const { return m_backgroundBrush; }

    void setForegroundBrush(const std::optional<Brush> &brush);
    std::optional<Brush> foregroundBrush() const { return m_foregroundBrush; }

    void setOutlineBrush(const std::optional<Brush> &brush);
    std::optional<Brush> outlineBrush() const { return m_outlineBrush; }

    void setClipRect(const RectF &rect);
    RectF clipRect() const { return m_clipRect; }

    SpriteBatcher *spriteBatcher() const { return m_spriteBatcher.get(); }

    void drawRect(const RectF &rect, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const RectF &clipRect, int depth);
    void drawText(std::u32string_view text, const glm::vec2 &pos, int depth);
    void drawGlyph(const Font::Glyph *glyph, const glm::vec2 &pos, bool outline, int depth);
    void drawGlyph(const Font::Glyph *glyph, const glm::vec2 &pos, const RectF &clipRect, bool outline, int depth);
    void drawCircle(const glm::vec2 &center, float radius, int depth);
    void drawCapsule(const RectF &rect, int depth);
    void drawRoundedRect(const RectF &rect, float cornerRadius, int depth);

private:
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

    void render();
    void updateTransformMatrix();

    int m_windowWidth{0};
    int m_windowHeight{0};
    std::unique_ptr<SpriteBatcher> m_spriteBatcher;
    Font *m_font{nullptr};
    std::optional<Brush> m_backgroundBrush; // rect, capsule, circle
    std::optional<Brush> m_foregroundBrush; // pixmap, text
    std::optional<Brush> m_outlineBrush;    // text outline
    RectF m_clipRect;
    bool m_clippingEnabled{false};
};

} // namespace muui
