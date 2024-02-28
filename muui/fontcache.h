#pragma once

#include "noncopyable.h"

#include "font.h"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace muui
{
class TextureAtlas;
class Font;

class FontCache : private NonCopyable
{
public:
    explicit FontCache(TextureAtlas *textureAtlas);
    ~FontCache();

    Font *font(std::string_view name, int pixelHeight, int outlineSize = 0);

    void setRootPath(const std::filesystem::path &path);
    const std::filesystem::path &rootPath() const { return m_rootPath; }

private:
    TextureAtlas *m_textureAtlas;
    struct FontKey
    {
        std::string name;
        int pixelHeight;
        int outlineSize;
        bool operator==(const FontKey &other) const = default;
    };
    struct FontKeyHasher
    {
        std::size_t operator()(const FontKey &key) const;
    };
    std::unordered_map<FontKey, std::unique_ptr<Font>, FontKeyHasher> m_fonts;
    std::filesystem::path m_rootPath;
};

} // namespace muui
