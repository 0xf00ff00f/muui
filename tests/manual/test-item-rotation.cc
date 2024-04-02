#include "panic.h"

#include <muui/application.h>
#include <muui/dropshadow.h>
#include <muui/font.h>
#include <muui/framebuffer.h>
#include <muui/gradienttexture.h>
#include <muui/item.h>
#include <muui/log.h>
#include <muui/painter.h>
#include <muui/screen.h>
#include <muui/shadereffect.h>
#include <muui/shadermanager.h>
#include <muui/spritebatcher.h>
#include <muui/system.h>
#include <muui/textureatlas.h>

#include <iostream>
#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

static const std::filesystem::path AssetsPath{ASSETSDIR};

class ItemRotationTest : public muui::Application
{
public:
    bool initialize() override;
    void resize(int width, int height) override;
    void update(float elapsed) override;
    void render() const override;
    void handleTouchEvent(TouchAction action, int x, int y) override;

private:
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Rectangle> m_rootItem;
    std::unique_ptr<muui::Screen> m_screen;
    muui::Button *m_button1{nullptr};
    muui::Button *m_button2{nullptr};
    muui::Button *m_button3{nullptr};
    float m_direction{1.0f};
    float m_time{0.0f};
    std::unique_ptr<muui::GradientTexture> m_gradientTexture;
};

bool ItemRotationTest::initialize()
{
    m_gradientTexture = std::make_unique<muui::GradientTexture>();
    m_gradientTexture->setColorAt(0, glm::vec4(1, 0, 0, 1));
    m_gradientTexture->setColorAt(1, glm::vec4(0, 0, 1, 1));
    m_gradientTexture->setColorAt(0.5, glm::vec4(0, 1, 1, 1));

    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(AssetsPath / "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    m_rootItem = std::make_unique<muui::Rectangle>();

    auto addButton = [this](muui::Item *parent, std::u32string_view text) {
        const muui::LinearGradient gradient = {
            .texture = m_gradientTexture.get(), .start = glm::vec2(0, 0), .end = glm::vec2(0, 1)};
        auto *button = parent->appendChild<muui::Button>(m_font.get(), text);
        button->foregroundBrush = glm::vec4{1};
        button->backgroundBrush = gradient;
        button->fillBackground = true;
        button->setMargins(muui::Margins{4, 4, 4, 4});
        button->setFixedWidth(200);
        button->setFixedHeight(60);
        return button;
    };

    m_button1 = addButton(m_rootItem.get(), U"button 1"sv);
    m_button1->setLeft(muui::Length::pixels(40));
    m_button1->setTop(muui::Length::pixels(40));
    m_button1->setTransformOrigin({100, 30});
    m_button1->clickedSignal.connect([] { std::cout << "button 1\n"; });

    m_button2 = addButton(m_button1, U"button 2"sv);
    m_button2->setLeft(muui::Length::percent(100));
    m_button2->setTop(muui::Length::percent(100));
    m_button2->setTransformOrigin({100, 30});
    m_button2->clickedSignal.connect([] { std::cout << "button 2\n"; });

    m_button3 = addButton(m_button2, U"button 3"sv);
    m_button3->setLeft(muui::Length::percent(100));
    m_button3->setTop(muui::Length::percent(100));
    m_button3->setTransformOrigin({100, 30});
    m_button3->clickedSignal.connect([] { std::cout << "button 3\n"; });

    m_screen = std::make_unique<muui::Screen>();
    m_screen->setRootItem(m_rootItem.get());

    return true;
}

void ItemRotationTest::resize(int width, int height)
{
    m_rootItem->setSize(width, height);
    m_screen->resize(width, height);
}

void ItemRotationTest::update(float elapsed)
{
    assert(m_rootItem);
    m_time += elapsed;
    m_button1->setRotation(0.3f * sinf(0.3f * m_time));
    m_button2->setRotation(0.2f * sinf(0.2f * m_time));
    m_button3->setRotation(0.1f * sinf(0.1f * m_time));
    m_rootItem->update(elapsed);
}

void ItemRotationTest::render() const
{
    glClearColor(0.8, 0.95, 1, 1);
    glViewport(0, 0, m_screen->width(), m_screen->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screen->render();
}

void ItemRotationTest::handleTouchEvent(TouchAction action, int x, int y)
{
    switch (action)
    {
    case TouchAction::Down:
        m_screen->handleTouchEvent(TouchAction::Down, x, y);
        break;
    case TouchAction::Up:
        m_screen->handleTouchEvent(TouchAction::Up, x, y);
        break;
    case TouchAction::Move:
        m_screen->handleTouchEvent(TouchAction::Move, x, y);
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    ItemRotationTest app;
    if (app.createWindow(800, 400, "hello", true))
        app.exec();
}
