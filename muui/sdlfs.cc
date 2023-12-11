#include "sdlfs.h"

#include <SDL.h>

namespace muui
{

class SDLFileReader : public FileReader
{
public:
    explicit SDLFileReader(VFS *vfs, const std::filesystem::path &path)
        : FileReader(vfs)
        , m_rw(SDL_RWFromFile(path.c_str(), "rb"))
    {
    }

    ~SDLFileReader()
    {
        if (m_rw)
            SDL_RWclose(m_rw);
    }

    bool isOpen() const { return m_rw != nullptr; }

    std::size_t read(std::byte *data, std::size_t size) override { return SDL_RWread(m_rw, data, 1, size); }

    std::vector<std::byte> readAll() override
    {
        const auto length = SDL_RWsize(m_rw) - SDL_RWtell(m_rw);
        std::vector<std::byte> data(length);
        if (read(data.data(), data.size()) != data.size())
            return {};
        return data;
    }

    void skip(std::size_t size) override { SDL_RWseek(m_rw, size, RW_SEEK_CUR); }

    bool eof() const override { return SDL_RWtell(m_rw) == SDL_RWsize(m_rw); }

private:
    SDL_RWops *m_rw{nullptr};
};

std::unique_ptr<FileReader> SDLFS::open(const std::filesystem::path &path)
{
    auto file = std::make_unique<SDLFileReader>(this, path);
    if (!file->isOpen())
        return {};
    return file;
}

} // namespace muui
