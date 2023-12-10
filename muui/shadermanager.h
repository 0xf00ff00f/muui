#pragma once

#include "noncopyable.h"
#include "shaderprogram.h"

#include <array>
#include <memory>

class Connection;

namespace gl
{
class ShaderProgram;
}

namespace muui
{

class ShaderManager : private NonCopyable
{
public:
    ShaderManager();
    ~ShaderManager();

    enum class Program
    {
        Flat,
        Text,
        Decal,
        Circle,
        RoundedRect,
        NumPrograms
    };
    void useProgram(Program program);

    enum class Uniform
    {
        ModelViewProjection,
        BaseColorTexture,
        NumUniforms
    };

    template<typename T>
    void setUniform(Uniform uniform, T &&value)
    {
        if (!m_currentProgram)
            return;
        const auto location = uniformLocation(uniform);
        if (location == -1)
            return;
        m_currentProgram->program->setUniform(location, std::forward<T>(value));
    }

    const gl::ShaderProgram *currentProgram() const
    {
        return m_currentProgram ? m_currentProgram->program.get() : nullptr;
    }

private:
    int uniformLocation(Uniform uniform);

    struct CachedProgram
    {
        std::unique_ptr<gl::ShaderProgram> program;
        std::array<int, static_cast<int>(Uniform::NumUniforms)> uniformLocations;
    };
    std::array<std::unique_ptr<CachedProgram>, static_cast<int>(Program::NumPrograms)> m_cachedPrograms;
    CachedProgram *m_currentProgram = nullptr;
};

} // namespace muui
