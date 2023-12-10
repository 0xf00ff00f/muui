#include "testwindow.h"

#include "panic.h"

#include <muui/system.h>

TestWindow::TestWindow(int width, int height, const char *title)
    : m_width(width)
    , m_height(height)
    , m_title(title)
{
}

TestWindow::~TestWindow()
{
    if (m_window)
        destroyWindow();
}

void TestWindow::run()
{
    createWindow();
    initialize();

    auto curTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window))
    {
        auto now = glfwGetTime();
        auto elapsed = now - curTime;
        curTime = now;

        render();
        update(elapsed);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void TestWindow::createWindow()
{
    glfwInit();

    glfwSetErrorCallback(
        [](int error, const char *description) { panic("GLFW error %08x: %s\n", error, description); });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    System::initialize();
}

void TestWindow::destroyWindow()
{
    System::shutdown();

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void TestWindow::initialize() {}

void TestWindow::render() {}

void TestWindow::update([[maybe_unused]] float dt) {}
