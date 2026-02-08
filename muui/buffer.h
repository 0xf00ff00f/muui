#pragma once

#include <span>

#include "gl.h"

namespace muui::gl
{

class Buffer
{
public:
    enum class Type : unsigned
    {
        Vertex = GL_ARRAY_BUFFER,
        Index = GL_ELEMENT_ARRAY_BUFFER
    };

    enum class Usage : unsigned
    {
        StaticDraw = GL_STATIC_DRAW,
        DynamicDraw = GL_DYNAMIC_DRAW,
        StreamDraw = GL_STREAM_DRAW
    };

    enum class Access : unsigned
    {
        Read = GL_MAP_READ_BIT,
        Write = GL_MAP_WRITE_BIT,
        Unsynchronized = GL_MAP_UNSYNCHRONIZED_BIT
    };

    explicit Buffer(Type type = Type::Vertex, Usage usage = Usage::StaticDraw);
    ~Buffer();

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&other);
    Buffer &operator=(Buffer &&other);

    friend void swap(Buffer &lhs, Buffer &rhs);

    void bind() const;
    void allocate(std::size_t size) const;
    void allocate(std::span<const std::byte> data) const;
    void write(std::size_t offset, std::span<const std::byte> data) const;

    template<typename T>
    T *mapRange(std::size_t offset, std::size_t length, Access access)
    {
        return static_cast<T *>(
            glMapBufferRange(m_type, offset * sizeof(T), length * sizeof(T), static_cast<GLbitfield>(access)));
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

constexpr Buffer::Access operator&(Buffer::Access x, Buffer::Access y)
{
    return static_cast<Buffer::Access>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
}

constexpr Buffer::Access &operator&=(Buffer::Access &x, Buffer::Access y)
{
    return x = x & y;
}

constexpr Buffer::Access operator|(Buffer::Access x, Buffer::Access y)
{
    return static_cast<Buffer::Access>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
}

constexpr Buffer::Access &operator|=(Buffer::Access &x, Buffer::Access y)
{
    return x = x | y;
}

} // namespace muui::gl
