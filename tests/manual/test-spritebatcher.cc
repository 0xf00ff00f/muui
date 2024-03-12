#include "panic.h"

#include <muui/application.h>
#include <muui/spritebatcher.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include <memory>

using namespace std::string_literals;
using namespace std::string_view_literals;

class SpriteBatcherTest : public muui::Application
{
protected:
    bool initialize() override;
    void resize(int width, int height) override;
    void update(float elapsed) override;
    void render() const override;

private:
    int m_width{0};
    int m_height{0};
    std::unique_ptr<muui::SpriteBatcher> m_spriteBatcher;
    float m_angle{0};
};

bool SpriteBatcherTest::initialize()
{
    m_spriteBatcher = std::make_unique<muui::SpriteBatcher>();

    return true;
}

void SpriteBatcherTest::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    const auto mvp = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);
    m_spriteBatcher->setMvp(mvp);
}

void SpriteBatcherTest::update(float elapsed)
{
    m_angle += 0.1f * elapsed;
}

void SpriteBatcherTest::render() const
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
    m_spriteBatcher->setBatchProgram(muui::ShaderManager::ProgramHandle::Flat);
    m_spriteBatcher->setBatchScissorBox({.position = {0, 0}, .size = {m_width, m_height}});

    auto addSprite = [this](const glm::vec2 &topLeft, const glm::vec2 &bottomRight) {
        struct Vertex
        {
            glm::vec2 position;
            glm::vec4 color;
        };
        std::array<Vertex, 4> quad = {{{{topLeft.x, topLeft.y}, {1, 0, 0, 1}},
                                       {{bottomRight.x, topLeft.y}, {0, 1, 0, 1}},
                                       {{bottomRight.x, bottomRight.y}, {0, 0, 1, 1}},
                                       {{topLeft.x, bottomRight.y}, {1, 1, 0, 1}}}};
        m_spriteBatcher->addSprite(quad, 0);
    };
    addSprite({20, 20}, {120, 120});

    {
        const auto t = glm::translate(glm::mat3(1.0), glm::vec2(300, 20));
        const auto r = glm::rotate(glm::mat3(1.0), m_angle);
        m_spriteBatcher->setSpriteTransform(t * r);
        addSprite({0, 0}, {100, 100});
    }

    {
        const auto t0 = glm::translate(glm::mat3(1.0), glm::vec2(300 + 100, 220 + 100));
        const auto r = glm::rotate(glm::mat3(1.0), m_angle);
        const auto t1 = glm::translate(glm::mat3(1.0), glm::vec2(-100, -100));
        m_spriteBatcher->setSpriteTransform(t0 * r * t1);
        addSprite({0, 0}, {100, 100});
    }

    m_spriteBatcher->flush();
}

int main(int argc, char *argv[])
{
    SpriteBatcherTest app;
    if (app.createWindow(800, 400, "hello", true))
        app.exec();
}
