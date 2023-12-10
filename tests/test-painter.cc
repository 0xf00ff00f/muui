#include "testwindow.h"

#include "panic.h"

#include <muui/font.h>
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
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_font;
};

void PainterTest::initialize()
{
    m_painter = std::make_unique<muui::Painter>();
    m_painter->setWindowSize(m_width, m_height);

    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::Grayscale);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(ASSETSDIR "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");
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

    m_painter->begin();
    m_painter->drawCircle({40, 40}, 30, glm::vec4(1), 0);
    m_painter->drawRoundedRect({{10, 80}, {100, 140}}, 20, glm::vec4(1), 0);
    m_painter->drawRect({{10, 150}, {100, 210}}, glm::vec4(1), 0);
    m_painter->setFont(m_font.get());
    m_painter->drawText(U"Sphinx of black quartz"sv, {10, 220}, glm::vec4(1), 0);
    m_painter->end();
}

int main(int argc, char *argv[])
{
    PainterTest w(800, 400, "hello");
    w.run();
}
