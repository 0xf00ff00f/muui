#include "texture.h"
#include "pixmap.h"

#include <memory>

namespace muui::gl
{

namespace
{
constexpr GLenum Target = GL_TEXTURE_2D;

GLenum toGLFormat(PixelType pixelType)
{
    return pixelType == PixelType::RGBA ? GL_RGBA : GL_LUMINANCE;
}

GLenum toGLInternalFormat(PixelType pixelType)
{
    return pixelType == PixelType::RGBA ? GL_RGBA : GL_LUMINANCE;
}
} // namespace

Texture::Texture(const Pixmap &pixmap)
    : Texture(pixmap.width, pixmap.height, pixmap.pixelType, pixmap.pixels.data())
{
}

Texture::Texture(int width, int height, PixelType pixelType, const unsigned char *data)
    : m_width(width)
    , m_height(height)
    , m_pixelType(pixelType)
{
    glGenTextures(1, &m_id);
    initialize();
    if (data)
        setData(data);
}

Texture::~Texture()
{
    if (m_id)
        glDeleteTextures(1, &m_id);
}

Texture::Texture(Texture &&other)
    : m_width(other.m_width)
    , m_height(other.m_height)
    , m_pixelType(other.m_pixelType)
    , m_id(other.m_id)
{
    other.m_width = 0;
    other.m_height = 0;
    other.m_pixelType = PixelType::Invalid;
    other.m_id = 0;
}

Texture &Texture::operator=(Texture &&other)
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_pixelType = other.m_pixelType;
    m_id = other.m_id;

    other.m_width = 0;
    other.m_height = 0;
    other.m_pixelType = PixelType::Invalid;
    other.m_id = 0;

    return *this;
}

void Texture::initialize()
{
    bind();

    glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(Target, 0, toGLInternalFormat(m_pixelType), m_width, m_height, 0, toGLFormat(m_pixelType),
                 GL_UNSIGNED_BYTE, nullptr);
}

void Texture::setData(const unsigned char *data) const
{
    bind();
    glTexSubImage2D(Target, 0, 0, 0, m_width, m_height, toGLFormat(m_pixelType), GL_UNSIGNED_BYTE, data);
}

void Texture::bind(int textureUnit) const
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(Target, m_id);
}

} // namespace muui::gl
