#include "font.h"

#include "file.h"
#include "log.h"
#include "pixmap.h"

#include <stb_truetype.h>

namespace muui
{

struct FontInfo
{
    std::vector<std::byte> buffer;
    stbtt_fontinfo font;
};

std::unique_ptr<FontInfo> loadFont(const std::filesystem::path &path)
{
    File file(path);
    if (!file)
    {
        log_error("Failed to load font %s", path.c_str());
        return {};
    }
    auto font = std::make_unique<FontInfo>();
    font->buffer = file.readAll();
    auto *ttfData = reinterpret_cast<const unsigned char *>(font->buffer.data());
    int result = stbtt_InitFont(&font->font, ttfData, stbtt_GetFontOffsetForIndex(ttfData, 0));
    if (result == 0)
    {
        log_error("Failed to parse font %s", path.c_str());
        return {};
    }
    log_info("Loaded font %s", path.c_str());
    return font;
}

namespace
{

class FontInfoCache
{
public:
    FontInfo *get(const std::filesystem::path &path);

private:
    std::unordered_map<std::string, std::unique_ptr<FontInfo>> m_fonts;
};

FontInfo *FontInfoCache::get(const std::filesystem::path &path)
{
    const auto key = path.lexically_normal().string();
    auto it = m_fonts.find(key);
    if (it == m_fonts.end())
        it = m_fonts.emplace(key, loadFont(path)).first;
    return it->second.get();
}

} // namespace

Font::Font(TextureAtlas *textureAtlas)
    : m_textureAtlas(textureAtlas)
{
}

Font::~Font() = default;

bool Font::load(const std::filesystem::path &path, int pixelHeight)
{
    static FontInfoCache cache;

    m_fontInfo = cache.get(path);
    if (!m_fontInfo)
        return false;

    m_scale = stbtt_ScaleForPixelHeight(&m_fontInfo->font, pixelHeight);

    int ascent;
    int descent;
    int lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo->font, &ascent, &descent, &lineGap);
    m_ascent = m_scale * ascent;
    m_descent = m_scale * descent;
    m_lineGap = m_scale * lineGap;

    m_pixelHeight = pixelHeight;

    return true;
}

const Font::Glyph *Font::glyph(int codepoint)
{
    auto it = m_glyphs.find(codepoint);
    if (it == m_glyphs.end())
        it = m_glyphs.emplace(codepoint, initializeGlyph(codepoint)).first;
    return it->second.get();
}

float Font::textWidth(std::u32string_view text)
{
    float width = 0.0f;
    for (auto ch : text)
    {
        if (const auto *g = glyph(ch); g)
            width += g->advanceWidth;
    }
    return width;
}

std::unique_ptr<Font::Glyph> Font::initializeGlyph(int codepoint)
{
    int ix0, iy0, ix1, iy1;
    stbtt_GetCodepointBitmapBox(&m_fontInfo->font, codepoint, m_scale, m_scale, &ix0, &iy0, &ix1, &iy1);

    const auto width = ix1 - ix0;
    const auto height = iy1 - iy0;

    std::vector<unsigned char> pixels;
    pixels.resize(width * height);
    stbtt_MakeCodepointBitmap(&m_fontInfo->font, pixels.data(), width, height, width, m_scale, m_scale, codepoint);

    constexpr auto Border = 1;

    Pixmap pixmap;
    pixmap.width = width + 2 * Border;
    pixmap.height = height + 2 * Border;
    pixmap.pixelType = PixelType::Grayscale;
    pixmap.pixels.resize(pixmap.width * pixmap.height);
    std::fill(pixmap.pixels.begin(), pixmap.pixels.end(), 0);
    for (int i = 0; i < height; ++i)
    {
        const auto *src = pixels.data() + i * width;
        auto *dest = pixmap.pixels.data() + (i + Border) * pixmap.width + Border;
        std::copy(src, src + width, dest);
    }

    auto packedPixmap = m_textureAtlas->addPixmap(pixmap);
    if (!packedPixmap)
    {
        log_error("Couldn't fit glyph %d in texture atlas", codepoint);
        return {};
    }

    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(&m_fontInfo->font, codepoint, &advanceWidth, &leftSideBearing);

    ix0 -= Border;
    ix1 += Border;
    iy0 -= Border;
    iy1 += Border;
    assert(packedPixmap->width == ix1 - ix0);
    assert(packedPixmap->height == iy1 - iy0);

    auto glyph = std::make_unique<Glyph>();
    glyph->boundingBox = RectI{{ix0, iy0}, {ix1, iy1}};
    glyph->advanceWidth = m_scale * advanceWidth;
    glyph->pixmap = *packedPixmap;
    return glyph;
}

} // namespace muui
