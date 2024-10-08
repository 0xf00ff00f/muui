#include "panic.h"

#include <muui/application.h>
#include <muui/font.h>
#include <muui/gradienttexture.h>
#include <muui/painter.h>
#include <muui/textureatlas.h>

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

class PainterTest : public muui::Application
{
protected:
    bool initialize() override;
    void resize(int width, int height) override;
    void render() const override;

private:
    std::unique_ptr<muui::Painter> m_painter;
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Font> m_outlineFont;
    std::unique_ptr<muui::GradientTexture> m_gradientTexture;
};

bool PainterTest::initialize()
{
    m_painter = std::make_unique<muui::Painter>();

    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512);

    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(ASSETSDIR "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    m_outlineFont = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_outlineFont->load(ASSETSDIR "OpenSans_Bold.ttf", 60, 8))
        panic("Failed to load font\n");

    m_gradientTexture = std::make_unique<muui::GradientTexture>();
    m_gradientTexture->setColorAt(0, glm::vec4(1, 0, 0, 1));
    m_gradientTexture->setColorAt(1, glm::vec4(0, 0, 1, 1));
    m_gradientTexture->setColorAt(0.5, glm::vec4(0, 1, 1, 1));

    return true;
}

void PainterTest::resize(int width, int height)
{
    m_painter->setWindowSize(width, height);
}

void PainterTest::render() const
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
    m_painter->setBackgroundBrush(glm::vec4(1));
    m_painter->drawCircle({40, 40}, 30, 0);
    m_painter->setBackgroundBrush(gradient);
    m_painter->drawCircle({140, 40}, 30, 0);
    m_painter->setBackgroundBrush(glm::vec4(1));
    m_painter->drawRoundedRect({{10, 80}, {100, 140}}, 20, 0);
    m_painter->setBackgroundBrush(gradient);
    m_painter->drawRoundedRect({{110, 80}, {200, 140}}, 20, 0);
    m_painter->setBackgroundBrush(glm::vec4(1));
    m_painter->drawRect({{10, 150}, {100, 210}}, 0);
    m_painter->setBackgroundBrush(gradient);
    m_painter->drawRect({{110, 150}, {210, 210}}, 0);

    m_painter->setFont(m_font.get());
    m_painter->setForegroundBrush(glm::vec4(1));
    m_painter->drawText(U"Sphinx of black quartz"sv, {10, 220}, 0);
    m_painter->setForegroundBrush(gradient);
    m_painter->drawText(U"The quick brown fox"s, {10, 280}, 0);

    const auto *glyph = m_font->glyph('@');
    m_painter->setForegroundBrush(glm::vec4(1));
    m_painter->drawPixmap(glyph->pixmap, {{250, 10}, {350, 110}}, 0);
    m_painter->setForegroundBrush(gradient);
    m_painter->drawPixmap(glyph->pixmap, {{250, 120}, {350, 220}}, 0);

    m_painter->setFont(m_outlineFont.get());
    m_painter->setForegroundBrush(glm::vec4(1));
    m_painter->setOutlineBrush(glm::vec4(1, 0, 0, 1));
    m_painter->drawText(U"Sphinx of black quartz"sv, {10, 340}, 0);
    m_painter->setOutlineBrush(gradient);
    m_painter->drawText(U"Sphinx of black quartz"sv, {10, 400}, 0);

    m_painter->end();
}

int main(int argc, char *argv[])
{
    PainterTest app;
    if (app.createWindow(800, 600, "hello"))
        app.exec();
}
