#include "buffer.h"

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
    : m_type{other.m_type}
    , m_usage{other.m_usage}
    , m_handle{other.m_handle}
{
    other.m_type = 0;
    other.m_usage = 0;
    other.m_handle = 0;
}

Buffer &Buffer::operator=(Buffer &&other)
{
    m_type = other.m_type;
    m_usage = other.m_usage;
    m_handle = other.m_handle;

    other.m_type = 0;
    other.m_usage = 0;
    other.m_handle = 0;

    return *this;
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
