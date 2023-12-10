#include "assetpath.h"

#include <fmt/core.h>

std::string assetPath(std::string_view name)
{
    return fmt::format("{}{}", ASSETSDIR, name);
}
