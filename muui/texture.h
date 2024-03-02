#pragma once

#include "abstracttexture.h"
#include "pixeltype.h"

#include "gl.h"

#include <memory>
#include <string>

namespace muui
{
struct Pixmap;
}

namespace muui::gl
{

class Texture : public AbstractTexture
{
public:
    enum class Target
    {
        Texture2D = GL_TEXTURE_2D,
        TextureCubeMap = GL_TEXTURE_CUBE_MAP,
    };

    explicit Texture(const Pixmap &pixmap, Target target = Target::Texture2D);
    Texture(int width, int height, PixelType pixelType, const unsigned char *data = nullptr,
            Target target = Target::Texture2D);
    ~Texture() override;

    Texture(Texture &) = delete;
    Texture &operator=(Texture &) = delete;

    Texture(Texture &&other);
    Texture &operator=(Texture &&other);

    enum class Filter
    {
        Linear = GL_LINEAR,
        Nearest = GL_NEAREST
    };
    void setMinificationFilter(Filter filter);
    void setMagnificationFilter(Filter filter);

    enum class WrapMode
    {
        Repeat = GL_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE
    };
    void setWrapModeS(WrapMode mode);
    void setWrapModeT(WrapMode mode);
    void setWrapModeR(WrapMode mode);

    void setData(const unsigned char *data, std::size_t faceIndex = 0) const;

    int width() const { return m_width; }
    int height() const { return m_height; }

    void bind(int textureUnit = 0) const override;

    GLuint id() const { return m_id; }

    explicit operator bool() const { return m_id != 0; }

private:
    void initialize();
    GLenum faceTarget(std::size_t faceIndex = 0) const;
    void allocateTextureData(std::size_t faceIndex = 0) const;
    int m_width{0};
    int m_height{0};
    PixelType m_pixelType{PixelType::Invalid};
    Target m_target{Target::Texture2D};
    GLuint m_id{0};
};

} // namespace muui::gl
