#pragma once

#include "mesh.h"
#include "shadereffect.h"

namespace muui
{

class DropShadow : public ShaderEffect
{
public:
    DropShadow();

    glm::vec2 offset = glm::vec2(0);
    glm::vec4 color = glm::vec4(0);

protected:
    void applyEffect(Painter *painter, int depth) override;

private:
    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
    };
    std::array<std::unique_ptr<gl::Framebuffer>, 2> m_pingPongBuffers;
    gl::Mesh<Vertex> m_quad;
};

} // namespace muui
