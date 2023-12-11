#include "resourcefs.h"

#include <algorithm>
#include <cassert>

namespace muui
{

class ResourceFileReader : public FileReader
{
public:
    ResourceFileReader(VFS *vfs, const cmrc::file &file)
        : FileReader(vfs)
        , m_file(file)
    {
    }

    std::size_t read(std::byte *data, std::size_t size) override
    {
        assert(m_index <= m_file.size());
        const auto length = std::min(m_file.size() - m_index, size);
        std::copy(begin() + m_index, end() + m_index + length, data);
        m_index += length;
        return length;
    }

    std::vector<std::byte> readAll() override
    {
        assert(m_index <= m_file.size());
        std::vector<std::byte> data(begin() + m_index, end());
        m_index = m_file.size();
        return data;
    }

    void skip(std::size_t size) override
    {
        if (eof())
            return;
        assert(m_index <= m_file.size());
        const auto length = std::min(m_file.size() - m_index, size);
        m_index += length;
    }

    bool eof() const override { return m_index >= m_file.size(); }

private:
    const std::byte *begin() const { return reinterpret_cast<const std::byte *>(m_file.begin()); }

    const std::byte *end() const { return reinterpret_cast<const std::byte *>(m_file.end()); }

    cmrc::file m_file;
    std::size_t m_index{0};
};

ResourceFS::ResourceFS(const cmrc::embedded_filesystem &fs)
    : m_fs(fs)
{
}

std::unique_ptr<FileReader> ResourceFS::open(const std::filesystem::path &path)
{
    if (!m_fs.is_file(path.string()))
        return {};
    return std::make_unique<ResourceFileReader>(this, m_fs.open(path.string()));
}

} // namespace muui
