#pragma once

#include "buffer.h"
#include "gl.h"
#include "vertexarray.h"

#include <memory>
#include <span>

namespace muui::gl
{

enum class Primitive
{
    Lines = GL_LINES,
    LineStrip = GL_LINE_STRIP,
    Triangles = GL_TRIANGLES,
    TriangleStrip = GL_TRIANGLE_STRIP,
    TriangleFan = GL_TRIANGLE_FAN
};

template<typename VertexT>
class Mesh
{
public:
    Mesh();

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    Mesh(Mesh &&other);
    Mesh &operator=(Mesh &&other);

    void setData(std::span<const VertexT> data);
    void render(Primitive primitive) const;

private:
    std::size_t m_vertexCount{0};
    Buffer m_buffer;
    VertexArray m_vao;
};

template<typename IndexT, typename VertexT>
class IndexedMesh
{
public:
    IndexedMesh();

    IndexedMesh(const IndexedMesh &) = delete;
    IndexedMesh &operator=(const IndexedMesh &) = delete;

    IndexedMesh(IndexedMesh &&other);
    IndexedMesh &operator=(IndexedMesh &&other);

    void setVertexData(std::span<const VertexT> data);
    void setIndexData(std::span<const IndexT> data);
    void render(Primitive primitive) const;

private:
    std::size_t m_vertexCount{0};
    Buffer m_vertexBuffer;
    std::size_t m_indexCount{0};
    Buffer m_indexBuffer;
    VertexArray m_vao;
};

} // namespace muui::gl

#include "mesh.inl"
