#pragma once

#include <filesystem>
#include <vector>

namespace muui
{

class VFS;

class FileReader
{
public:
    FileReader(VFS *vfs)
        : m_vfs(vfs)
    {
    }
    virtual ~FileReader() = default;

    VFS *vfs() const { return m_vfs; }
    virtual std::size_t read(std::byte *data, std::size_t size) = 0;
    virtual std::vector<std::byte> readAll() = 0;
    virtual bool skip(std::size_t size) = 0;
    virtual bool eof() const = 0;

private:
    VFS *m_vfs;
};

class VFS
{
public:
    virtual ~VFS() = default;

    virtual std::unique_ptr<FileReader> open(const std::filesystem::path &path) = 0;
};

}; // namespace muui
