#pragma once

#include "noncopyable.h"
#include "shaderprogram.h"

#include <array>
#include <memory>
#include <optional>

class Connection;

namespace gl
{
class ShaderProgram;
}

namespace muui
{

struct ProgramDescription
{
    std::filesystem::path vertexShaderPath;
    std::filesystem::path fragmentShaderPath;
};

class ShaderManager : private NonCopyable
{
public:
    ShaderManager();
    ~ShaderManager();

    using ProgramHandle = int;

    static constexpr ProgramHandle InvalidProgram = -1;

    static constexpr ProgramHandle ProgramCopy = 0;
    static constexpr ProgramHandle ProgramFlat = 1;
    static constexpr ProgramHandle ProgramDecal = 2;
    static constexpr ProgramHandle ProgramCircle = 3;
    static constexpr ProgramHandle ProgramRoundedRect = 4;
    static constexpr ProgramHandle ProgramText = 5;
    static constexpr ProgramHandle ProgramGradient = 6;
    static constexpr ProgramHandle ProgramDecalGradient = 7;
    static constexpr ProgramHandle ProgramCircleGradient = 8;
    static constexpr ProgramHandle ProgramRoundedRectGradient = 9;
    static constexpr ProgramHandle ProgramTextGradient = 10;
    static constexpr ProgramHandle ProgramGaussianBlur = 11;

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
