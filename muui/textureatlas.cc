#include "textureatlas.h"

#include "log.h"
#include "pixmap.h"

namespace muui
{

TextureAtlas::TextureAtlas(int pageWidth, int pageHeight)
    : m_pageWidth(pageWidth)
    , m_pageHeight(pageHeight)
{
}

TextureAtlas::~TextureAtlas() = default;

int TextureAtlas::pageWidth() const
{
    return m_pageWidth;
}

int TextureAtlas::pageHeight() const
{
    return m_pageHeight;
}

std::optional<PackedPixmap> TextureAtlas::addPixmap(const Pixmap &pm)
{
    if (pm.width > m_pageWidth || pm.height > m_pageHeight)
    {
        log_error("Pixmap too large for texture atlas");
        return std::nullopt;
    }

    const auto pixelType = pm.pixelType;

    std::optional<RectF> texCoord;
    LazyTexture *texture = nullptr;

    for (auto &entry : m_pages)
    {
        auto &page = entry->page;

        if (page.pixelType() != pixelType)
            continue;

        if ((texCoord = page.insert(pm)))
        {
            entry->texture.markDirty();
            texture = &entry->texture;
            break;
        }
    }

    if (!texCoord)
    {
        m_pages.emplace_back(new PageTexture(m_pageWidth, m_pageHeight, pixelType));
        auto &entry = m_pages.back();
        texCoord = entry->page.insert(pm);
        if (!texCoord)
        {
            // shouldn't ever happen
            assert(false);
            return std::nullopt;
        }
        texture = &entry->texture;
    }

    PackedPixmap packedPixmap;
    packedPixmap.width = pm.width;
    packedPixmap.height = pm.height;
    packedPixmap.texCoord = *texCoord;
    packedPixmap.texture = texture;

    return packedPixmap;
}

int TextureAtlas::pageCount() const
{
    return m_pages.size();
}

const TextureAtlasPage &TextureAtlas::page(int index) const
{
    return m_pages[index]->page;
}

TextureAtlas::PageTexture::PageTexture(int width, int height, PixelType pixelType)
    : page(width, height, pixelType)
    , texture(&page.pixmap())
{
}

} // namespace muui
