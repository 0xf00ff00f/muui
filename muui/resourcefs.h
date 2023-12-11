#include "vfs.h"

#include <cmrc/cmrc.hpp>

namespace muui
{

class ResourceFS : public VFS
{
public:
    ResourceFS(const cmrc::embedded_filesystem &fs);

    std::unique_ptr<FileReader> open(const std::filesystem::path &path) override;

private:
    cmrc::embedded_filesystem m_fs;
};

} // namespace muui
