#include "application.h"

#include "log.h"
#include "system.h"

#include <cassert>

namespace muui
{

Application::Application() = default;

Application::~Application()
{
    if (m_initialized)
    {
        System::shutdown();

        assert(m_context);
        SDL_GL_DeleteContext(m_context);
        m_context = nullptr;

        assert(m_window);
        SDL_DestroyWindow(m_window);
        m_window = nullptr;

        SDL_Quit();

        m_initialized = false;
    }
}

bool Application::createWindow(int width, int height, const char *title, bool windowed)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        log_error("Failed to initialize SDL");
        return false;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    Uint32 flags = SDL_WINDOW_OPENGL;
#ifndef __ANDROID__
    if (windowed)
        flags |= SDL_WINDOW_RESIZABLE;
    else
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags),
        SDL_DestroyWindow);
    if (!window)
    {
        log_error("Failed to create window");
        SDL_Quit();
        return false;
    }

    std::unique_ptr<std::remove_pointer<SDL_GLContext>::type, decltype(&SDL_GL_DeleteContext)> context(
        SDL_GL_CreateContext(window.get()), SDL_GL_DeleteContext);
    if (!context)
    {
        log_error("Failed to create OpenGL context");
        SDL_Quit();
        return false;
    }

    if (!System::initialize())
        return false;

    m_window = window.release();
    m_context = context.release();

    log_info("GL Vendor: %s", glGetString(GL_VENDOR));
    log_info("GL Renderer: %s", glGetString(GL_RENDERER));
    log_info("GL Version: %s", glGetString(GL_VERSION));

    SDL_GL_SetSwapInterval(0);

    if (!initialize())
        return false;

    m_initialized = true;

    return true;
}

void Application::exec()
{
    if (!m_initialized)
        return;

    int width = 0, height = 0;
    SDL_GetWindowSize(m_window, &width, &height);

    resize(width, height);

    auto recreateGLContext = [this] {
        m_context = SDL_GL_CreateContext(m_window);
        m_contextRecreatedSignal();
        log_info("GL context recreated!");
    };

    m_running = true;
    bool firstFrame = true;
    Uint32 lastUpdate = 0;
    int frameCount = 0;
    Uint32 tRate0 = ~0u;
    while (m_running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_WINDOWEVENT: {
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED: {
                    const auto width = event.window.data1;
                    const auto height = event.window.data2;
                    log_info("Window resized to %dx%d", width, height);
                    resize(width, height);
                    break;
                }
                break;
                }
                break;
            }
            case SDL_RENDER_DEVICE_RESET: {
                recreateGLContext();
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED)
                    handleTouchEvent(TouchAction::Down, event.button.x, event.button.y);
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_RELEASED)
                    handleTouchEvent(TouchAction::Up, event.button.x, event.button.y);
                break;
            }
            case SDL_MOUSEMOTION: {
                if (event.button.button & SDL_BUTTON_LMASK)
                    handleTouchEvent(TouchAction::Move, event.motion.x, event.motion.y);
                break;
            }
            case SDL_KEYDOWN: {
                const SDL_Keycode key = event.key.keysym.sym;
                switch (key)
                {
                case SDLK_ESCAPE:
                    m_running = false;
                    break;
                default:
                    handleKeyPress(event.key.keysym);
                    break;
                }
                break;
            }
            case SDL_TEXTINPUT: {
                handleTextInputEvent(event.text.text); // utf-8 encoded
                break;
            }
            case SDL_QUIT:
                m_running = false;
                break;
            default:
                break;
            }
        }

        const Uint32 now = SDL_GetTicks();
        if (firstFrame)
        {
            lastUpdate = now;
            firstFrame = false;
        }
        const auto elapsed = static_cast<float>(now - lastUpdate) / 1000.0f;
        update(elapsed);
        lastUpdate = now;

        render();
        SDL_GL_SwapWindow(m_window);

        ++frameCount;

        if (tRate0 == ~0u)
            tRate0 = now;
        if (now - tRate0 >= 5000)
        {
            const auto seconds = static_cast<float>(now - tRate0) / 1000;
            m_fps = frameCount / seconds;
            tRate0 = now;
            frameCount = 0;
        }
    }
}

void Application::quit()
{
    m_running = false;
}

void Application::update(float) {}

void Application::handleKeyPress(const SDL_Keysym &) {}

void Application::handleTouchEvent(TouchAction, int, int) {}

void Application::handleTextInputEvent(std::string_view) {}

} // namespace muui
