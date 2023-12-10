#pragma once

#include "noncopyable.h"

#include "font.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

class TextureAtlas;

namespace muui
{
class Font;

class FontCache : private NonCopyable
{
public:
    explicit FontCache(TextureAtlas *textureAtlas);
    ~FontCache();

    Font *font(std::string_view source, int pixelHeight);

private:
    TextureAtlas *m_textureAtlas;
    struct FontKey
    {
        std::string name;
        int pixelHeight;
        bool operator==(const FontKey &other) const = default;
    };
    struct FontKeyHasher
    {
        std::size_t operator()(const FontKey &key) const;
    };
    std::unordered_map<FontKey, std::unique_ptr<Font>, FontKeyHasher> m_fonts;
};

} // namespace muui
