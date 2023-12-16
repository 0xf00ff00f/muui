#include "testwindow.h"

#include "panic.h"

#include <muui/font.h>
#include <muui/framebuffer.h>
#include <muui/item.h>
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

static muui::ShaderManager::ProgramHandle tintProgramHandle()
{
    static auto handle = [] {
        auto *shaderManager = muui::getShaderManager();
        return shaderManager->addProgram({
            .vertexShaderPath = AssetsPath / "tint.vert",
            .fragmentShaderPath = AssetsPath / "tint.frag",
        });
    }();
    return handle;
}

class TintEffect : public muui::ShaderEffect
{
protected:
    void applyEffect(muui::SpriteBatcher *spriteBatcher, const glm::vec2 &pos, int depth) override;
};

void TintEffect::applyEffect(muui::SpriteBatcher *spriteBatcher, const glm::vec2 &pos, int depth)
{
    const auto size = glm::vec2{m_framebuffer->width(), m_framebuffer->height()};
    spriteBatcher->setBatchProgram(tintProgramHandle());
    spriteBatcher->setBatchTexture(m_framebuffer->texture());
    const auto prevBlendFunc = spriteBatcher->batchBlendFunc();
    using muui::BlendFunc;
    spriteBatcher->setBatchBlendFunc({BlendFunc::Factor::One, BlendFunc::Factor::OneMinusSourceAlpha});
    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
    };
    const Vertex topLeftVertex = {.position = pos, .texCoord = {0, 1}};
    const Vertex bottomRightVertex = {.position = pos + size, .texCoord = {1, 0}};
    spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, depth);
    spriteBatcher->setBatchBlendFunc(prevBlendFunc);
}

class EffectTest : public TestWindow
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
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Item> m_rootItem;
    std::unique_ptr<muui::Screen> m_screen;
};

void EffectTest::initialize()
{
    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::Grayscale);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(AssetsPath / "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    auto container = std::make_unique<muui::Column>();
    container->setMargins(muui::Margins{8, 8, 8, 8});
    container->setSpacing(12);

    auto label = std::make_unique<muui::Label>(m_font.get(), U"Sphinx of black quartz"sv);
    label->fillBackground = true;
    label->backgroundColor = {1, 1, 0, 0.5};
    label->setMargins(muui::Margins{12, 12, 12, 12});
    label->shape = muui::Item::Shape::RoundedRectangle;
    label->cornerRadius = 12.0f;
    label->color = {0, 1, 1, 1};
    label->setShaderEffect<TintEffect>();

    auto toggle = std::make_unique<muui::Switch>(60.0f, 30.0f);
    toggle->setChecked(true);
    toggle->backgroundColor = glm::vec4(0.5, 0.5, 0.5, 1);
    toggle->toggledSignal.connect([label = label.get()](bool checked) {
        if (checked)
            label->setShaderEffect<TintEffect>();
        else
            label->clearShaderEffect();
    });

    container->append(std::move(label));
    container->append(std::move(toggle));

    m_rootItem = std::move(container);

    m_screen = std::make_unique<muui::Screen>();
    m_screen->resize(m_width, m_height);
    m_screen->setRootItem(m_rootItem.get());
}

void EffectTest::update(float elapsed)
{
    assert(m_rootItem);
    m_rootItem->update(elapsed);
}

void EffectTest::render()
{
    glClearColor(0.25, 0.5, 0.75, 1);
    glViewport(0, 0, m_screen->width(), m_screen->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screen->render();
}

void EffectTest::mouseButtonEvent(int button, int action, [[maybe_unused]] int mods)
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

void EffectTest::mouseMoveEvent(double x, double y)
{
    int state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
        m_screen->handleTouchEvent(TouchAction::Move, x, y);
}

int main(int argc, char *argv[])
{
    EffectTest w(800, 400, "hello");
    w.run();
}
