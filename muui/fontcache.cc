#include "fontcache.h"

#include "log.h"

#include <fmt/core.h>

#include <cstddef>

namespace muui
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
    hash = hash * 31 + static_cast<std::size_t>(key.outlineSize);
    hash = hash * 31 + std::hash<std::string>()(key.name);
    return hash;
}

Font *FontCache::font(std::string_view source, int pixelHeight, int outlineSize)
{
    FontKey key{std::string(source), pixelHeight};
    auto it = m_fonts.find(key);
    if (it == m_fonts.end())
    {
        auto font = std::make_unique<Font>(m_textureAtlas);
        const auto path = m_rootPath / fmt::format("{}.ttf", source);
        if (!font->load(path, pixelHeight, outlineSize))
        {
            log_error("Failed to load font {}", source);
            font.reset();
        }
        it = m_fonts.emplace(std::move(key), std::move(font)).first;
    }
    return it->second.get();
}

void FontCache::setRootPath(const std::filesystem::path &path)
{
    m_rootPath = path;
}

} // namespace muui
