#include "buffer.h"

#include <utility>

namespace muui::gl
{

Buffer::Buffer(Type type, Usage usage)
    : m_type(static_cast<GLenum>(type))
    , m_usage(static_cast<GLenum>(usage))
{
    glGenBuffers(1, &m_handle);
}

Buffer::~Buffer()
{
    if (m_handle)
        glDeleteBuffers(1, &m_handle);
}

Buffer::Buffer(Buffer &&other)
    : m_type{std::exchange(other.m_type, 0)}
    , m_usage{std::exchange(other.m_usage, 0)}
    , m_handle{std::exchange(other.m_handle, 0)}
{
}

Buffer &Buffer::operator=(Buffer other)
{
    swap(*this, other);
    return *this;
}

void swap(Buffer &lhs, Buffer &rhs)
{
    using std::swap;
    swap(lhs.m_type, rhs.m_type);
    swap(lhs.m_usage, rhs.m_usage);
    swap(lhs.m_handle, rhs.m_handle);
}

void Buffer::bind() const
{
    glBindBuffer(m_type, m_handle);
}

void Buffer::allocate(std::size_t size) const
{
    allocate(size, nullptr);
}

void Buffer::allocate(std::span<const std::byte> data) const
{
    allocate(data.size(), data.data());
}

void Buffer::allocate(std::size_t size, const std::byte *data) const
{
    glBufferData(m_type, size, data, m_usage);
}

void Buffer::write(std::size_t offset, std::span<const std::byte> data) const
{
    glBufferSubData(m_type, offset, data.size(), data.data());
}

void Buffer::unmap()
{
    glUnmapBuffer(m_type);
}

} // namespace muui::gl
