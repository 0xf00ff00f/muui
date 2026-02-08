#pragma once

#ifdef MUUI_USE_SDL2
#include <SDL_log.h>
#else
#include <cstdio>
#endif
#include <fmt/format.h>

enum class LogPriority
{
#ifdef MUUI_USE_SDL2
    Error = SDL_LOG_PRIORITY_ERROR,
    Info = SDL_LOG_PRIORITY_INFO,
    Debug = SDL_LOG_PRIORITY_DEBUG
#else
    Error,
    Info,
    Debug
#endif
};

#ifndef MUUI_USE_SDL2
inline const char *logPriorityName(LogPriority priority)
{
    switch (priority)
    {
    case LogPriority::Error:
        return "error";
        break;
    case LogPriority::Info:
        return "info";
        break;
    case LogPriority::Debug:
        return "debug";
        break;
    default:
        return "???";
    }
}
#endif

template<typename... Args>
inline void log_message(LogPriority priority, fmt::format_string<Args...> fmt, Args &&...args)
{
    const auto message = fmt::vformat(fmt.get(), fmt::make_format_args(args...));
#ifdef MUUI_USE_SDL2
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, static_cast<SDL_LogPriority>(priority), "%s", message.c_str());
#else
    fprintf(stderr, "[%s] %s\n", logPriorityName(priority), message.c_str());
#endif
}

template<typename... Args>
inline void log_error(fmt::format_string<Args...> fmt, Args &&...args)
{
    log_message(LogPriority::Error, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_info(fmt::format_string<Args...> fmt, Args &&...args)
{
    log_message(LogPriority::Info, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_debug(fmt::format_string<Args...> fmt, Args &&...args)
{
    log_message(LogPriority::Debug, fmt, std::forward<Args>(args)...);
}
