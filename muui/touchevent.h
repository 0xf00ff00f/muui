#pragma once

#include <glm/glm.hpp>

namespace muui
{

struct TouchEvent
{
    enum class Type
    {
        Press,
        Release,
        Click,
        DragBegin,
        DragMove,
        DragEnd,
    };
    Type type;
    glm::vec2 position;
};

} // namespace muui
