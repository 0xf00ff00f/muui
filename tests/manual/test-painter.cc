#include "testwindow.h"

#include "panic.h"

#include <muui/font.h>
#include <muui/gradienttexture.h>
#include <muui/painter.h>
#include <muui/textureatlas.h>

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

class PainterTest : public TestWindow
{
public:
    using TestWindow::TestWindow;

    void initialize() override;
    void render() override;

private:
    std::unique_ptr<muui::Painter> m_painter;
    std::unique_ptr<muui::TextureAtlas> m_textureAtlasGrayscale;
    std::unique_ptr<muui::TextureAtlas> m_textureAtlasRGBA;
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Font> m_outlineFont;
    std::unique_ptr<muui::GradientTexture> m_gradientTexture;
};

void PainterTest::initialize()
{
    m_painter = std::make_unique<muui::Painter>();
    m_painter->setWindowSize(m_width, m_height);

    m_textureAtlasGrayscale = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::Grayscale);
    m_font = std::make_unique<muui::Font>(m_textureAtlasGrayscale.get());
    if (!m_font->load(ASSETSDIR "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    m_textureAtlasRGBA = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::RGBA);
    m_outlineFont = std::make_unique<muui::Font>(m_textureAtlasRGBA.get());
    if (!m_outlineFont->load(ASSETSDIR "OpenSans_Bold.ttf", 60, 8))
        panic("Failed to load font\n");

    m_gradientTexture = std::make_unique<muui::GradientTexture>();
    m_gradientTexture->setColorAt(0, glm::vec4(1, 0, 0, 1));
    m_gradientTexture->setColorAt(1, glm::vec4(0, 0, 1, 1));
    m_gradientTexture->setColorAt(0.5, glm::vec4(0, 1, 1, 1));
}

void PainterTest::render()
{
    glClearColor(0.25, 0.25, 0.25, 1);
    glViewport(0, 0, m_painter->windowWidth(), m_painter->windowHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    const muui::LinearGradient gradient = {
        .texture = m_gradientTexture.get(), .start = glm::vec2(0, 0), .end = glm::vec2(500, 0)};

    m_painter->begin();
    m_painter->drawCircle({40, 40}, 30, glm::vec4(1), 0);
    m_painter->drawCircle({140, 40}, 30, gradient, 0);
    m_painter->drawRoundedRect({{10, 80}, {100, 140}}, 20, glm::vec4(1), 0);
    m_painter->drawRoundedRect({{110, 80}, {200, 140}}, 20, gradient, 0);
    m_painter->drawRect({{10, 150}, {100, 210}}, glm::vec4(1), 0);
    m_painter->drawRect({{110, 150}, {210, 210}}, gradient, 0);

    m_painter->setFont(m_font.get());
    m_painter->drawText(U"Sphinx of black quartz"sv, {10, 220}, glm::vec4(1), 0);
    m_painter->drawText(U"The quick brown fox"s, {10, 280}, gradient, 0);

    const auto *glyph = m_font->glyph('@');
    m_painter->drawPixmap(glyph->pixmap, {{250, 10}, {350, 110}}, glm::vec4(1), 0);
    m_painter->drawPixmap(glyph->pixmap, {{250, 120}, {350, 220}}, gradient, 0);

    m_painter->setFont(m_outlineFont.get());
    m_painter->drawText(U"Sphinx of black quartz"sv, {10, 340}, glm::vec4(1), glm::vec4(1, 0, 0, 1), 0);
    m_painter->drawText(U"Sphinx of black quartz"sv, {10, 400}, glm::vec4(1), gradient, 0);

    m_painter->end();
}

int main(int argc, char *argv[])
{
    PainterTest w(800, 600, "hello");
    w.run();
}
