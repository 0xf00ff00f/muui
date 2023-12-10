#include "fontcache.h"

#include "assetpath.h"
#include "log.h"

#include <fmt/core.h>

#include <cstddef>

namespace miniui
{

FontCache::FontCache(TextureAtlas *textureAtlas)
    : m_textureAtlas(textureAtlas)
{
}

FontCache::~FontCache() = default;

std::size_t FontCache::FontKeyHasher::operator()(const FontCache::FontKey &key) const
{
    std::size_t hash = 311;
    hash = hash * 31 + static_cast<std::size_t>(key.pixelHeight);
    hash = hash * 31 + std::hash<std::string>()(key.name);
    return hash;
}

Font *FontCache::font(std::string_view name, int pixelHeight)
{
    FontKey key{std::string(name), pixelHeight};
    auto it = m_fonts.find(key);
    if (it == m_fonts.end())
    {
        auto font = std::make_unique<Font>(m_textureAtlas);
        const auto path = assetPath(fmt::format("fonts/{}.ttf", name));
        if (!font->load(path, pixelHeight))
        {
            log_error("Failed to load font %s", path.c_str());
            font.reset();
        }
        it = m_fonts.emplace(std::move(key), std::move(font)).first;
    }
    return it->second.get();
}

} // namespace miniui
