#pragma once

#include "buffer.h"
#include "noncopyable.h"
#include "shadermanager.h"
#include "transform.h"
#include "vertexarray.h"

#include <glm/vec2.hpp>

#include <algorithm>
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

struct BlendFunc
{
    enum class Factor
    {
        Zero = GL_ZERO,
        One = GL_ONE,
        SourceAlpha = GL_SRC_ALPHA,
        OneMinusSourceAlpha = GL_ONE_MINUS_SRC_ALPHA,
        DestAlpha = GL_DST_ALPHA,
        OneMinusDestAlpha = GL_ONE_MINUS_DST_ALPHA
    };
    Factor sourceFactor;
    Factor destFactor;
    bool operator==(const BlendFunc &) const = default;
};

class SpriteBatcher : private NonCopyable
{
public:
    SpriteBatcher();
    ~SpriteBatcher();

    void setMvp(const glm::mat4 &mvp);
    glm::mat4 mvp() const { return m_mvp; }

    void setTransform(const Transform &transform);
    const Transform &transform() const { return m_transform; }

    void translate(const glm::vec2 &p);
    void rotate(float angle);

    void setBatchProgram(ShaderManager::ProgramHandle program);
    ShaderManager::ProgramHandle batchProgram() const { return m_batchProgram; }

    void setBatchTexture(const AbstractTexture *texture);
    const AbstractTexture *batchTexture() const { return m_batchTexture; }

    void setBatchGradientTexture(const AbstractTexture *texture);
    const AbstractTexture *batchGradientTexture() const { return m_batchGradientTexture; }

    void setBatchBlendFunc(BlendFunc blendFunc);
    BlendFunc batchBlendFunc() const { return m_batchBlendFunc; }

    void begin();
    void flush();

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const std::array<VertexT, 4> &verts, int depth)
    {
        std::array<SpriteVertex, 4> spriteVerts;
        std::transform(verts.begin(), verts.end(), spriteVerts.begin(), [this](const VertexT &v) {
            const auto position = m_transform.map(v.position);
            if constexpr (HasPosition<VertexT> && HasTexCoord<VertexT> && HasColor<VertexT>)
            {
                return SpriteVertex(position, v.texCoord, v.color, {});
            }
            else if constexpr (HasPosition<VertexT> && HasColor<VertexT>)
            {
                return SpriteVertex(position, {}, v.color, {});
            }
            else if constexpr (HasPosition<VertexT> && HasTexCoord<VertexT>)
            {
                return SpriteVertex(position, v.texCoord, {}, {});
            }
            else
            {
                return SpriteVertex(position, {}, {}, {});
            }
        });
        addSprite(spriteVerts, depth);
    }

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, int depth)
    {
        addSprite(unpack(topLeft, bottomRight), depth);
    }

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, const glm::vec4 &color, int depth)
    {
        addSprite(unpack(topLeft, bottomRight, color), depth);
    }

    template<typename VertexT>
        requires HasPosition<VertexT>
    void addSprite(const VertexT &topLeft, const VertexT &bottomRight, const glm::vec4 &fgColor,
                   const glm::vec4 &bgColor, int depth)
    {
        addSprite(unpack(topLeft, bottomRight, fgColor, bgColor), depth);
    }

