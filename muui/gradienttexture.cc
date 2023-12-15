#include "gradienttexture.h"

#include <algorithm>

namespace muui
{

constexpr auto TextureSize = 1024;

GradientTexture::GradientTexture()
    : m_texture(TextureSize, 1, PixelType::RGBA)
{
    m_texture.setMinificationFilter(gl::Texture::Filter::Linear);
    m_texture.setMagnificationFilter(gl::Texture::Filter::Linear);
    m_texture.setWrapModeS(gl::Texture::WrapMode::ClampToEdge);
    m_texture.setWrapModeT(gl::Texture::WrapMode::ClampToEdge);
}

void GradientTexture::bind(int textureUnit) const
{
    if (m_dirty)
    {
        updateTextureData();
        m_dirty = false;
    }
    m_texture.bind(textureUnit);
}

void GradientTexture::setColorAt(float position, const glm::vec4 &color)
{
    auto it = std::upper_bound(m_stops.begin(), m_stops.end(), position,
                               [](float position, const GradientStop &stop) { return position < stop.position; });
    m_stops.insert(it, GradientStop{.position = position, .color = color});
    m_dirty = true;
}

void GradientTexture::setStops(const std::vector<GradientStop> &stops)
{
    m_stops = stops;
    std::stable_sort(m_stops.begin(), m_stops.end(),
                     [](const GradientStop &a, const GradientStop &b) { return a.position < b.position; });
    m_dirty = true;
}

void GradientTexture::updateTextureData() const
{
    static_assert(sizeof(glm::u8vec4) == pixelSizeInBytes(PixelType::RGBA));
    std::vector<glm::u8vec4> pixels(TextureSize);

    int curStop = 0;
    for (int i = 0; i < TextureSize; ++i)
    {
        const glm::vec4 color = [this, &curStop, i]() -> glm::vec4 {
            if (m_stops.empty())
                return {};

            if (m_stops.size() == 1)
                return m_stops.front().color;

            const auto position = static_cast<float>(i) / (TextureSize - 1);
            if (curStop == 0 && position < m_stops[curStop].position)
                return m_stops[curStop].color;

            assert(m_stops.size() >= 2);
            assert(curStop < m_stops.size() - 1);
            while (curStop < m_stops.size() - 1 && m_stops[curStop + 1].position < position)
                ++curStop;

            if (curStop == m_stops.size() - 1)
                return m_stops[curStop].color;

            assert(m_stops.size() >= 2);
            assert(curStop < m_stops.size() - 1);
            assert(position >= m_stops[curStop].position && position <= m_stops[curStop + 1].position);
            const float t =
                (position - m_stops[curStop].position) / (m_stops[curStop + 1].position - m_stops[curStop].position);
            return glm::mix(m_stops[curStop].color, m_stops[curStop + 1].color, t);
        }();
        pixels[i] = glm::u8vec4(color * 255.0f);
    }

    m_texture.setData(reinterpret_cast<const unsigned char *>(pixels.data()));
}

} // namespace muui
