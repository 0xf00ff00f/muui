#pragma once

#include <span>

#include "gl.h"

namespace muui::gl
{

class Buffer
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

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&other);
    Buffer &operator=(Buffer &&other);

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

    explicit operator bool() const { return m_handle != 0; }

private:
    void allocate(std::size_t size, const std::byte *data) const;
    void initialize();

    GLenum m_type{0};
    GLenum m_usage{0};
    GLuint m_handle{0};
};

} // namespace muui::gl
