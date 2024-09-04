#pragma once

#include "noncopyable.h"
#include "shaderprogram.h"

#include <memory>

class Connection;

namespace muui
{

namespace gl
{
class ShaderProgram;
}

struct ProgramDescription
{
    struct Define
    {
        std::string key;
        std::string value;
    };
    std::vector<Define> defines;
    std::filesystem::path vertexShaderPath;
    std::filesystem::path fragmentShaderPath;
};

class ShaderManager : private NonCopyable
{
public:
    ShaderManager();
    ~ShaderManager();

    enum class ProgramHandle : int
    {
        Invalid = -1,

        Copy = 0,
        Flat,
        Decal,
        Circle,
        RoundedRect,
        Text,
        TextOutline,
        Gradient,
        DecalGradient,
        CircleGradient,
        RoundedRectGradient,
        TextGradient,
        TextGradientOutline,
        GaussianBlur,

        NumDefaultPrograms,
    };

    ProgramHandle addProgram(const ProgramDescription &description);

    void useProgram(ProgramHandle handle);

    template<typename T>
    void setUniform(const std::string &uniform, T &&value)
    {
        if (!m_currentProgram)
            return;
        const auto location = uniformLocation(uniform);
        if (location == -1)
            return;
        if (m_currentProgram->program)
            m_currentProgram->program->setUniform(location, std::forward<T>(value));
    }

    const gl::ShaderProgram *currentProgram() const
    {
        return m_currentProgram ? m_currentProgram->program.get() : nullptr;
    }

private:
    void addBasicPrograms();
    int uniformLocation(const std::string &uniform);

    struct CachedProgram
    {
        ProgramDescription description;
        std::unique_ptr<gl::ShaderProgram> program;
        std::unordered_map<std::string, int> uniformLocations;
    };
    std::vector<std::unique_ptr<CachedProgram>> m_cachedPrograms;
    CachedProgram *m_currentProgram = nullptr;
};

} // namespace muui
