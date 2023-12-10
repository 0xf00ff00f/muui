#include <cstdio>
#include <cstdlib>

void panic(const char *fmt)
{
    std::fprintf(stderr, "%s", fmt);
    std::abort();
}

template<typename... Args>
void panic(const char *fmt, const Args &...args)
{
    std::fprintf(stderr, fmt, args...);
    std::abort();
}
