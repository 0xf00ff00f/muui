#pragma once

#include <filesystem>
#include <memory>
#include <vector>

namespace muui
{

class FileReader;
class VFS;

class File
{
public:
    explicit File(const std::filesystem::path &path);
    ~File();

    operator bool() const;
    const std::filesystem::path &path() const { return m_path; }
    VFS *vfs() const;
    std::size_t read(std::byte *data, std::size_t size);
    std::vector<std::byte> readAll();
    bool skip(std::size_t size);
    bool eof() const;

private:
    std::filesystem::path m_path;
    std::unique_ptr<FileReader> m_reader;
};

} // namespace muui