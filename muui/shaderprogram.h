#pragma once

#include "gl.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace muui::gl
{

class ShaderProgram
{
public:
    ShaderProgram();
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram &) = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;

    ShaderProgram(ShaderProgram &&other);
    ShaderProgram &operator=(ShaderProgram &&other);

    bool addShader(GLenum type, const std::filesystem::path &path);
    bool addShaderSource(GLenum type, std::string_view source);
    bool link();
    const std::string &log() const { return m_log; }

    void bind() const;

    int uniformLocation(const char *name) const;
    int attributeLocation(const char *name) const;
    void bindAttributeLocation(std::size_t index, const char *name) const;

    void setUniform(int location, int v) const;
    void setUniform(int location, float v) const;
    void setUniform(int location, const glm::vec2 &v) const;
    void setUniform(int location, const glm::vec3 &v) const;
    void setUniform(int location, const glm::vec4 &v) const;

    void setUniform(int location, const std::vector<float> &v) const;
    void setUniform(int location, const std::vector<glm::vec2> &v) const;
    void setUniform(int location, const std::vector<glm::vec3> &v) const;
    void setUniform(int location, const std::vector<glm::vec4> &v) const;

    void setUniform(int location, const glm::mat3 &mat) const;
    void setUniform(int location, const glm::mat4 &mat) const;

    GLuint id() const { return m_id; }

    explicit operator bool() const { return m_id != 0; }

private:
    bool compileAndAttachShader(GLenum type, std::string_view source);

    GLuint m_id{0};
    std::string m_log;
};

} // namespace muui::gl
