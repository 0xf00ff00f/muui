#include "testwindow.h"

#include "panic.h"

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

constexpr glm::vec3 rgbToColor(unsigned color)
{
    const auto r = static_cast<float>((color >> 16) & 0xff);
    const auto g = static_cast<float>((color >> 8) & 0xff);
    const auto b = static_cast<float>(color & 0xff);
    return (1.0f / 255.0f) * glm::vec3(r, g, b);
}

class DropShadowTest : public TestWindow
{
public:
    using TestWindow::TestWindow;

    void initialize() override;
    void update(float elapsed) override;
    void render() override;
    void mouseButtonEvent(int button, int action, int mods) override;
    void mouseMoveEvent(double x, double y) override;

private:
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_bigFont;
    std::unique_ptr<muui::Font> m_smallFont;
    std::unique_ptr<muui::Item> m_rootItem;
    std::unique_ptr<muui::Screen> m_screen;
    muui::Item *m_innerContainer{nullptr};
    float m_direction{1.0f};
};

void DropShadowTest::initialize()
{
    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::Grayscale);
    m_bigFont = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_bigFont->load(AssetsPath / "OpenSans_Bold.ttf", 80))
        panic("Failed to load font\n");
    m_smallFont = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_smallFont->load(AssetsPath / "OpenSans-Light.ttf", 18))
        panic("Failed to load font\n");

    auto outerContainer = std::make_unique<muui::Column>();
    outerContainer->setMargins(muui::Margins{8, 8, 8, 8});
    outerContainer->setSpacing(12);

    auto innerContainer = std::make_unique<muui::Column>();
    m_innerContainer = innerContainer.get();
    innerContainer->setMargins(muui::Margins{12, 12, 12, 12});

    auto label = std::make_unique<muui::Label>(m_bigFont.get(), U"Sphinx of black quartz"sv);
    label->color = glm::vec4(rgbToColor(0x040a18), 1);

    innerContainer->append(std::move(label));

    auto applyDropShadow = [](auto *item) {
        auto dropShadow = std::make_unique<muui::DropShadow>();
        dropShadow->offset = glm::vec2{0, 0};
        dropShadow->color = glm::vec4{1, 0, 0, 1};
        item->setShaderEffect(std::move(dropShadow));
    };
    applyDropShadow(innerContainer.get());

    auto enableEffect = std::make_unique<muui::Switch>(60.0f, 30.0f);
    enableEffect->setChecked(true);
    enableEffect->backgroundBrush = glm::vec4{0.5, 0.5, 0.5, 1};
    enableEffect->toggledSignal.connect([this, applyDropShadow, item = innerContainer.get()](bool checked) {
        if (checked)
        {
            applyDropShadow(item);
        }
        else
        {
            item->clearShaderEffect();
        }
    });

    outerContainer->append(std::move(innerContainer));
    outerContainer->append(std::move(enableEffect));

    m_rootItem = std::move(outerContainer);

    m_screen = std::make_unique<muui::Screen>();
    m_screen->resize(m_width, m_height);
    m_screen->setRootItem(m_rootItem.get());
}

void DropShadowTest::update(float elapsed)
{
    assert(m_rootItem);
    m_rootItem->update(elapsed);
}

void DropShadowTest::render()
{
    glClearColor(0.8, 0.95, 1, 1);
    glViewport(0, 0, m_screen->width(), m_screen->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screen->render();
}

void DropShadowTest::mouseButtonEvent(int button, int action, [[maybe_unused]] int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    switch (action)
    {
    case GLFW_PRESS: {
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);
        m_screen->handleTouchEvent(TouchAction::Down, x, y);
        break;
    }
    case GLFW_RELEASE: {
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);
        m_screen->handleTouchEvent(TouchAction::Up, x, y);
        break;
    }
    default:
        break;
    }
}

void DropShadowTest::mouseMoveEvent(double x, double y)
{
    int state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
        m_screen->handleTouchEvent(TouchAction::Move, x, y);
}

int main(int argc, char *argv[])
{
    DropShadowTest w(800, 400, "hello");
    w.run();
}
