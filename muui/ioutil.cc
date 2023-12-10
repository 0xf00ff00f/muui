#include "ioutil.h"

#ifdef USE_SDL
#include <SDL.h>
#else
#include <fstream>
#endif

namespace util
{

std::optional<std::vector<std::byte>> readFile(const std::string &path)
{
#ifdef USE_SDL
    SDL_RWops *rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw == nullptr)
        return std::nullopt;
    const auto size = SDL_RWsize(rw);
    std::vector<std::byte> data(size);
    SDL_RWread(rw, data.data(), data.size(), 1);
    SDL_RWclose(rw);
    return data;
#else
    std::ifstream file(path);
    if (!file.is_open())
        return {};

    auto *buf = file.rdbuf();

    const std::size_t size = buf->pubseekoff(0, file.end, file.in);
    buf->pubseekpos(0, file.in);

    std::vector<std::byte> data(size);
    buf->sgetn(reinterpret_cast<char *>(data.data()), size);

    return data;
#endif
}

} // namespace util
