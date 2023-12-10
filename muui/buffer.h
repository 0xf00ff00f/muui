#pragma once

#include "noncopyable.h"

#include <span>

#include "gl.h"

namespace muui::gl
{

class Buffer : private NonCopyable
{
public:
    enum class Type
    {
        Vertex,
        Index
    };

    enum class Usage
    {
        StaticDraw,
        DynamicDraw,
        StreamDraw,
    };

    explicit Buffer(Type type = Type::Vertex, Usage usage = Usage::StaticDraw);
    ~Buffer();

    void bind() const;
    void allocate(std::size_t size) const;
    void allocate(std::span<const std::byte> data) const;
    void write(std::size_t offset, std::span<const std::byte> data) const;

    // TODO: enum class for access bits?
    template<typename T>
    T *mapRange(std::size_t offset, std::size_t length, GLbitfield access)
    {
        return static_cast<T *>(glMapBufferRange(m_type, offset * sizeof(T), length * sizeof(T), access));
    }
    void unmap();

    GLuint handle() const { return m_handle; }

private:
    void allocate(std::size_t size, const std::byte *data) const;
    void initialize();

    GLenum m_type;
    GLenum m_usage;
    GLuint m_handle = 0;
};

} // namespace gl
