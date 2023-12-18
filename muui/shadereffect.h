#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "painter.h"

namespace muui
{
class Item;

namespace gl
{
class Framebuffer;
}

class ShaderEffect
{
public:
    virtual ~ShaderEffect();

    void resize(int width, int height);

    void render(Item &item, Painter *painter, const glm::vec2 &pos, int depth);

    int width() const { return m_painter.windowWidth(); }
    int height() const { return m_painter.windowHeight(); }

protected:
    virtual void applyEffect(SpriteBatcher *spriteBatcher, const glm::vec2 &pos, int depth) = 0;

    std::unique_ptr<gl::Framebuffer> m_framebuffer;

private:
    Painter m_painter;
};

} // namespace muui
