#pragma once

#include <muui/gl.h>
#include <muui/noncopyable.h>

#include <GLFW/glfw3.h>

#include <string>

class TestWindow : public NonCopyable
{
public:
    TestWindow(int width, int height, const char *title);
    ~TestWindow();

    void run();

protected:
    virtual void initialize();
    virtual void update(float dt);
    virtual void render();
    virtual void mouseButtonEvent(int button, int action, int mods);
    virtual void mouseMoveEvent(double x, double y);

    int m_width{0};
    int m_height{0};
    GLFWwindow *m_window{nullptr};

private:
    void createWindow();
    void destroyWindow();

    std::string m_title;
    bool m_initialized{false};
};
