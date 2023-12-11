#include "diskfs.h"

#include <cstdio>
#include <memory>

namespace muui
{

namespace
{

class DiskFile : public VFS::File
{
public:
    explicit DiskFile(VFS *vfs, const std::filesystem::path &path)
        : VFS::File(vfs)
        , m_stream(fopen(path.c_str(), "rb"))
    {
    }

    ~DiskFile() override
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

} // namespace

std::unique_ptr<VFS::File> DiskFS::open(const std::filesystem::path &path)
{
    auto file = std::make_unique<DiskFile>(this, path);
    if (!file->isOpen())
        return {};
    return file;
}

} // namespace muui
