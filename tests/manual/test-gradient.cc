#include "panic.h"

#include <muui/application.h>
#include <muui/gradienttexture.h>
#include <muui/spritebatcher.h>

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

class GradientTest : public muui::Application
{
protected:
    void initialize() override;
    void resize(int width, int height) override;
    void render() const override;

private:
    int m_width{0};
    int m_height{0};
    std::unique_ptr<muui::SpriteBatcher> m_spriteBatcher;
    std::unique_ptr<muui::GradientTexture> m_gradientTexture;
};

void GradientTest::initialize()
{
    m_spriteBatcher = std::make_unique<muui::SpriteBatcher>();

    m_gradientTexture = std::make_unique<muui::GradientTexture>();
    m_gradientTexture->setColorAt(0.0f, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
    m_gradientTexture->setColorAt(1.0f, glm::vec4(1.0f));
    m_gradientTexture->setColorAt(0.5f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
}

void GradientTest::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    const auto mvp = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);
    m_spriteBatcher->setTransformMatrix(mvp);
}

void GradientTest::render() const
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
    m_spriteBatcher->setBatchProgram(muui::ShaderManager::ProgramHandle::Gradient);
    m_spriteBatcher->setBatchGradientTexture(m_gradientTexture.get());
    m_spriteBatcher->setBatchScissorBox({.position = {0, 0}, .size = {m_width, m_height}});
    {
        const glm::vec2 gradientFrom{20, 20};
        const glm::vec2 gradientTo{320, 220};
        struct Vertex
        {
            glm::vec2 position;
        };
        const Vertex topLeft{.position = {20, 20}};
        const Vertex bottomRight{.position = {320, 220}};
        m_spriteBatcher->addSprite(topLeft, bottomRight,
                                   glm::vec4(gradientFrom.x, gradientFrom.y, gradientTo.x, gradientTo.y), 0);
    }
    m_spriteBatcher->flush();
}

int main(int argc, char *argv[])
{
    GradientTest app;
    if (app.createWindow(800, 400, "hello", true))
        app.exec();
}
