#include "pixmapcache.h"

#include "log.h"

#include <fmt/core.h>

namespace muui
{

PixmapCache::PixmapCache(TextureAtlas *textureAtlas)
    : m_textureAtlas(textureAtlas)
{
}

PixmapCache::~PixmapCache() = default;

std::optional<PackedPixmap> PixmapCache::pixmap(std::string_view source)
{
    auto key = std::string(source);
    auto it = m_pixmaps.find(key);
    if (it == m_pixmaps.end())
    {
        auto pixmap = [this, &source]() -> std::optional<PackedPixmap> {
            const auto path = m_rootPath / source;
            Pixmap pm = loadPixmap(path.string());
            if (!pm)
            {
                log_error("Failed to load image {}", path.c_str());
                return std::nullopt;
            }
            return m_textureAtlas->addPixmap(pm);
        }();
        it = m_pixmaps.emplace(std::move(key), pixmap).first;
    }
    return it->second;
}

void PixmapCache::setRootPath(const std::filesystem::path &path)
{
    m_rootPath = path;
}

} // namespace muui
