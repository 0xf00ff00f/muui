#include "lazytexture.h"

#include "pixmap.h"

namespace muui
{

LazyTexture::LazyTexture(const Pixmap *pixmap)
    : m_pixmap(pixmap)
    , m_texture(pixmap->width, pixmap->height, pixmap->pixelType)
    , m_dirty(true)
{
    m_texture.setMinificationFilter(gl::Texture::Filter::Linear);
    m_texture.setMagnificationFilter(gl::Texture::Filter::Linear);
    m_texture.setWrapModeS(gl::Texture::WrapMode::Repeat);
    m_texture.setWrapModeT(gl::Texture::WrapMode::Repeat);
}

void LazyTexture::markDirty()
{
    m_dirty = true;
}

void LazyTexture::bind(int textureUnit) const
{
    if (m_dirty)
    {
        m_texture.setData(m_pixmap->pixels.data());
        m_dirty = false;
    }
    m_texture.bind(textureUnit);
}

const Pixmap *LazyTexture::pixmap() const
{
    return m_pixmap;
}

} // namespace muui
