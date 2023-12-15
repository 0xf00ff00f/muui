#pragma once

#include "abstracttexture.h"
#include "texture.h"

#include <glm/glm.hpp>

#include <vector>

namespace muui
{

struct GradientStop
{
    float position;
    glm::vec4 color;
};

class GradientTexture : public AbstractTexture
{
public:
    GradientTexture();

    void bind(int textureUnit = 0) const override;

    void setColorAt(float position, const glm::vec4 &color);
    void setStops(const std::vector<GradientStop> &stops);

private:
    void updateTextureData() const;

    gl::Texture m_texture;
    std::vector<GradientStop> m_stops;
    mutable bool m_dirty;
};

} // namespace muui
