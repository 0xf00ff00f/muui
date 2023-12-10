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

    int m_width{0};
    int m_height{0};

private:
    void createWindow();
    void destroyWindow();

    std::string m_title;
    bool m_initialized{false};
    GLFWwindow *m_window{nullptr};
};
