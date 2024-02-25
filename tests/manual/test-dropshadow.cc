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

class DropShadowTest : public muui::Application
{
public:
    void initialize() override;
    void resize(int width, int height) override;
    void update(float elapsed) override;
    void render() const override;
    void handleTouchEvent(TouchAction action, int x, int y) override;

private:
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Container> m_rootItem;
    std::unique_ptr<muui::Screen> m_screen;
    float m_direction{1.0f};
};

void DropShadowTest::initialize()
{
    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::Grayscale);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(AssetsPath / "OpenSans_Bold.ttf", 80))
        panic("Failed to load font\n");

    m_rootItem = std::make_unique<muui::Column>();
    m_rootItem->setMargins(muui::Margins{8, 8, 8, 8});
    m_rootItem->setSpacing(12);

    auto *item = m_rootItem->appendChild<muui::Label>(m_font.get(), U"Sphinx of black quartz"sv);
    item->brush = glm::vec4(1);

    auto applyDropShadow = [](auto *item) {
        auto dropShadow = std::make_unique<muui::DropShadow>();
        dropShadow->offset = glm::vec2{2, 2};
        dropShadow->color = glm::vec4{1, 0, 0, 0.5};
        item->setShaderEffect(std::move(dropShadow));
    };
    applyDropShadow(item);

    auto *enableEffect = m_rootItem->appendChild<muui::Switch>(60.0f, 30.0f);
    enableEffect->setChecked(true);
    enableEffect->backgroundBrush = glm::vec4{0.5, 0.5, 0.5, 1};
    enableEffect->toggledSignal.connect([this, applyDropShadow, item](bool checked) {
        if (checked)
        {
            applyDropShadow(item);
        }
        else
        {
            item->clearShaderEffect();
        }
    });

    m_screen = std::make_unique<muui::Screen>();
    m_screen->setRootItem(m_rootItem.get());
}

void DropShadowTest::resize(int width, int height)
{
    m_screen->resize(width, height);
}

void DropShadowTest::update(float elapsed)
{
    assert(m_rootItem);
    m_rootItem->update(elapsed);
}

void DropShadowTest::render() const
{
    glClearColor(0.8, 0.95, 1, 1);
    glViewport(0, 0, m_screen->width(), m_screen->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screen->render();
}

void DropShadowTest::handleTouchEvent(TouchAction action, int x, int y)
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
    DropShadowTest app;
    if (app.createWindow(800, 400, "hello", true))
        app.exec();
}
