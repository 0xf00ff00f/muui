#include "diskfs.h"

#include <cstdio>
#include <memory>

namespace muui
{

class DiskFileReader : public FileReader
{
public:
    explicit DiskFileReader(VFS *vfs, const std::filesystem::path &path)
        : FileReader(vfs)
        , m_stream(fopen(path.c_str(), "rb"))
    {
    }

    ~DiskFileReader() override
    {
        if (m_stream)
            fclose(m_stream);
    }

    bool isOpen() const { return m_stream != nullptr; }

    std::size_t read(std::byte *data, std::size_t size) override { return fread(data, 1, size, m_stream); }

    std::vector<std::byte> readAll() override
    {
        const auto cur = ftell(m_stream);
        fseek(m_stream, 0l, SEEK_END);
        const auto size = ftell(m_stream);
        fseek(m_stream, cur, SEEK_SET);
        const auto length = size - cur;
        std::vector<std::byte> data(length);
        if (read(data.data(), data.size()) != data.size())
            return {};
        return data;
    }

    bool skip(std::size_t size) override { return fseek(m_stream, size, SEEK_CUR) == 0; }

    bool eof() const override { return feof(m_stream); }

private:
    FILE *m_stream{nullptr};
};

std::unique_ptr<FileReader> DiskFS::open(const std::filesystem::path &path)
{
    auto file = std::make_unique<DiskFileReader>(this, path);
    if (!file->isOpen())
        return {};
    return file;
}

} // namespace muui
