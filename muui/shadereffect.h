#pragma once

#include "painter.h"

#include <muslots/muslots.h>

#include <glm/glm.hpp>

#include <memory>

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
    explicit ShaderEffect(int padding = 0);
    virtual ~ShaderEffect();

    Item *source() const { return m_source; }
    void setSource(Item *source);

    void render(Painter *painter, int depth);

    int width() const { return m_painter.windowWidth(); }
    int height() const { return m_painter.windowHeight(); }

protected:
    virtual void applyEffect(Painter *painter, int depth) = 0;

    int m_padding{0};
    std::unique_ptr<gl::Framebuffer> m_framebuffer;

private:
    Item *m_source{nullptr};
    muslots::Connection m_resizedConnection;
    Painter m_painter;
};

} // namespace muui
