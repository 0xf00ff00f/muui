#include "shadermanager.h"

#include "log.h"

#include <fmt/core.h>

#include <optional>
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
    if (!program->addShader(GL_VERTEX_SHADER, description.vertexShaderPath))
    {
        log_error("Failed to add vertex shader for program %s: %s", description.vertexShaderPath.c_str(),
                  program->log().c_str());
        return {};
    }
    if (!program->addShader(GL_FRAGMENT_SHADER, description.fragmentShaderPath))
    {
        log_error("Failed to add fragment shader for program %s: %s", description.fragmentShaderPath.c_str(),
                  program->log().c_str());
        return {};
    }
    if (!program->link())
    {
        log_error("Failed to link program: %s", program->log().c_str());
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
        return InvalidProgram;
    auto cachedProgram = std::make_unique<CachedProgram>();
    cachedProgram->description = description;
    cachedProgram->program = std::move(program);
    m_cachedPrograms.push_back(std::move(cachedProgram));
    return static_cast<ProgramHandle>(m_cachedPrograms.size() - 1);
}

void ShaderManager::useProgram(ProgramHandle handle)
{
    assert(handle >= 0 && handle < m_cachedPrograms.size());
    auto &cachedProgram = m_cachedPrograms[handle];
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
        it = uniformLocations.insert(it, {uniform, location});
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
    static const std::vector<Program> programs = {{"flat.vert", "flat.frag"},
                                                  {"decal.vert", "decal.frag"},
                                                  {"circle.vert", "circle.frag"},
                                                  {"roundedrect.vert", "roundedrect.frag"},
                                                  {"text.vert", "text.frag"}};
    assert(m_cachedPrograms.empty());
    m_cachedPrograms.reserve(programs.size());
    for (const auto &program : programs)
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
