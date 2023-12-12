#include "file.h"

#include "diskfs.h"
#include "resourcefs.h"

CMRC_DECLARE(assets);

namespace muui
{

File::File(const std::filesystem::path &path)
    : m_path(path)
{
    static const std::filesystem::path resourceRoot{":"};
    if (!path.empty())
    {
        if (*path.begin() == resourceRoot)
        {
            static ResourceFS fs{cmrc::assets::get_filesystem()};
            m_reader = fs.open(std::filesystem::relative(path, resourceRoot));
        }
        else
        {
            static DiskFS fs;
            m_reader = fs.open(path);
        }
    }
}

File::~File() = default;

VFS *File::vfs() const
{
    if (!m_reader)
        return nullptr;
    return m_reader->vfs();
}

std::size_t File::read(std::byte *data, std::size_t size)
{
    if (!m_reader)
        return 0;
    return m_reader->read(data, size);
}

std::vector<std::byte> File::readAll()
{
    if (!m_reader)
        return {};
    return m_reader->readAll();
}

bool File::skip(std::size_t size)
{
    if (!m_reader)
        return false;
    return m_reader->skip(size);
}

bool File::eof() const
{
    if (!m_reader)
        return true;
    return m_reader->eof();
}

File::operator bool() const
{
    return m_reader.operator bool();
}

} // namespace muui