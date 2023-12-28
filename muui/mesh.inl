#include <concepts>

#include <glm/glm.hpp>

#include "reflect.h"

namespace muui::gl
{

namespace detail
{
template<typename T>
concept Scalar = std::is_integral_v<T> || std::is_floating_point_v<T>;

template<typename T>
struct GLSize;

template<Scalar T>
struct GLSize<T>
{
    static constexpr GLint value = 1;
};

template<glm::length_t L, typename T, glm::qualifier Q>
struct GLSize<glm::vec<L, T, Q>>
{
    static constexpr GLint value = L;
};

template<typename T>
struct GLType;

template<>
struct GLType<int8_t>
{
    static constexpr GLenum value = GL_BYTE;
};

template<>
struct GLType<uint8_t>
{
    static constexpr GLenum value = GL_UNSIGNED_BYTE;
};

template<>
struct GLType<int16_t>
{
    static constexpr GLenum value = GL_SHORT;
};

template<>
struct GLType<uint16_t>
{
    static constexpr GLenum value = GL_UNSIGNED_SHORT;
};

template<>
struct GLType<int32_t>
{
    static constexpr GLenum value = GL_INT;
};

template<>
struct GLType<uint32_t>
{
    static constexpr GLenum value = GL_UNSIGNED_INT;
};

template<>
struct GLType<float>
{
    static_assert(sizeof(float) == 4);
    static constexpr GLenum value = GL_FLOAT;
};

template<glm::length_t L, typename T, glm::qualifier Q>
struct GLType<glm::vec<L, T, Q>>
{
    static constexpr auto value = GLType<T>::value;
};

} // namespace detail

template<typename VertexT>
Mesh<VertexT>::Mesh()
    : m_buffer(Buffer::Type::Vertex, Buffer::Usage::StaticDraw)
{
    m_buffer.bind();
    VertexArray::Binder binder(&m_vao);
    reflect::forEachMember(VertexT{},
                           [index = 0, offset = static_cast<const std::byte *>(nullptr)](const auto &m) mutable {
                               using Type = std::decay_t<decltype(m)>;
                               glEnableVertexAttribArray(index);
                               const auto size = detail::GLSize<Type>::value;
                               const auto type = detail::GLType<Type>::value;
                               glVertexAttribPointer(index, size, type, GL_FALSE, sizeof(VertexT), offset);
                               ++index;
                               offset += sizeof(m);
                           });
}

template<typename VertexT>
Mesh<VertexT>::Mesh(Mesh<VertexT> &&other)
    : m_vertexCount{other.m_vertexCount}
    , m_buffer{std::move(other.m_buffer)}
    , m_vao{std::move(other.m_vao)}
{
    other.m_vertexCount = 0;
}

template<typename VertexT>
Mesh<VertexT> &Mesh<VertexT>::operator=(Mesh<VertexT> &&other)
{
    m_vertexCount = other.m_vertexCount;
    m_buffer = std::move(other.m_buffer);
    m_vao = std::move(other.m_vao);

    other.m_vertexCount = 0;

    return *this;
}

template<typename VertexT>
void Mesh<VertexT>::setData(std::span<const VertexT> data)
{
    m_vertexCount = data.size();
    m_buffer.bind();
    m_buffer.allocate(std::as_bytes(data));
}

template<typename VertexT>
void Mesh<VertexT>::render(Primitive primitive) const
{
    VertexArray::Binder binder(&m_vao);
    glDrawArrays(static_cast<GLenum>(primitive), 0, m_vertexCount);
}

template<typename IndexT, typename VertexT>
IndexedMesh<IndexT, VertexT>::IndexedMesh()
    : m_vertexBuffer(Buffer::Type::Vertex, Buffer::Usage::StaticDraw)
    , m_indexBuffer(Buffer::Type::Index, Buffer::Usage::StaticDraw)
{
    VertexArray::Binder binder(&m_vao);
    m_vertexBuffer.bind();
    m_indexBuffer.bind(); // needs to be called while the VAO is bound so that it's stored in the VAO
    reflect::forEachMember(VertexT{},
                           [index = 0, offset = static_cast<const std::byte *>(nullptr)](const auto &m) mutable {
                               using Type = std::decay_t<decltype(m)>;
                               glEnableVertexAttribArray(index);
                               const auto size = detail::GLSize<Type>::value;
                               const auto type = detail::GLType<Type>::value;
                               glVertexAttribPointer(index, size, type, GL_FALSE, sizeof(VertexT), offset);
                               ++index;
                               offset += sizeof(m);
                           });
}

template<typename IndexT, typename VertexT>
IndexedMesh<IndexT, VertexT>::IndexedMesh(IndexedMesh<IndexT, VertexT> &&other)
    : m_vertexCount{other.m_vertexCount}
    , m_vertexBuffer{std::move(other.m_vertexBuffer)}
    , m_indexCount{other.m_indexCount}
    , m_indexBuffer{std::move(other.m_indexBuffer)}
    , m_vao{std::move(other.m_vao)}
{
    other.m_vertexCount = 0;
    other.m_indexCount = 0;
}

template<typename IndexT, typename VertexT>
IndexedMesh<IndexT, VertexT> &IndexedMesh<IndexT, VertexT>::operator=(IndexedMesh<IndexT, VertexT> &&other)
{
    m_vertexCount = other.m_vertexCount;
    m_vertexBuffer = std::move(other.m_vertexBuffer);
    m_indexCount = other.m_indexCount;
    m_indexBuffer = std::move(other.m_indexBuffer);
    m_vao = std::move(other.m_vao);

    other.m_vertexCount = 0;
    other.m_indexCount = 0;

    return *this;
}

template<typename IndexT, typename VertexT>
void IndexedMesh<IndexT, VertexT>::setVertexData(std::span<const VertexT> data)
{
    m_vertexCount = data.size();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(std::as_bytes(data));
}

template<typename IndexT, typename VertexT>
void IndexedMesh<IndexT, VertexT>::setIndexData(std::span<const IndexT> data)
{
    m_indexCount = data.size();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(std::as_bytes(data));
}

template<typename IndexT, typename VertexT>
void IndexedMesh<IndexT, VertexT>::render(Primitive primitive) const
{
    VertexArray::Binder binder(&m_vao);
    glDrawElements(static_cast<GLenum>(primitive), m_indexCount, detail::GLType<IndexT>::value, nullptr);
}

} // namespace muui::gl
