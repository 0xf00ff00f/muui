#pragma once

#include <optional>
#include <string>
#include <vector>

namespace util
{
std::optional<std::vector<std::byte>> readFile(const std::string &path);
} // namespace util
