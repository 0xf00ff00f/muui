#include "vertexarray.h"

#include <utility>

namespace muui::gl
{

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_handle);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_handle);
}

VertexArray::VertexArray(VertexArray &&other)
    : m_handle{std::exchange(other.m_handle, 0)}
{
}

VertexArray &VertexArray::operator=(VertexArray other)
{
    swap(*this, other);
    return *this;
}

void swap(VertexArray &lhs, VertexArray &rhs)
{
    std::swap(lhs.m_handle, rhs.m_handle);
}

void VertexArray::bind() const
{
    glBindVertexArray(m_handle);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

} // namespace muui::gl
