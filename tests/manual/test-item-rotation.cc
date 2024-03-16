#include "panic.h"

#include <muui/application.h>
#include <muui/dropshadow.h>
#include <muui/font.h>
#include <muui/framebuffer.h>
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
    muui::Button *m_button{nullptr};
    float m_direction{1.0f};
};

bool ItemRotationTest::initialize()
{
    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(AssetsPath / "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    m_rootItem = std::make_unique<muui::Rectangle>();

    m_button = m_rootItem->appendChild<muui::Button>(m_font.get(), U"Sphinx of black quartz"sv);
    m_button->foregroundBrush = glm::vec4{1};
    m_button->backgroundBrush = glm::vec4{1, 0, 0, 1};
    m_button->fillBackground = true;
    m_button->setMargins(muui::Margins{8, 8, 8, 8});
    m_button->setLeft(muui::Length::pixels(50));
    m_button->setTop(muui::Length::pixels(50));
    m_button->setFixedWidth(400);
    m_button->setFixedHeight(60);
    m_button->setTransformOrigin(glm::vec2{400, 60});
    m_button->clickedSignal.connect([] { std::cout << "**** clicked\n"; });

    m_button->setRotation(0.15f);

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
    m_button->setRotation(m_button->rotation() + 0.1f * elapsed);
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
