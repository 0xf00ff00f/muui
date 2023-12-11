#pragma once

#include "noncopyable.h"

#include "gl.h"
#include "vfs.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace muui::gl
{

class ShaderProgram : private NonCopyable
{
public:
    ShaderProgram();
    ~ShaderProgram();

    bool addShader(GLenum type, VFS::File *file);
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

private:
    void initialize();
    bool compileAndAttachShader(GLenum type, std::string_view source);

    GLuint m_id = 0;
    std::string m_log;
};

} // namespace gl
