#include "pixmap.h"

#include "file.h"

#include <stb_image.h>

#include <cassert>

namespace muui
{

Pixmap loadPixmap(const std::filesystem::path &path, bool flip)
{
    if (flip)
        stbi_set_flip_vertically_on_load(1);

    File file(path);
    if (!file)
        return {};

    static const stbi_io_callbacks callbacks = {.read = [](void *user, char *data, int size) -> int {
                                                    auto *file = static_cast<File *>(user);
                                                    return file->read(reinterpret_cast<std::byte *>(data), size);
                                                },
                                                .skip = [](void *user, int n) -> void {
                                                    auto *file = static_cast<File *>(user);
                                                    file->skip(n);
                                                },
                                                .eof = [](void *user) -> int {
                                                    auto *file = static_cast<File *>(user);
                                                    return file->eof();
                                                }};

    int width, height, channels;
    unsigned char *data = stbi_load_from_callbacks(&callbacks, &file, &width, &height, &channels, 4);
    if (!data)
        return {};

    Pixmap pm;
    pm.width = width;
    pm.height = height;
    pm.pixelType = PixelType::RGBA;
    pm.pixels.assign(data, data + width * height * 4);

    stbi_image_free(data);

    return pm;
}

} // namespace muui
