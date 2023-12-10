#pragma once

#include "noncopyable.h"

namespace muui
{

class AbstractTexture : private NonCopyable
{
public:
    virtual ~AbstractTexture() = default;

    virtual void bind(int textureUnit = 0) const = 0;
};

} // namespace muui
