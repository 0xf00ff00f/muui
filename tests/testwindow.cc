#include "testwindow.h"

#include "panic.h"

#include <muui/system.h>

#include <iostream>

namespace
{

const char *debugSource(GLenum source)
{
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        return "SOURCE_API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "SOURCE_WINDOW_SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "SOURCE_SHADER_COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "SOURCE_THIRD_PARTY";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "SOURCE_APPLICATION";
    case GL_DEBUG_SOURCE_OTHER:
        return "SOURCE_OTHER";
    default:
        return "???";
    }
}

const char *debugType(GLenum type)
{
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        return "TYPE_ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "TYPE_DEPRECATED_BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "TYPE_UNDEFINED_BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "TYPE_PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "TYPE_PERFORMANCE";
    case GL_DEBUG_TYPE_MARKER:
        return "TYPE_MARKER";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "TYPE_PUSH_GROUP";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "TYPE_POP_GROUP";
    case GL_DEBUG_TYPE_OTHER:
        return "TYPE_OTHER";
    default:
        return "???";
    }
}

const char *debugSeverity(GLenum severity)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        return "SEVERITY_HIGH";
    case GL_DEBUG_SEVERITY_MEDIUM:
        return "SEVERITY_MEDIUM";
    case GL_DEBUG_SEVERITY_LOW:
        return "SEVERITY_LOW";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "SEVERITY_NOTIFICATION";
    default:
        return "???";
    }
}

} // namespace

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

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 16);
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    muui::System::initialize();

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(
        [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message,
           const void * /*user*/) {
            if (type != GL_DEBUG_TYPE_OTHER)
            {
                std::cout << debugSource(source) << ':' << debugType(type) << ':' << debugSeverity(severity) << ':'
                          << message << '\n';
            }
        },
        nullptr);
}

void TestWindow::destroyWindow()
{
    muui::System::shutdown();

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void TestWindow::initialize() {}

void TestWindow::render() {}

void TestWindow::update([[maybe_unused]] float dt) {}
