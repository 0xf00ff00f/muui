#pragma once

#include "textureatlas.h"
#include "util.h"

#include <glm/glm.hpp>
#include <stb_truetype.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

struct Pixmap;

namespace muui
{

class Font
{
public:
    explicit Font(TextureAtlas *textureAtlas);
    ~Font();

    bool load(const std::filesystem::path &path, int pixelHeight);

    struct Glyph
    {
        RectI boundingBox;
        float advanceWidth;
        PackedPixmap pixmap;
    };
    const Glyph *glyph(int codepoint);

    int pixelHeight() const { return m_pixelHeight; }
    float ascent() const { return m_ascent; }
    float descent() const { return m_descent; }
    float lineGap() const { return m_lineGap; }
    float textWidth(std::u32string_view text);

private:
    std::unique_ptr<Glyph> initializeGlyph(int codepoint);

    TextureAtlas *m_textureAtlas;
    std::vector<std::byte> m_ttfBuffer;
    stbtt_fontinfo m_font;
    std::unordered_map<int, std::unique_ptr<Glyph>> m_glyphs;
    int m_pixelHeight;
    float m_scale = 0.0f;
    float m_ascent;
    float m_descent;
    float m_lineGap;
};

} // namespace muui
