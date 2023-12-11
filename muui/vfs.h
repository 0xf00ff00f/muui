#pragma once

#include <filesystem>
#include <vector>

namespace muui
{

class VFS
{
public:
    virtual ~VFS() = default;

    struct File
    {
        File(VFS *vfs)
            : m_vfs(vfs)
        {
        }
        virtual ~File() = default;

        VFS *vfs() const { return m_vfs; }
        virtual std::size_t read(std::byte *data, std::size_t size) = 0;
        virtual std::vector<std::byte> readAll() = 0;
        virtual bool skip(std::size_t size) = 0;
        virtual bool eof() const = 0;

    private:
        VFS *m_vfs;
    };

    virtual std::unique_ptr<File> open(const std::filesystem::path &path) = 0;
};

}; // namespace muui
