#pragma once

#include "flags.h"
#include "noncopyable.h"
#include "uiinput.h"

#include <muslots/muslots.h>

#include <SDL.h>

#include <string_view>

struct SDL_Window;

namespace muui
{

enum class WindowFlag : unsigned
{
    None = 0,
    Windowed = 1 << 0,
    VSync = 1 << 1
};
MUUI_DEFINE_FLAGS(WindowFlags, WindowFlag)

class Application : private NonCopyable
{
public:
    Application();
    virtual ~Application();

    virtual bool createWindow(int width, int height, const char *title,
                              WindowFlags flags = WindowFlag::Windowed | WindowFlag::VSync);
    void exec();
    void quit();

    muslots::Signal<> &contextRecreatedEvent() { return m_contextRecreatedSignal; }

    float framesPerSecond() const { return m_fps; }

protected:
    virtual bool initialize() = 0;
    virtual void resize(int width, int height) = 0;
    virtual void render() const = 0;
    virtual void update(float elapsed);
    virtual void handleKeyPress(const SDL_Keysym &keysym);
    virtual void handleTouchEvent(TouchAction action, int x, int y);
    virtual void handleTextInputEvent(std::string_view text);

private:
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_context = nullptr;
    bool m_initialized = false;
    muslots::Signal<> m_contextRecreatedSignal;
    bool m_running = false;
    float m_fps = 0.0f;
};

} // namespace muui
