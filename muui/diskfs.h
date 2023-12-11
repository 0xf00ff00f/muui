#include "vfs.h"

namespace muui
{

class DiskFS : public VFS
{
public:
    std::unique_ptr<FileReader> open(const std::filesystem::path &path) override;
};

} // namespace muui
