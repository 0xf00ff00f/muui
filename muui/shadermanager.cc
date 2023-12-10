#include "shadermanager.h"

#include "assetpath.h"
#include "log.h"

#include <fmt/core.h>

#include <span>
#include <type_traits>

namespace
{
std::string shaderPath(std::string_view basename)
{
    return assetPath(fmt::format("shaders/{}", basename));
}

std::unique_ptr<gl::ShaderProgram> loadProgram(const char *vertexShader, const char *fragmentShader)
{
    auto program = std::make_unique<gl::ShaderProgram>();
    if (!program->addShader(GL_VERTEX_SHADER, shaderPath(vertexShader)))
    {
        log_error("Failed to add vertex shader for program %s: %s", vertexShader, program->log().c_str());
        return {};
    }
    if (!program->addShader(GL_FRAGMENT_SHADER, shaderPath(fragmentShader)))
    {
        log_error("Failed to add fragment shader for program %s: %s", fragmentShader, program->log().c_str());
        return {};
    }
    if (!program->link())
    {
        log_error("Failed to link program: %s", program->log().c_str());
        return {};
    }
    return program;
}

std::unique_ptr<gl::ShaderProgram> loadProgram(ShaderManager::Program id)
{
    struct ProgramSource
    {
        const char *vertexShader;
        const char *fragmentShader;
    };
    static const ProgramSource programSources[] = {
        // flat
        {"flat.vert", "flat.frag"},
        // text
        {"text.vert", "text.frag"},
        // decal
        {"decal.vert", "decal.frag"},
        // circle
        {"circle.vert", "circle.frag"},
        // rounded rect
        {"roundedrect.vert", "roundedrect.frag"},
    };
    static_assert(std::extent_v<decltype(programSources)> == static_cast<int>(ShaderManager::Program::NumPrograms),
                  "expected number of programs to match");

    const auto &sources = programSources[static_cast<int>(id)];
    return loadProgram(sources.vertexShader, sources.fragmentShader);
}

} // namespace

ShaderManager::ShaderManager() = default;
ShaderManager::~ShaderManager() = default;

void ShaderManager::useProgram(Program id)
{
    auto &cachedProgram = m_cachedPrograms[static_cast<int>(id)];
    if (!cachedProgram)
    {
        cachedProgram.reset(new CachedProgram);
        cachedProgram->program = loadProgram(id);
        auto &uniforms = cachedProgram->uniformLocations;
        std::fill(uniforms.begin(), uniforms.end(), -1);
    }
    if (cachedProgram.get() == m_currentProgram)
        return;
    if (cachedProgram->program)
        cachedProgram->program->bind();
    m_currentProgram = cachedProgram.get();
}

int ShaderManager::uniformLocation(Uniform id)
{
    if (!m_currentProgram || !m_currentProgram->program)
    {
        return -1;
    }
    auto location = m_currentProgram->uniformLocations[static_cast<int>(id)];
    if (location == -1)
    {
        static constexpr const char *uniformNames[] = {
            // clang-format off
            "mvp",
            "baseColorTexture",
            // clang-format on
        };
        static_assert(std::extent_v<decltype(uniformNames)> == static_cast<int>(Uniform::NumUniforms),
                      "expected number of uniforms to match");

        location = m_currentProgram->program->uniformLocation(uniformNames[static_cast<int>(id)]);
        m_currentProgram->uniformLocations[static_cast<int>(id)] = location;
    }
    return location;
}
