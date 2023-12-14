#pragma once

#include "buffer.h"
#include "noncopyable.h"
#include "shadermanager.h"

#include <glm/vec2.hpp>

#include <array>

namespace muui
{
class AbstractTexture;
struct PackedPixmap;

// clang-format off
template<typename VertexT>
concept HasPosition = requires(VertexT v) {
                          { v.position.x } -> std::convertible_to<float>;
                          { v.position.y } -> std::convertible_to<float>;
                      };

template<typename VertexT>
concept HasTexCoord = requires(VertexT v) {
                          { v.texCoord.x } -> std::convertible_to<float>;
                          { v.texCoord.y } -> std::convertible_to<float>;
                      };

template<typename VertexT>
concept HasColor = requires(VertexT v) {
                       { v.color.r } -> std::convertible_to<float>;
                       { v.color.g } -> std::convertible_to<float>;
                       { v.color.b } -> std::convertible_to<float>;
                       { v.color.a } -> std::convertible_to<float>;
                   };
// clang-format on

class SpriteBatcher : private NonCopyable
{
public:
    SpriteBatcher();
    ~SpriteBatcher();

    void setTransformMatrix(const glm::mat4 &matrix);
    glm::mat4 transformMatrix() const { return m_transformMatrix; }

    void setBatchProgram(ShaderManager::ProgramHandle program);
    ShaderManager::ProgramHandle batchProgram() const { return m_batchProgram; }

    void setBatchTexture(const AbstractTexture *texture);
    const AbstractTexture *batchTexture() const { return m_batchTexture; }

    struct ScissorBox
    {
        glm::ivec2 position;
        glm::ivec2 size;
        bool operator==(const ScissorBox &) const = default;
    };
    void setBatchScissorBox(const ScissorBox &scissorBox);
    ScissorBox batchScissorBox() const { return m_batchScissorBox; }

    void begin();
    void flush();

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const std::array<VertexT, 4> &verts, int depth)
    {
        if (m_quadCount == MaxQuadsPerBatch)
            flush();

        auto &sprite = m_sprites[m_quadCount++];
        sprite.texture = m_batchTexture;
        sprite.program = m_batchProgram;
        sprite.depth = depth;
        sprite.scissorBox = m_batchScissorBox;
        std::copy(verts.begin(), verts.end(), sprite.vertices.begin());
    }

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, int depth)
    {
        if (m_quadCount == MaxQuadsPerBatch)
            flush();

        auto &sprite = m_sprites[m_quadCount++];
        sprite.texture = m_batchTexture;
        sprite.program = m_batchProgram;
        sprite.depth = depth;
        sprite.scissorBox = m_batchScissorBox;

        initializeVertices(sprite.vertices, topLeft, bottomRight);
    }

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, const glm::vec4 &color, int depth)
    {
        if (m_quadCount == MaxQuadsPerBatch)
            flush();

        auto &sprite = m_sprites[m_quadCount++];
        sprite.texture = m_batchTexture;
        sprite.program = m_batchProgram;
        sprite.depth = depth;
        sprite.scissorBox = m_batchScissorBox;

        initializeVertices(sprite.vertices, topLeft, bottomRight);

        sprite.vertices[0].fgColor = color;
        sprite.vertices[1].fgColor = color;
        sprite.vertices[2].fgColor = color;
        sprite.vertices[3].fgColor = color;
    }

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, const glm::vec4 &fgColor,
                   const glm::vec4 &bgColor, int depth)
    {
        if (m_quadCount == MaxQuadsPerBatch)
            flush();

        auto &sprite = m_sprites[m_quadCount++];
        sprite.texture = m_batchTexture;
        sprite.program = m_batchProgram;
        sprite.depth = depth;
        sprite.scissorBox = m_batchScissorBox;

        initializeVertices(sprite.vertices, topLeft, bottomRight);

        sprite.vertices[0].fgColor = fgColor;
        sprite.vertices[1].fgColor = fgColor;
        sprite.vertices[2].fgColor = fgColor;
        sprite.vertices[3].fgColor = fgColor;

        sprite.vertices[0].bgColor = bgColor;
        sprite.vertices[1].bgColor = bgColor;
        sprite.vertices[2].bgColor = bgColor;
        sprite.vertices[3].bgColor = bgColor;
    }

private:
    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        glm::vec4 fgColor;
        glm::vec4 bgColor;

        Vertex() = default;

        template<typename InVertexT>
            requires HasPosition<InVertexT>
        Vertex(const InVertexT &inVertex)
            : position(inVertex.position)
        {
        }

        template<typename InVertexT>
            requires HasPosition<InVertexT> && HasTexCoord<InVertexT>
        Vertex(const InVertexT &inVertex)
            : position(inVertex.position)
            , texCoord(inVertex.texCoord)
        {
        }

        template<typename InVertexT>
            requires HasPosition<InVertexT> && HasColor<InVertexT>
        Vertex(const InVertexT &inVertex)
            : position(inVertex.position)
            , fgColor(inVertex.color)
        {
        }

        template<typename InVertexT>
            requires HasPosition<InVertexT> && HasTexCoord<InVertexT> && HasColor<InVertexT>
        Vertex(const InVertexT &inVertex)
            : position(inVertex.position)
            , texCoord(inVertex.texCoord)
            , fgColor(inVertex.color)
        {
        }
    };

    struct Sprite
    {
        ShaderManager::ProgramHandle program;
        const AbstractTexture *texture;
        std::array<Vertex, 4> vertices;
        int depth;
        ScissorBox scissorBox;
    };

    template<typename VertexT>
        requires HasPosition<VertexT>
    void initializeVertices(std::array<Vertex, 4> &vertices, const VertexT &topLeft, const VertexT &bottomRight)
    {
        auto &v0 = vertices[0];
        auto &v1 = vertices[1];
        auto &v2 = vertices[2];
        auto &v3 = vertices[3];

        const auto &p0 = topLeft.position;
        const auto &p1 = bottomRight.position;
        v0.position = {p0.x, p0.y};
        v1.position = {p1.x, p0.y};
        v2.position = {p1.x, p1.y};
        v3.position = {p0.x, p1.y};

        if constexpr (HasTexCoord<VertexT>)
        {
            const auto &t0 = topLeft.texCoord;
            const auto &t1 = bottomRight.texCoord;
            v0.texCoord = {t0.x, t0.y};
            v1.texCoord = {t1.x, t0.y};
            v2.texCoord = {t1.x, t1.y};
            v3.texCoord = {t0.x, t1.y};
        }
    }

    static constexpr int BufferCapacity = 0x100000;                       // in floats
    static constexpr int GLVertexSize = sizeof(Vertex) / sizeof(GLfloat); // in floats
    static constexpr int GLQuadSize = 6 * GLVertexSize;                   // 6 verts per quad
    static constexpr int MaxQuadsPerBatch = BufferCapacity / GLQuadSize;

    std::array<Sprite, MaxQuadsPerBatch> m_sprites;
    int m_quadCount{0};
    gl::Buffer m_buffer;
    glm::mat4 m_transformMatrix;
    ShaderManager::ProgramHandle m_batchProgram{ShaderManager::InvalidProgram};
    const AbstractTexture *m_batchTexture{nullptr};
    ScissorBox m_batchScissorBox;
    bool m_bufferAllocated{false};
    int m_bufferOffset{0};
    GLuint m_vao{0};
};

} // namespace muui
