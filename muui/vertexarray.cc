#include "vertexarray.h"

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
    : m_handle{other.m_handle}
{
    other.m_handle = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&other)
{
    m_handle = other.m_handle;

    other.m_handle = 0;

    return *this;
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
