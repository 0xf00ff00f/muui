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

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

static const std::filesystem::path AssetsPath{ASSETSDIR};

class ScrollAreaTest : public muui::Application
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
    float m_direction{1.0f};
};

bool ScrollAreaTest::initialize()
{
    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(AssetsPath / "OpenSans_Bold.ttf", 300))
        panic("Failed to load font\n");

    m_rootItem = std::make_unique<muui::Rectangle>();

    auto label = std::make_unique<muui::Label>(m_font.get(), U"Sphinx of black quartz"sv);
    label->foregroundBrush = glm::vec4{1};
    label->backgroundBrush = glm::vec4{1, 0, 0, 1};
    label->fillBackground = true;

    auto* scrollArea = m_rootItem->appendChild<muui::ScrollArea>(std::move(label));
    scrollArea->setLeft(muui::Length::pixels(50));
    scrollArea->setTop(muui::Length::pixels(50));
    scrollArea->setViewportSize(muui::Size{150, 150});

    m_screen = std::make_unique<muui::Screen>();
    m_screen->setRootItem(m_rootItem.get());

    return true;
}

void ScrollAreaTest::resize(int width, int height)
{
    m_rootItem->setSize(width, height);
    m_screen->resize(width, height);
}

void ScrollAreaTest::update(float elapsed)
{
    assert(m_rootItem);
    m_rootItem->update(elapsed);
}

void ScrollAreaTest::render() const
{
    glClearColor(0.8, 0.95, 1, 1);
    glViewport(0, 0, m_screen->width(), m_screen->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screen->render();
}

void ScrollAreaTest::handleTouchEvent(TouchAction action, int x, int y)
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
    ScrollAreaTest app;
    if (app.createWindow(800, 400, "hello", true))
        app.exec();
}
