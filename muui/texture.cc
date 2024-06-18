#include "texture.h"
#include "pixmap.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>

namespace muui::gl
{

namespace
{
GLenum toGLFormat(PixelType pixelType)
{
    return pixelType == PixelType::RGBA ? GL_RGBA : GL_LUMINANCE;
}

GLenum toGLInternalFormat(PixelType pixelType)
{
    return pixelType == PixelType::RGBA ? GL_RGBA : GL_LUMINANCE;
}
} // namespace

Texture::Texture(const Pixmap &pixmap, Target target)
    : Texture(pixmap.width, pixmap.height, pixmap.pixelType, pixmap.pixels.data(), target)
{
}

Texture::Texture(int width, int height, PixelType pixelType, const unsigned char *data, Target target)
    : m_width(width)
    , m_height(height)
    , m_pixelType(pixelType)
    , m_target(target)
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
    : m_width(std::exchange(other.m_width, 0))
    , m_height(std::exchange(other.m_height, 0))
    , m_pixelType(std::exchange(other.m_pixelType, PixelType::Invalid))
    , m_id(std::exchange(other.m_id, 0))
{
}

Texture &Texture::operator=(Texture other)
{
    swap(*this, other);
    return *this;
}

void swap(Texture &lhs, Texture &rhs)
{
    using std::swap;
    swap(lhs.m_width, rhs.m_width);
    swap(lhs.m_height, rhs.m_height);
    swap(lhs.m_pixelType, rhs.m_pixelType);
    swap(lhs.m_id, rhs.m_id);
}

void Texture::initialize()
{
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    switch (m_target)
    {
    case Target::TextureCubeMap:
        for (std::size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
            allocateTextureData(faceIndex);
        break;
    default:
        allocateTextureData();
        break;
    }
}

void Texture::setMinificationFilter(Filter filter)
{
    bind();
    glTexParameteri(static_cast<GLenum>(m_target), GL_TEXTURE_MIN_FILTER, static_cast<GLint>(filter));
}

void Texture::setMagnificationFilter(Filter filter)
{
    bind();
    glTexParameteri(static_cast<GLenum>(m_target), GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filter));
}

void Texture::setWrapModeS(WrapMode mode)
{
    bind();
    glTexParameteri(static_cast<GLenum>(m_target), GL_TEXTURE_WRAP_S, static_cast<GLint>(mode));
}

void Texture::setWrapModeT(WrapMode mode)
{
    bind();
    glTexParameteri(static_cast<GLenum>(m_target), GL_TEXTURE_WRAP_T, static_cast<GLint>(mode));
}

void Texture::setWrapModeR(WrapMode mode)
{
    bind();
    glTexParameteri(static_cast<GLenum>(m_target), GL_TEXTURE_WRAP_R, static_cast<GLint>(mode));
}

void Texture::allocateTextureData(std::size_t faceIndex) const
{
    bind();
    glTexImage2D(faceTarget(faceIndex), 0, toGLInternalFormat(m_pixelType), m_width, m_height, 0,
                 toGLFormat(m_pixelType), GL_UNSIGNED_BYTE, nullptr);
}

void Texture::setData(const unsigned char *data, std::size_t faceIndex) const
{
    bind();
    glTexSubImage2D(faceTarget(faceIndex), 0, 0, 0, m_width, m_height, toGLFormat(m_pixelType), GL_UNSIGNED_BYTE, data);
}

GLenum Texture::faceTarget(std::size_t faceIndex) const
{
    switch (m_target)
    {
    case Target::TextureCubeMap:
        assert(faceIndex < 6);
        return static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex);
    default:
        assert(faceIndex == 0);
        return static_cast<GLenum>(m_target);
    }
}

void Texture::bind(int textureUnit) const
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(static_cast<GLenum>(m_target), m_id);
}

} // namespace muui::gl
