#include "shadermanager.h"

#include "log.h"

#include <fmt/core.h>

#include <span>
#include <type_traits>
#include <vector>

namespace muui
{

namespace
{
std::unique_ptr<gl::ShaderProgram> loadProgram(const ProgramDescription &description)
{
    auto program = std::make_unique<gl::ShaderProgram>();
    auto addShader = [program = program.get()](gl::Shader::Type type, const std::filesystem::path &path,
                                               const std::span<const ProgramDescription::Define> defines) {
        gl::Shader shader(type);
        if (!defines.empty())
        {
            std::string definesSource;
            for (const auto &define : defines)
                definesSource += "#define " + define.key + " " + define.value + "\n";
            shader.addSource(definesSource);
        }
        if (!shader.addSourceFromFile(path))
        {
            log_error("Failed to load shader {}", path.c_str());
            return false;
        }
        if (!shader.compile())
        {
            log_error("Failed to compile shader {}: {}", path.c_str(), shader.log());
            return false;
        }
        program->attach(std::move(shader));
        return true;
    };
    if (!addShader(gl::Shader::Type::Vertex, description.vertexShaderPath, description.defines))
    {
        return {};
    }
    if (!addShader(gl::Shader::Type::Fragment, description.fragmentShaderPath, description.defines))
    {
        return {};
    }
    if (!program->link())
    {
        log_error("Failed to link program: {}", program->log());
        return {};
    }
    return program;
}

} // namespace

ShaderManager::ShaderManager()
{
    addBasicPrograms();
}

ShaderManager::~ShaderManager() = default;

ShaderManager::ProgramHandle ShaderManager::addProgram(const ProgramDescription &description)
{
    auto program = loadProgram(description);
    if (!program)
        return ProgramHandle::Invalid;
    auto cachedProgram = std::make_unique<CachedProgram>();
    cachedProgram->description = description;
    cachedProgram->program = std::move(program);
    m_cachedPrograms.push_back(std::move(cachedProgram));
    return ProgramHandle{static_cast<int>(m_cachedPrograms.size()) - 1};
}

void ShaderManager::useProgram(ProgramHandle handle)
{
    if (handle == ProgramHandle::Invalid) {
        m_currentProgram = nullptr;
        return;
    }
    const auto index = static_cast<int>(handle);
    assert(index >= 0 && index < m_cachedPrograms.size());
    auto &cachedProgram = m_cachedPrograms[index];
    if (cachedProgram.get() == m_currentProgram)
        return;
    if (cachedProgram->program)
        cachedProgram->program->bind();
    m_currentProgram = cachedProgram.get();
}

int ShaderManager::uniformLocation(const std::string &uniform)
{
    if (!m_currentProgram || !m_currentProgram->program)
        return -1;
    auto &uniformLocations = m_currentProgram->uniformLocations;
    auto it = uniformLocations.find(uniform);
    if (it == uniformLocations.end())
    {
        auto location = m_currentProgram->program->uniformLocation(uniform.c_str());
        it = uniformLocations.emplace(uniform, location).first;
    }
    return it->second;
}

void ShaderManager::addBasicPrograms()
{
    struct Program
    {
        const char *vertexShader;
        const char *fragmentShader;
    };
    static const Program programSources[] = {
        {"copy.vert", "copy.frag"},
        {"flat.vert", "flat.frag"},
        {"decal.vert", "decal.frag"},
        {"circle.vert", "circle.frag"},
        {"roundedrect.vert", "roundedrect.frag"},
        {"text.vert", "text.frag"},
        {"text.vert", "textoutline.frag"},
        {"gradient.vert", "gradient.frag"},
        {"decalgradient.vert", "decalgradient.frag"},
        {"circlegradient.vert", "circlegradient.frag"},
        {"roundedrectgradient.vert", "roundedrectgradient.frag"},
        {"textgradient.vert", "textgradient.frag"},
        {"textgradient.vert", "textgradientoutline.frag"},
        {"gaussianblur.vert", "gaussianblur.frag"},
    };
    static_assert(std::extent_v<decltype(programSources)> == static_cast<int>(ProgramHandle::NumDefaultPrograms));

    assert(m_cachedPrograms.empty());
    m_cachedPrograms.reserve(std::size(programSources));
    for (const auto &program : programSources)
    {
        static const std::filesystem::path shaderRootPath{":/assets/shaders"};
        auto cachedProgram = std::make_unique<CachedProgram>();
        cachedProgram->description = {.vertexShaderPath = shaderRootPath / program.vertexShader,
                                      .fragmentShaderPath = shaderRootPath / program.fragmentShader};
        cachedProgram->program = loadProgram(cachedProgram->description);
        m_cachedPrograms.push_back(std::move(cachedProgram));
    }
}

} // namespace muui