private:
    struct SpriteVertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        glm::vec4 fgColor;
        glm::vec4 bgColor;

        SpriteVertex() = default;

        SpriteVertex(const glm::vec2 &position, const glm::vec2 &texCoord, const glm::vec4 &fgColor,
                     const glm::vec4 &bgColor)
            : position(position)
            , texCoord(texCoord)
            , fgColor(fgColor)
            , bgColor(bgColor)
        {
        }

        template<typename InVertexT>
        requires HasPosition<InVertexT> SpriteVertex(const InVertexT &inVertex)
            : position(inVertex.position)
        {
        }

        template<typename InVertexT>
        requires HasPosition<InVertexT> && HasTexCoord<InVertexT> SpriteVertex(const InVertexT &inVertex)
            : position(inVertex.position)
            , texCoord(inVertex.texCoord)
        {
        }

        template<typename InVertexT>
        requires HasPosition<InVertexT> && HasColor<InVertexT> SpriteVertex(const InVertexT &inVertex)
            : position(inVertex.position)
            , fgColor(inVertex.color)
        {
        }

        template<typename InVertexT>
        requires HasPosition<InVertexT> && HasTexCoord<InVertexT> && HasColor<InVertexT>
        SpriteVertex(const InVertexT &inVertex)
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
        const AbstractTexture *gradientTexture;
        std::array<SpriteVertex, 4> vertices;
        int depth;
        BlendFunc blendFunc;
    };

    template<typename VertexT>
        requires HasPosition<VertexT>
    std::array<SpriteVertex, 4> unpack(const VertexT &topLeft, const VertexT &bottomRight,
                                       const glm::vec4 &fgColor = {}, const glm::vec4 &bgColor = {}) const
    {
        const auto &p0 = topLeft.position;
        const auto &p1 = bottomRight.position;
        return {SpriteVertex{m_transform.map({p0.x, p0.y}), {}, fgColor, bgColor},
                SpriteVertex{m_transform.map({p1.x, p0.y}), {}, fgColor, bgColor},
                SpriteVertex{m_transform.map({p1.x, p1.y}), {}, fgColor, bgColor},
                SpriteVertex{m_transform.map({p0.x, p1.y}), {}, fgColor, bgColor}};
    }

    template<typename VertexT>
        requires HasPosition<VertexT> && HasTexCoord<VertexT>
    std::array<SpriteVertex, 4> unpack(const VertexT &topLeft, const VertexT &bottomRight,
                                       const glm::vec4 &fgColor = {}, const glm::vec4 &bgColor = {}) const
    {
        const auto &p0 = topLeft.position;
        const auto &p1 = bottomRight.position;
        const auto &t0 = topLeft.texCoord;
        const auto &t1 = bottomRight.texCoord;
        return {SpriteVertex{m_transform.map({p0.x, p0.y}), {t0.x, t0.y}, fgColor, bgColor},
                SpriteVertex{m_transform.map({p1.x, p0.y}), {t1.x, t0.y}, fgColor, bgColor},
                SpriteVertex{m_transform.map({p1.x, p1.y}), {t1.x, t1.y}, fgColor, bgColor},
                SpriteVertex{m_transform.map({p0.x, p1.y}), {t0.x, t1.y}, fgColor, bgColor}};
    }

    void addSprite(const std::array<SpriteVertex, 4> &verts, int depth)
    {
        if (m_quadCount == MaxQuadsPerBatch)
            flush();

        auto &sprite = m_sprites[m_quadCount++];
        sprite.texture = m_batchTexture;
        sprite.gradientTexture = m_batchGradientTexture;
        sprite.program = m_batchProgram;
        sprite.depth = depth;
        sprite.blendFunc = m_batchBlendFunc;
        sprite.vertices = verts;
    }

    static constexpr int MaxQuadsPerBatch = 512 * 1024;
    static constexpr int GLVertexSize = sizeof(SpriteVertex) / sizeof(GLfloat); // in floats

    std::array<Sprite, MaxQuadsPerBatch> m_sprites;
    int m_quadCount{0};
    gl::Buffer m_vertexBuffer;
    gl::Buffer m_indexBuffer;
    gl::VertexArray m_vao;
    glm::mat4 m_mvp;
    Transform m_transform;
    ShaderManager::ProgramHandle m_batchProgram{ShaderManager::ProgramHandle::Invalid};
    const AbstractTexture *m_batchTexture{nullptr};
    const AbstractTexture *m_batchGradientTexture{nullptr};
    BlendFunc m_batchBlendFunc{BlendFunc::Factor::SourceAlpha, BlendFunc::Factor::OneMinusSourceAlpha};
    bool m_bufferAllocated{false};
    int m_quadIndex{0};
};

} // namespace muui
