#include "shaderprogram.h"

#include "file.h"

#include <fmt/core.h>

#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cassert>
#include <memory>
#include <optional>
#include <regex>
#include <streambuf>
#include <utility>

namespace muui::gl
{

namespace
{

class FileStreamBuf : public std::streambuf
{
public:
    FileStreamBuf(const std::filesystem::path &path)
        : m_file{File(path)}
    {
    }

    explicit operator bool() const { return m_file.operator bool(); }

private:
    int_type underflow() override
    {
        if (gptr() == egptr())
        {
            const auto read = m_file.read(reinterpret_cast<std::byte *>(m_buffer.data()), m_buffer.size());
            if (read > 0)
                setg(m_buffer.data(), m_buffer.data(), m_buffer.data() + read);
        }
        if (gptr() == egptr())
            return traits_type::eof();
        return traits_type::to_int_type(*gptr());
    }

    File m_file;
    std::array<char, 1024> m_buffer;
};

class FileStream : public std::istream
{
public:
    FileStream(const std::filesystem::path &path)
        : m_sb{path}
        , std::istream{&m_sb}
    {
        if (!m_sb)
            clear(failbit);
    }

private:
    FileStreamBuf m_sb;
};

std::optional<std::string> readShaderSource(const std::filesystem::path &path)
{
    FileStream is(path);
    if (!is)
        return {};

    static const std::regex includeRegex(R"!(^#include "([^"]+)"$)!");

    std::string source;

    for (std::string line; std::getline(is, line);)
    {
        if (std::smatch match; std::regex_match(line, match, includeRegex))
        {
            assert(match.size() == 2);
            const auto includeFile = match[1].str();
            const auto includePath = path.parent_path() / includeFile;
            const auto includeSource = readShaderSource(includePath.string());
            if (!includeSource)
                return {};
            source.append(*includeSource);
        }
        else
        {
            source.append(line);
            source.push_back('\n');
        }
    }

    return source;
}

} // namespace

Shader::Shader(Type type)
    : m_type{type}
    , m_id{glCreateShader(static_cast<GLenum>(type))}
{
}

Shader::~Shader()
{
    if (m_id)
        glDeleteShader(m_id);
}

Shader::Shader(Shader &&other)
    : m_type{std::exchange(other.m_type, Type::Invalid)}
    , m_id{std::exchange(other.m_id, 0)}
    , m_sources{std::move(other.m_sources)}
    , m_log{std::move(other.m_log)}
{
    other.m_sources.clear();
    other.m_log.clear();
}

Shader &Shader::operator=(Shader other)
{
    swap(*this, other);
    return *this;
}

void swap(Shader &lhs, Shader &rhs)
{
    using std::swap;
    swap(lhs.m_type, rhs.m_type);
    swap(lhs.m_id, rhs.m_id);
    swap(lhs.m_sources, rhs.m_sources);
    swap(lhs.m_log, rhs.m_log);
}

bool Shader::addSourceFromFile(const std::filesystem::path &path)
{
    auto source = readShaderSource(path);
    if (!source)
    {
        m_log = fmt::format("Failed to load {}", path.string());
        return false;
    }
    m_sources.push_back(std::move(*source));
    return true;
}

void Shader::addSource(std::string_view source)
{
    m_sources.emplace_back(source);
}

bool Shader::compile()
{
    static const std::string VersionString{"#version 300 es\n"};

    std::vector<const GLchar *> strings{VersionString.data()};
    std::transform(m_sources.begin(), m_sources.end(), std::back_inserter(strings),
                   [](const auto &source) { return static_cast<const GLchar *>(source.data()); });

    std::vector<GLint> lengths{static_cast<GLint>(VersionString.size())};
    std::transform(m_sources.begin(), m_sources.end(), std::back_inserter(lengths),
                   [](const auto &source) { return static_cast<GLint>(source.size()); });

    assert(strings.size() == lengths.size());
    glShaderSource(m_id, strings.size(), strings.data(), lengths.data());
    glCompileShader(m_id);

    GLint status;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_log.clear();
        GLint length = 0;
        glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &length);
        if (length > 1)
        {
            m_log.resize(length);
            glGetShaderInfoLog(m_id, length, nullptr, m_log.data());
        }
        return false;
    }

    return true;
}

ShaderProgram::ShaderProgram()
    : m_id{glCreateProgram()}
{
}

ShaderProgram::~ShaderProgram()
{
    if (m_id)
    {
        for (auto &shader : m_attachedShaders)
            glDetachShader(m_id, shader.id());
        glDeleteProgram(m_id);
    }
}

ShaderProgram::ShaderProgram(ShaderProgram &&other)
    : m_id(std::exchange(other.m_id, 0))
    , m_log(std::move(other.m_log))
    , m_attachedShaders(std::move(other.m_attachedShaders))
{
    other.m_log.clear();
    other.m_attachedShaders.clear();
}

ShaderProgram &ShaderProgram::operator=(ShaderProgram other)
{
    swap(*this, other);
    return *this;
}

void swap(ShaderProgram &lhs, ShaderProgram &rhs)
{
    using std::swap;
    swap(lhs.m_id, rhs.m_id);
    swap(lhs.m_log, rhs.m_log);
    swap(lhs.m_attachedShaders, rhs.m_attachedShaders);
}

bool ShaderProgram::addShader(Shader::Type type, const std::filesystem::path &path)
{
    Shader shader{type};
    if (!shader.addSourceFromFile(path))
    {
        m_log = fmt::format("failed to load {}", path.string());
        return false;
    }
    if (!shader.compile())
    {
        m_log = shader.log();
        return false;
    }
    attach(std::move(shader));
    return true;
}

bool ShaderProgram::addShaderSource(Shader::Type type, std::string_view source)
{
    Shader shader{type};
    shader.addSource(source);
    if (!shader.compile())
    {
        m_log = shader.log();
        return false;
    }
    attach(std::move(shader));
    return true;
}

void ShaderProgram::attach(Shader &&shader)
{
    glAttachShader(m_id, shader.id());
    m_attachedShaders.push_back(std::move(shader));
}

bool ShaderProgram::link()
{
    glLinkProgram(m_id);

    GLint status;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_log.clear();
        GLint length;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);
        if (length > 1)
        {
            m_log.resize(length);
            glGetProgramInfoLog(m_id, length, nullptr, m_log.data());
        }
        return false;
    }

    return true;
}

void ShaderProgram::bind() const
{
    glUseProgram(m_id);
}

int ShaderProgram::uniformLocation(const char *name) const
{
    return glGetUniformLocation(m_id, name);
}

int ShaderProgram::attributeLocation(const char *name) const
{
    return glGetAttribLocation(m_id, name);
}

void ShaderProgram::bindAttributeLocation(std::size_t index, const char *name) const
{
    glBindAttribLocation(m_id, index, name);
}

void ShaderProgram::setUniform(int location, int value) const
{
    glUniform1i(location, value);
}

void ShaderProgram::setUniform(int location, float value) const
{
    glUniform1f(location, value);
}

void ShaderProgram::setUniform(int location, const glm::vec2 &value) const
{
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec3 &value) const
{
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::vec4 &value) const
{
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const std::vector<float> &value) const
{
    glUniform1fv(location, value.size(), value.data());
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec2> &value) const
{
    glUniform2fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec3> &value) const
{
    glUniform3fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const std::vector<glm::vec4> &value) const
{
    glUniform4fv(location, value.size(), reinterpret_cast<const float *>(value.data()));
}

void ShaderProgram::setUniform(int location, const glm::mat3 &value) const
{
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setUniform(int location, const glm::mat4 &value) const
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace muui::gl
