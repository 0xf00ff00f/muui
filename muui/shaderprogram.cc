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

ShaderProgram::ShaderProgram()
    : m_id{glCreateProgram()}
{
}

ShaderProgram::~ShaderProgram()
{
    if (m_id)
        glDeleteProgram(m_id);
}

ShaderProgram::ShaderProgram(ShaderProgram &&other)
    : m_id(other.m_id)
    , m_log(std::move(other.m_log))
{
    other.m_id = 0;
    other.m_log.clear();
}

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other)
{
    m_id = other.m_id;
    m_log = std::move(other.m_log);

    other.m_id = 0;
    other.m_log.clear();

    return *this;
}

bool ShaderProgram::addShader(GLenum type, const std::filesystem::path &path)
{
    const auto source = readShaderSource(path);
    if (!source)
    {
        m_log = fmt::format("failed to load {}", path.string());
        return false;
    }
    return addShaderSource(type, *source);
}

bool ShaderProgram::addShaderSource(GLenum type, std::string_view source)
{
    return compileAndAttachShader(type, source);
}

bool ShaderProgram::compileAndAttachShader(GLenum type, std::string_view source)
{
    const auto shader = glCreateShader(type);

    const std::array sources = {source.data()};
    const std::array lengths = {static_cast<GLint>(source.size())};
    static_assert(sources.size() == lengths.size());
    glShaderSource(shader, sources.size(), sources.data(), lengths.data());
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        m_log.clear();
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        if (length > 1)
        {
            m_log.resize(length);
            glGetShaderInfoLog(shader, length, nullptr, m_log.data());
        }
        return false;
    }

    glAttachShader(m_id, shader);

    return true;
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
