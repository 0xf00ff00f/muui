#include "testwindow.h"

#include "panic.h"

#include <muui/spritebatcher.h>

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

class SpriteBatcherTest : public TestWindow
{
public:
    using TestWindow::TestWindow;

    void initialize() override;
    void render() override;

private:
    std::unique_ptr<muui::SpriteBatcher> m_spriteBatcher;
};

void SpriteBatcherTest::initialize()
{
    m_spriteBatcher = std::make_unique<muui::SpriteBatcher>();
    const auto mvp = glm::ortho(0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f);
    m_spriteBatcher->setTransformMatrix(mvp);
}

void SpriteBatcherTest::render()
{
    glClearColor(0.25, 0.25, 0.25, 1);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    m_spriteBatcher->begin();
    m_spriteBatcher->setBatchProgram(muui::ShaderManager::ProgramFlat);
    m_spriteBatcher->setBatchScissorBox({.position = {0, 0}, .size = {m_width, m_height}});
    {
        struct Vertex
        {
            glm::vec2 position;
            glm::vec4 color;
        };
        std::array<Vertex, 4> quad = {{{{20, 20}, {1, 0, 0, 1}},
                                       {{220, 20}, {0, 1, 0, 1}},
                                       {{220, 220}, {0, 0, 1, 1}},
                                       {{20, 220}, {1, 1, 0, 1}}}};
        m_spriteBatcher->addSprite(quad, 0);
    }
    m_spriteBatcher->flush();
}

int main(int argc, char *argv[])
{
    SpriteBatcherTest w(800, 400, "hello");
    w.run();
}
