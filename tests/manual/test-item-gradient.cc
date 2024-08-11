#include "panic.h"

#include <muui/application.h>
#include <muui/font.h>
#include <muui/gradienttexture.h>
#include <muui/item.h>
#include <muui/painter.h>
#include <muui/textureatlas.h>

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

class LayoutTest : public muui::Application
{
protected:
    bool initialize() override;
    void resize(int width, int height) override;
    void render() const override;

private:
    std::unique_ptr<muui::Painter> m_painter;
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Item> m_rootItem;
    std::unique_ptr<muui::GradientTexture> m_gradientTexture;
};

bool LayoutTest::initialize()
{
    m_gradientTexture = std::make_unique<muui::GradientTexture>();
    m_gradientTexture->setColorAt(0, glm::vec4(1, 0, 0, 1));
    m_gradientTexture->setColorAt(1, glm::vec4(0, 0, 1, 1));
    m_gradientTexture->setColorAt(0.5, glm::vec4(0, 1, 1, 1));

    m_painter = std::make_unique<muui::Painter>();

    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(ASSETSDIR "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    auto addRectangle = [this](muui::Item *parent, float width, float height) {
        const muui::LinearGradient gradient = {
            .texture = m_gradientTexture.get(), .start = glm::vec2(0, 0), .end = glm::vec2(0, 1)};
        auto *rect = parent->appendChild<muui::Rectangle>(width, height);
        rect->fillBackground = true;
        rect->backgroundBrush = gradient;
    };

    auto addLabel = [this](muui::Item *parent, std::u32string_view text) {
        const muui::LinearGradient gradient = {
            .texture = m_gradientTexture.get(), .start = glm::vec2(0, 0.2), .end = glm::vec2(0, 0.8)};
        auto *label = parent->appendChild<muui::Label>(m_font.get(), text);
        label->fillBackground = true;
        label->backgroundBrush = glm::vec4{1, 0, 0, 1};
        label->foregroundBrush = gradient;
    };

    auto root = std::make_unique<muui::Column>();
    root->setSpacing(8);

    addRectangle(root.get(), 200, 100);
    addLabel(root.get(), U"hello, world"sv);
    addRectangle(root.get(), 200, 100);
    addLabel(root.get(), U"hello, world"sv);

    m_rootItem = std::move(root);

    return true;
}

void LayoutTest::resize(int width, int height)
{
    m_painter->setWindowSize(width, height);
}

void LayoutTest::render() const
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
    m_painter->translate({8, 8});
    m_rootItem->render(m_painter.get());
    m_painter->end();
}

int main(int argc, char *argv[])
{
    LayoutTest app;
    if (app.createWindow(800, 400, "hello"))
        app.exec();
}
