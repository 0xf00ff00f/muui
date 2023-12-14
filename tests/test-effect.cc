#include "testwindow.h"

#include "panic.h"

#include <muui/font.h>
#include <muui/framebuffer.h>
#include <muui/item.h>
#include <muui/painter.h>
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
    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
    };
    const Vertex topLeftVertex = {.position = pos, .texCoord = {0, 1}};
    const Vertex bottomRightVertex = {.position = pos + size, .texCoord = {1, 0}};
    spriteBatcher->addSprite(topLeftVertex, bottomRightVertex, depth);
}

class EffectTest : public TestWindow
{
public:
    using TestWindow::TestWindow;

    void initialize() override;
    void render() override;

private:
    std::unique_ptr<muui::Painter> m_painter;
    std::unique_ptr<muui::TextureAtlas> m_textureAtlas;
    std::unique_ptr<muui::Font> m_font;
    std::unique_ptr<muui::Item> m_rootItem;
};

void EffectTest::initialize()
{
    m_painter = std::make_unique<muui::Painter>();
    m_painter->setWindowSize(m_width, m_height);

    m_textureAtlas = std::make_unique<muui::TextureAtlas>(512, 512, PixelType::Grayscale);
    m_font = std::make_unique<muui::Font>(m_textureAtlas.get());
    if (!m_font->load(AssetsPath / "OpenSans_Bold.ttf", 60))
        panic("Failed to load font\n");

    auto label = std::make_unique<muui::Label>(m_font.get(), U"Sphinx of black quartz"sv);
    label->fillBackground = true;
    label->backgroundColor = {0.5, 1, 1, 1};
    label->setMargins(muui::Margins{12, 12, 12, 12});
    label->shape = muui::Item::Shape::RoundedRectangle;
    label->cornerRadius = 12.0f;
    label->color = {1, 1, 1, 1};
    label->setShaderEffect<TintEffect>();

    m_rootItem = std::move(label);
}

void EffectTest::render()
{
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, m_painter->windowWidth(), m_painter->windowHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_SCISSOR_TEST);

    m_painter->begin();
    m_rootItem->render(m_painter.get(), {20, 20});
    m_painter->end();
}

int main(int argc, char *argv[])
{
    EffectTest w(800, 400, "hello");
    w.run();
}
