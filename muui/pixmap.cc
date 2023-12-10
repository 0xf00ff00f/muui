#include "pixmap.h"

#ifdef USE_SDL
#include <SDL.h>
#endif

#include <stb_image.h>

#include <cassert>

Pixmap loadPixmap(const std::string &path, bool flip)
{
    if (flip)
        stbi_set_flip_vertically_on_load(1);

#ifdef USE_SDL
    SDL_RWops *rw = SDL_RWFromFile(path.c_str(), "rb");
    if (rw == nullptr)
        return {};

    static const stbi_io_callbacks callbacks = {.read = [](void *user, char *data, int size) -> int {
                                                    auto *rw = static_cast<SDL_RWops *>(user);
                                                    return SDL_RWread(rw, data, 1, size);
                                                },
                                                .skip = [](void *user, int n) -> void {
                                                    auto *rw = static_cast<SDL_RWops *>(user);
                                                    SDL_RWseek(rw, n, RW_SEEK_CUR);
                                                },
                                                .eof = [](void *user) -> int {
                                                    auto *rw = static_cast<SDL_RWops *>(user);
                                                    return SDL_RWtell(rw) == SDL_RWsize(rw);
                                                }};

    int width, height, channels;
    unsigned char *data = stbi_load_from_callbacks(&callbacks, rw, &width, &height, &channels, 4);
    if (!data)
        return {};

    SDL_RWclose(rw);

    Pixmap pm;
    pm.width = width;
    pm.height = height;
    pm.pixelType = PixelType::RGBA;
    pm.pixels.assign(data, data + width * height * 4);

    stbi_image_free(data);

    return pm;
#else
    return {};
#endif
}
