#pragma once

#include <cstdio>

enum class LogPriority
{
    Error,
    Info,
    Debug
};

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

inline void log_message(LogPriority priority, const char *fmt)
{
    fprintf(stderr, "[%s] %s\n", logPriorityName(priority), fmt);
}

template<typename... Args>
inline void log_message(LogPriority priority, const char *fmt, const Args &...args)
{
    fprintf(stderr, "[%s] ", logPriorityName(priority));
    fprintf(stderr, fmt, args...);
    fputc('\n', stderr);
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
