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

void dilateAlpha(Pixmap &pixmap, int outlineSize)
{
    assert(pixmap.pixelType == PixelType::RGBA);

    const auto width = pixmap.width;
    const auto height = pixmap.height;
    auto *pixels = reinterpret_cast<glm::u8vec4 *>(pixmap.pixels.data());

    std::vector<glm::vec2> closest(width * height);

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const auto index = y * width + x;
            const auto sourceAlpha = pixels[index].a;
            closest[index] = sourceAlpha > 0.5f ? glm::ivec2{x, y} : glm::ivec2{-1};
            // TODO sobel filter to estimate sub-pixel distance, see
            // https://bgolus.medium.com/the-quest-for-very-wide-outlines-ba82ed442cd9
        }
    }

    std::size_t jumpDistance = 1;
    while (jumpDistance < outlineSize)
        jumpDistance <<= 1;

    while (jumpDistance > 0)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const std::size_t index = y * width + x;
                auto curClosest = closest[index];
                auto closestDistance = curClosest != glm::vec2{-1} ? glm::distance(glm::vec2{x, y}, curClosest)
                                                                   : std::numeric_limits<float>::max();
                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dy == 0)
                            continue;
                        const auto ys = y + dy * jumpDistance;
                        const auto xs = x + dx * jumpDistance;
                        if (ys >= 0 && ys < height && xs >= 0 && xs < width)
                        {
                            const auto candidate = closest[ys * width + xs];
                            if (candidate != glm::vec2{-1})
                            {
                                const auto candidateDistance = glm::distance(glm::vec2{x, y}, candidate);
                                if (candidateDistance < closestDistance)
                                {
                                    closest[index] = candidate;
                                    closestDistance = candidateDistance;
                                }
                            }
                        }
                    }
                }
            }
        }
        jumpDistance >>= 1;
    }

    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const auto i = y * width + x;
            const auto origAlpha = static_cast<int>(pixels[i].a);
            const auto curClosest = closest[i];
            const auto closestDistance = curClosest == glm::vec2{-1} ? std::numeric_limits<float>::max()
                                                                     : glm::distance(glm::vec2{x, y}, curClosest);
            const auto alpha = 255 * (1.0f - glm::smoothstep(static_cast<float>(outlineSize) - 1.0f,
                                                             static_cast<float>(outlineSize) + 1.0f, closestDistance));
            const auto destColor = glm::u8vec4(origAlpha, origAlpha, origAlpha, alpha);
            pixels[i] = destColor;
        }
    }
}

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

    std::vector<unsigned char> pixels;
    pixels.resize(width * height);
    stbtt_MakeCodepointBitmap(&m_fontInfo->font, pixels.data(), width, height, width, m_scale, m_scale, codepoint);

    constexpr auto Border = 1;

    const int margin = Border + m_outlineSize;
    const auto pixelType = m_outlineSize == 0 ? PixelType::Grayscale : PixelType::RGBA;

    Pixmap pixmap;
    pixmap.width = width + 2 * margin;
    pixmap.height = height + 2 * margin;
    pixmap.pixelType = pixelType;
    pixmap.pixels.resize(pixmap.width * pixmap.height * pixelSizeInBytes(pixelType));
    std::fill(pixmap.pixels.begin(), pixmap.pixels.end(), 0);
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

    if (m_outlineSize > 0)
    {
        dilateAlpha(pixmap, m_outlineSize);
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
