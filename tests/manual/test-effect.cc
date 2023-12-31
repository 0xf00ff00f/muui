#include "testwindow.h"

#include "panic.h"

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

static muui::ShaderManager::ProgramHandle transitionProgramHandle()
{
    static auto handle = [] {
        auto *shaderManager = muui::getShaderManager();
        return shaderManager->addProgram({
            .vertexShaderPath = AssetsPath / "transition.vert",
            .fragmentShaderPath = AssetsPath / "transition.frag",
        });
    }();
    return handle;
}

class TransitionEffect : public muui::ShaderEffect
{
public:
    ~TransitionEffect() override = default;

    float progress{0.0f};
    float spacing{30.0f};
    float slope{3.0f};

private:
    void applyEffect(muui::Painter *painter, const glm::vec2 &pos, int depth) override;
};

void TransitionEffect::applyEffect(muui::Painter *painter, const glm::vec2 &pos, int depth)
{
    auto *spriteBatcher = painter->spriteBatcher();
    const auto size = glm::vec2{m_framebuffer->width(), m_framebuffer->height()};
    spriteBatcher->setBatchProgram(transitionProgramHandle());
    spriteBatcher->setBatchTexture(m_framebuffer->texture());
    const auto prevBlendFunc = spriteBatcher->batchBlendFunc();
    using muui::BlendFunc;
    spriteBatcher->setBatchBlendFunc({BlendFunc::Factor::One, BlendFunc::Factor::OneMinusSourceAlpha});
    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
    };
    const Vertex topLeftVertex{.position = pos - glm::vec2(m_padding), .texCoord = {0, 1}};
    const Vertex bottomRightVertex{.position = topLeftVertex.position + size, .texCoord = {1, 0}};

    const glm::vec4 parameters{spacing, slope, progress, 0.0f};
    spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, parameters, depth);
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
    std::unique_ptr<muui::Font> m_bigFont;
    std::unique_ptr<muui::Font> m_smallFont;
    std::unique_ptr<muui::Item> m_rootItem;
    std::unique_ptr<muui::Screen> m_screen;
    muui::Item *m_innerContainer{nullptr};
    float m_direction{1.0f};
};

void EffectTest::initialize()
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
    innerContainer->setMargins(muui::Margins{1, 1, 1, 1});

    auto innerColumn = std::make_unique<muui::Column>();
    innerColumn->fillBackground = true;
    innerColumn->backgroundBrush = glm::vec4{1, 1, 1, 0.75};
    innerColumn->setMargins(muui::Margins{12, 12, 12, 12});
    innerColumn->shape = muui::Item::Shape::RoundedRectangle;
    innerColumn->cornerRadius = 12.0f;

    auto label = std::make_unique<muui::Label>(m_bigFont.get(), U"Sphinx of black quartz"sv);
    label->brush = glm::vec4(rgbToColor(0x040a18), 1);

    auto bottomRow = std::make_unique<muui::Row>();

    auto text = std::make_unique<muui::MultiLineText>(m_smallFont.get());
    text->setFixedWidth(500);
    text->setText(
        U"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec a semper quam. Donec tempor bibendum nulla a viverra. Aenean non urna sit amet dolor hendrerit efficitur vitae dapibus ante. Vestibulum et hendrerit metus. Integer ornare, purus vel ultricies porta, nisl ligula vehicula quam, faucibus malesuada diam risus id lacus. Donec velit nisl, cursus id enim at, sagittis bibendum enim. Phasellus elementum quam eu ultrices rhoncus. Pellentesque vel dui id turpis euismod consequat. Fusce ac aliquam nibh. Mauris laoreet tincidunt sem eget varius."sv);
    text->color = glm::vec4(rgbToColor(0x040a18), 1);

    auto image = std::make_unique<muui::Image>((AssetsPath / "vim.png").string());

    bottomRow->append(std::move(text));
    bottomRow->append(std::move(image));

    innerColumn->append(std::move(label));
    innerColumn->append(std::move(bottomRow));

    innerContainer->append(std::move(innerColumn));
    innerContainer->setShaderEffect(std::make_unique<TransitionEffect>());

    auto direction = std::make_unique<muui::Switch>(60.0f, 30.0f);
    direction->setChecked(true);
    direction->backgroundBrush = glm::vec4{0.5, 0.5, 0.5, 1};
    direction->toggledSignal.connect([this](bool checked) {
        if (checked)
            m_direction = 1.0f;
        else
            m_direction = -1.0f;
    });

    auto enableEffect = std::make_unique<muui::Switch>(60.0f, 30.0f);
    enableEffect->setChecked(true);
    enableEffect->backgroundBrush = glm::vec4{0.5, 0.5, 0.5, 1};
    enableEffect->toggledSignal.connect([this, item = innerContainer.get()](bool checked) {
        if (checked)
        {
            item->setShaderEffect(std::make_unique<TransitionEffect>());
        }
        else
        {
            item->clearShaderEffect();
        }
    });

    outerContainer->append(std::move(innerContainer));
    outerContainer->append(std::move(direction));
    outerContainer->append(std::move(enableEffect));

    m_rootItem = std::move(outerContainer);

    m_screen = std::make_unique<muui::Screen>();
    m_screen->resize(m_width, m_height);
    m_screen->setRootItem(m_rootItem.get());
}

void EffectTest::update(float elapsed)
{
    assert(m_rootItem);
    if (auto *effect = static_cast<TransitionEffect *>(m_innerContainer->shaderEffect()))
    {
        float t = effect->progress;
        t += 0.5f * elapsed * m_direction;
        t = std::clamp(t, 0.0f, 1.0f);
        effect->progress = t;
    }
    m_rootItem->update(elapsed);
}

void EffectTest::render()
{
    glClearColor(0.8, 0.95, 1, 1);
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
