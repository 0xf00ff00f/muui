#pragma once

#include "noncopyable.h"

#include "util.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

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

    void drawRect(const RectF &rect, const glm::vec4 &color, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &color, int depth);
    void drawPixmap(const PackedPixmap &pixmap, const RectF &rect, const RectF &clipRect, const glm::vec4 &color,
                    int depth);
    void drawText(std::u32string_view text, const glm::vec2 &pos, const glm::vec4 &color, int depth);
    void drawCircle(const glm::vec2 &center, float radius, const glm::vec4 &color, int depth);
    void drawCapsule(const RectF &rect, const glm::vec4 &color, int depth);
    void drawRoundedRect(const RectF &rect, float cornerRadius, const glm::vec4 &color, int depth);

private:
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
