#pragma once

#include "buffer.h"
#include "noncopyable.h"
#include "shadermanager.h"
#include "util.h"

#include <glm/vec2.hpp>

#include <array>

namespace muui
{
class AbstractTexture;
struct PackedPixmap;

class SpriteBatcher : private NonCopyable
{
public:
    SpriteBatcher();
    ~SpriteBatcher();

    void setTransformMatrix(const glm::mat4 &matrix);
    glm::mat4 transformMatrix() const { return m_transformMatrix; }

    void setBatchProgram(ShaderManager::Program program);
    ShaderManager::Program batchProgram() const { return m_batchProgram; }

    void setClipRect(const RectF &rect);

    void begin();
    void flush();

    using Quad = std::array<glm::vec2, 4>;

    void addSprite(const Quad &quad, const glm::vec4 &color, int depth);
    void addSprite(const PackedPixmap &pixmap, const Quad &quad, const glm::vec4 &color, int depth);
    void addSprite(const AbstractTexture *texture, const Quad &quad, const RectF &texRect, const glm::vec4 &color,
                   int depth);

    void addSprite(const Quad &quad, const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth);
    void addSprite(const PackedPixmap &pixmap, const Quad &quad, const glm::vec4 &fgColor, const glm::vec4 &bgColor,
                   int depth);
    void addSprite(const AbstractTexture *texture, const Quad &quad, const RectF &texRect, const glm::vec4 &fgColor,
                   const glm::vec4 &bgColor, int depth);

    void addSprite(const RectF &rect, const glm::vec4 &color, int depth);
    void addSprite(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &color, int depth);
    void addSprite(const AbstractTexture *texture, const RectF &rect, const RectF &texRect, const glm::vec4 &color,
                   int depth);

    void addSprite(const RectF &rect, const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth);
    void addSprite(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &fgColor, const glm::vec4 &bgColor,
                   int depth);
    void addSprite(const AbstractTexture *texture, const RectF &rect, const RectF &texRect, const glm::vec4 &fgColor,
                   const glm::vec4 &bgColor, int depth);

private:
    struct Sprite
    {
        ShaderManager::Program program;
        const AbstractTexture *texture;
        Quad quad;
        RectF texRect;
        glm::vec4 fgColor;
        glm::vec4 bgColor;
        int depth;
    };

    struct Vertex
    {
        glm::vec2 position;
        glm::vec2 texCoord;
        glm::vec4 fgColor;
        glm::vec4 bgColor;
    };

    static constexpr int BufferCapacity = 0x100000;                       // in floats
    static constexpr int GLVertexSize = sizeof(Vertex) / sizeof(GLfloat); // in floats
    static constexpr int GLQuadSize = 6 * GLVertexSize;                   // 6 verts per quad
    static constexpr int MaxQuadsPerBatch = BufferCapacity / GLQuadSize;

    std::array<Sprite, MaxQuadsPerBatch> m_sprites;
    int m_quadCount = 0;
    gl::Buffer m_buffer;
    glm::mat4 m_transformMatrix;
    ShaderManager::Program m_batchProgram = ShaderManager::Program::Flat;
    bool m_bufferAllocated = false;
    int m_bufferOffset = 0;
    GLuint m_vao = 0;
};

} // namespace gl
