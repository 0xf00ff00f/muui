#pragma once

#include "textureatlas.h"
#include "util.h"

#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>

struct Pixmap;

namespace muui
{
class FontInfo;

class Font
{
public:
    explicit Font(TextureAtlas *textureAtlas);

    bool load(const std::filesystem::path &path, int pixelHeight, int outlineSize = 0);

    struct Glyph
    {
        RectI boundingBox;
        float advanceWidth;
        PackedPixmap pixmap;
    };
    const Glyph *glyph(int codepoint);

    int pixelHeight() const { return m_pixelHeight; }
    int outlineSize() const { return m_outlineSize; }
    float ascent() const { return m_ascent; }
    float descent() const { return m_descent; }
    float lineGap() const { return m_lineGap; }
    float textWidth(std::u32string_view text);

private:
    std::unique_ptr<Glyph> initializeGlyph(int codepoint);

    TextureAtlas *m_textureAtlas;
    FontInfo *m_fontInfo{nullptr};
    std::unordered_map<int, std::unique_ptr<Glyph>> m_glyphs;
    int m_pixelHeight{0};
    int m_outlineSize{0};
    float m_scale{0.0f};
    float m_ascent{0.0f};
    float m_descent{0.0f};
    float m_lineGap{0.0f};
};

} // namespace muui
