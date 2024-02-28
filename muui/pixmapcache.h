#pragma once

#include "noncopyable.h"

#include "textureatlas.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace muui
{
class TextureAtlas;

class PixmapCache : private NonCopyable
{
public:
    explicit PixmapCache(TextureAtlas *textureAtlas);
    ~PixmapCache();

    std::optional<PackedPixmap> pixmap(std::string_view source);

    void setRootPath(const std::filesystem::path &path);
    const std::filesystem::path &rootPath() const { return m_rootPath; }

private:
    TextureAtlas *m_textureAtlas;
    std::unordered_map<std::string, std::optional<PackedPixmap>> m_pixmaps;
    std::filesystem::path m_rootPath;
};

} // namespace muui
