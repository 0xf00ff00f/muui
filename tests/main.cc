#include "testwindow.h"

#include <muui/painter.h>

#include <memory>

class PainterTest : public TestWindow
{
public:
    using TestWindow::TestWindow;

    void initialize() override;
    void render() override;

private:
    std::unique_ptr<miniui::Painter> m_painter;
};

void PainterTest::initialize()
{
    m_painter = std::make_unique<miniui::Painter>();
    m_painter->setWindowSize(m_width, m_height);
}

void PainterTest::render()
{
    glClearColor(0.25, 0.25, 0.25, 1);
    glViewport(0, 0, m_painter->windowWidth(), m_painter->windowHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    m_painter->begin();
    m_painter->drawCircle({40, 40}, 30, glm::vec4(1), 0);
    m_painter->end();
}

int main(int argc, char *argv[])
{
    PainterTest w(400, 400, "hello");
    w.run();
}
