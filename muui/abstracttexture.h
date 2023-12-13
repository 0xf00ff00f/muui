#pragma once

namespace muui
{

class AbstractTexture
{
public:
    virtual ~AbstractTexture() = default;

    virtual void bind(int textureUnit = 0) const = 0;
};

} // namespace muui
