#pragma once

#include <glm/glm.hpp>

#include <variant>

namespace muui
{

class GradientTexture;

struct LinearGradient
{
    GradientTexture *texture{nullptr};
    glm::vec2 start{};
    glm::vec2 end{};

    bool operator==(const LinearGradient &) const = default;
};

using Color = glm::vec4;

using Brush = std::variant<Color, LinearGradient>;

} // namespace muui
