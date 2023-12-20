#pragma once

#include "noncopyable.h"

#include "brush.h"
#include "util.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace muui
{
class Font;
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

    void setClipRect(const RectF &rect);
    RectF clipRect() const { return m_clipRect; }

    SpriteBatcher *spriteBatcher() const { return m_spriteBatcher.get(); }

    void drawRect(const RectF &rect, const Brush &brush, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const Brush &color, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const RectF &clipRect, const Brush &color,
                    int depth);
    void drawText(std::u32string_view text, const glm::vec2 &pos, const Brush &brush, int depth);
    void drawCircle(const glm::vec2 &center, float radius, const Brush &brush, int depth);
    void drawCapsule(const RectF &rect, const Brush &brush, int depth);
    void drawRoundedRect(const RectF &rect, float cornerRadius, const Brush &brush, int depth);

private:
    void setRectProgram(const Color &color);
    void setRectProgram(const LinearGradient &gradient);

    void setDecalProgram(const Color &color);
    void setDecalProgram(const LinearGradient &gradient);

    void setTextProgram(const Color &color);
    void setTextProgram(const LinearGradient &gradient);

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

    int m_windowWidth = 0;
    int m_windowHeight = 0;
    std::unique_ptr<SpriteBatcher> m_spriteBatcher;
    Font *m_font = nullptr;
    RectF m_clipRect;
    bool m_clippingEnabled = false;
};

} // namespace muui
