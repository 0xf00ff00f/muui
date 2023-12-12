#pragma once

#ifdef MUUI_USE_SDL2
#include <SDL_log.h>
#else
#include <cstdio>
#endif

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

inline void log_message(LogPriority priority, const char *fmt)
{
#ifdef MUUI_USE_SDL2
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, static_cast<SDL_LogPriority>(priority), "%s", fmt);
#else
    fprintf(stderr, "[%s] %s\n", logPriorityName(priority), fmt);
#endif
}

template<typename... Args>
inline void log_message(LogPriority priority, const char *fmt, const Args &...args)
{
#ifdef MUUI_USE_SDL2
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, static_cast<SDL_LogPriority>(priority), fmt, args...);
#else
    fprintf(stderr, "[%s] ", logPriorityName(priority));
    fprintf(stderr, fmt, args...);
    fputc('\n', stderr);
#endif
}

template<typename... Args>
inline void log_error(const char *fmt, const Args &...args)
{
    log_message(LogPriority::Error, fmt, args...);
}

template<typename... Args>
inline void log_info(const char *fmt, const Args &...args)
{
    log_message(LogPriority::Info, fmt, args...);
}

template<typename... Args>
inline void log_debug(const char *fmt, const Args &...args)
{
    log_message(LogPriority::Debug, fmt, args...);
}
