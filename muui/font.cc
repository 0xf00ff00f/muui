#include "font.h"

#include "file.h"
#include "log.h"
#include "pixmap.h"

#include <algorithm>

#include <stb_truetype.h>

namespace muui
{

struct FontInfo
{
    std::vector<std::byte> buffer;
    stbtt_fontinfo font;
};

namespace
{

std::unique_ptr<FontInfo> loadFont(const std::filesystem::path &path)
{
    File file(path);
    if (!file)
    {
        log_error("Failed to load font {}", path.c_str());
        return {};
    }
    auto font = std::make_unique<FontInfo>();
    font->buffer = file.readAll();
    auto *ttfData = reinterpret_cast<const unsigned char *>(font->buffer.data());
    int result = stbtt_InitFont(&font->font, ttfData, stbtt_GetFontOffsetForIndex(ttfData, 0));
    if (result == 0)
    {
        log_error("Failed to parse font {}", path.c_str());
        return {};
    }
    log_info("Loaded font {}", path.c_str());
    return font;
}

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

bool Font::load(const std::filesystem::path &path, int pixelHeight, int outlineSize)
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
    m_outlineSize = outlineSize;

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

    constexpr auto Border = 1;

    const int margin = Border + m_outlineSize;
    const auto pixelType = m_outlineSize == 0 ? PixelType::Grayscale : PixelType::RGBA;

    Pixmap pixmap;
    pixmap.width = width + 2 * margin;
    pixmap.height = height + 2 * margin;
    pixmap.pixelType = pixelType;
    pixmap.pixels.resize(pixmap.width * pixmap.height * pixelSizeInBytes(pixelType));
    std::fill(pixmap.pixels.begin(), pixmap.pixels.end(), 0);

    if (m_outlineSize == 0)
    {
        std::vector<unsigned char> pixels;
        pixels.resize(width * height);
        stbtt_MakeCodepointBitmap(&m_fontInfo->font, pixels.data(), width, height, width, m_scale, m_scale, codepoint);

        for (int i = 0; i < height; ++i)
        {
            const auto *src = pixels.data() + i * width;
            auto *dest = pixmap.pixels.data() + ((i + margin) * pixmap.width + margin) * pixelSizeInBytes(pixelType);
            if (pixelType == PixelType::Grayscale)
            {
                std::copy(src, src + width, dest);
            }
            else
            {
                static_assert(sizeof(glm::u8vec4) == 4);
                std::transform(src, src + width, reinterpret_cast<glm::u8vec4 *>(dest),
                               [](unsigned char v) { return glm::u8vec4(255, 255, 255, v); });
            }
        }
    }
    else
    {
        constexpr auto kOnEdgeValue = 180;
        const auto pixelDistScale = static_cast<float>(kOnEdgeValue) / margin;
        int sdfWidth = 0, sdfHeight = 0, xOff = 0, yOff = 0;
        auto *sdf = stbtt_GetCodepointSDF(&m_fontInfo->font, m_scale, codepoint, margin, kOnEdgeValue, pixelDistScale,
                                          &sdfWidth, &sdfHeight, &xOff, &yOff);
        if (sdf)
        {
            assert(sdfWidth == width + 2 * margin);
            assert(sdfHeight == height + 2 * margin);
            assert(pixmap.pixelType == PixelType::RGBA);

            auto *source = sdf;
            auto *destPixels = reinterpret_cast<glm::u8vec4 *>(pixmap.pixels.data());

            const auto glyphEdge = static_cast<float>(kOnEdgeValue) / 255.0f;
            const auto outlineEdge = static_cast<float>(kOnEdgeValue - m_outlineSize * pixelDistScale) / 255.0f;
            const auto feather = pixelDistScale / 255.0f;

            for (std::size_t i = 0; i < sdfWidth * sdfHeight; ++i)
            {
                const auto distance = static_cast<float>(*source++) / 255.0f;
                const auto alpha =
                    static_cast<int>(255.0f * glm::smoothstep(outlineEdge - feather, outlineEdge + feather, distance));
                const auto color =
                    static_cast<int>(255.0f * glm::smoothstep(glyphEdge - feather, glyphEdge + feather, distance));
                *destPixels++ = glm::u8vec4{color, color, color, alpha};
            }

            free(sdf);
        }
    }

    auto packedPixmap = m_textureAtlas->addPixmap(pixmap);
    if (!packedPixmap)
    {
        log_error("Couldn't fit glyph {} in texture atlas", codepoint);
        return {};
    }

    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(&m_fontInfo->font, codepoint, &advanceWidth, &leftSideBearing);

    ix0 -= margin;
    ix1 += margin;
    iy0 -= margin;
    iy1 += margin;
    assert(packedPixmap->width == ix1 - ix0);
    assert(packedPixmap->height == iy1 - iy0);

    auto glyph = std::make_unique<Glyph>();
    glyph->boundingBox = RectI{{ix0, iy0}, {ix1, iy1}};
    glyph->advanceWidth = m_scale * advanceWidth;
    glyph->pixmap = *packedPixmap;
    return glyph;
}

} // namespace muui
