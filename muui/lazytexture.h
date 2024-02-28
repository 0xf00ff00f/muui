#pragma once

#include "abstracttexture.h"
#include "texture.h"

namespace muui
{
struct Pixmap;

class LazyTexture : public AbstractTexture
{
public:
    explicit LazyTexture(const Pixmap *pixmap);

    void markDirty();

    void bind(int textureUnit = 0) const override;

    const Pixmap *pixmap() const;

private:
    const Pixmap *m_pixmap;
    gl::Texture m_texture;
    mutable bool m_dirty;
};

} // namespace muui
