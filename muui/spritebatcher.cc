#include "spritebatcher.h"
#include "abstracttexture.h"
#include "system.h"
#include "textureatlas.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace muui
{

namespace
{
constexpr SpriteBatcher::Quad asQuad(const RectF &rect)
{
    const auto &min = rect.min;
    const auto &max = rect.max;
    return {{{min.x, min.y}, {max.x, min.y}, {max.x, max.y}, {min.x, max.y}}};
}
} // namespace

SpriteBatcher::SpriteBatcher()
    : m_buffer(gl::Buffer::Type::Vertex, gl::Buffer::Usage::DynamicDraw)
{
    glGenVertexArrays(1, &m_vao);

    m_buffer.bind();
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(2 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(4 * sizeof(GLfloat)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid *>(8 * sizeof(GLfloat)));
}

SpriteBatcher::~SpriteBatcher()
{
    glDeleteVertexArrays(1, &m_vao);
}

void SpriteBatcher::setTransformMatrix(const glm::mat4 &matrix)
{
    m_transformMatrix = matrix;
}

void SpriteBatcher::setBatchProgram(ShaderManager::ProgramHandle program)
{
    m_batchProgram = program;
}

void SpriteBatcher::setScissorBox(const ScissorBox &scissorBox)
{
    m_scissorBox = scissorBox;
}

void SpriteBatcher::begin()
{
    m_quadCount = 0;
    m_batchProgram = ShaderManager::InvalidProgram;
    m_scissorBox = ScissorBox{};
}

void SpriteBatcher::addSprite(const RectF &rect, const glm::vec4 &color, int depth)
{
    addSprite(nullptr, asQuad(rect), {}, color, {}, depth);
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &color, int depth)
{
    addSprite(pixmap.texture, asQuad(rect), pixmap.texCoord, color, {}, depth);
}

void SpriteBatcher::addSprite(const AbstractTexture *texture, const RectF &rect, const RectF &texRect,
                              const glm::vec4 &color, int depth)
{
    addSprite(texture, asQuad(rect), texRect, color, {}, depth);
}

void SpriteBatcher::addSprite(const Quad &quad, const glm::vec4 &color, int depth)
{
    addSprite(nullptr, quad, {}, color, {}, depth);
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const Quad &quad, const glm::vec4 &color, int depth)
{
    addSprite(pixmap.texture, quad, pixmap.texCoord, color, {}, depth);
}

void SpriteBatcher::addSprite(const AbstractTexture *texture, const Quad &quad, const RectF &texRect,
                              const glm::vec4 &color, int depth)
{
    addSprite(texture, quad, texRect, color, {}, depth);
}

void SpriteBatcher::addSprite(const RectF &rect, const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth)
{
    addSprite(nullptr, asQuad(rect), {}, fgColor, bgColor, depth);
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const RectF &rect, const glm::vec4 &fgColor,
                              const glm::vec4 &bgColor, int depth)
{
    addSprite(pixmap.texture, asQuad(rect), pixmap.texCoord, fgColor, bgColor, depth);
}

void SpriteBatcher::addSprite(const AbstractTexture *texture, const RectF &rect, const RectF &texRect,
                              const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth)
{
    addSprite(texture, asQuad(rect), texRect, fgColor, bgColor, depth);
}

void SpriteBatcher::addSprite(const Quad &quad, const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth)
{
    addSprite(nullptr, quad, {}, fgColor, bgColor, depth);
}

void SpriteBatcher::addSprite(const PackedPixmap &pixmap, const Quad &quad, const glm::vec4 &fgColor,
                              const glm::vec4 &bgColor, int depth)
{
    addSprite(pixmap.texture, quad, pixmap.texCoord, fgColor, bgColor, depth);
}

void SpriteBatcher::addSprite(const AbstractTexture *texture, const Quad &quad, const RectF &texRect,
                              const glm::vec4 &fgColor, const glm::vec4 &bgColor, int depth)
{
    if (m_quadCount == MaxQuadsPerBatch)
        flush();

    assert(m_batchProgram != ShaderManager::InvalidProgram);
    auto &sprite = m_sprites[m_quadCount++];
    sprite.texture = texture;
    sprite.program = m_batchProgram;
    sprite.quad = quad;
    sprite.texRect = texRect;
    sprite.fgColor = fgColor;
    sprite.bgColor = bgColor;
    sprite.depth = depth;
    sprite.scissorBox = m_scissorBox;
}

void SpriteBatcher::flush()
{
    if (m_quadCount == 0)
        return;

    static std::array<const Sprite *, MaxQuadsPerBatch> sortedQuads;
    const auto quadsEnd = m_sprites.begin() + m_quadCount;
    std::transform(m_sprites.begin(), quadsEnd, sortedQuads.begin(), [](const Sprite &sprite) { return &sprite; });
    const auto sortedQuadsEnd = sortedQuads.begin() + m_quadCount;
    std::stable_sort(sortedQuads.begin(), sortedQuadsEnd, [](const Sprite *a, const Sprite *b) {
        // TODO: scissorBox?
        return std::tie(a->depth, a->texture, a->program) < std::tie(b->depth, b->texture, b->program);
    });

    m_buffer.bind();
    glBindVertexArray(m_vao);

    const AbstractTexture *currentTexture = nullptr;
    ShaderManager::ProgramHandle currentProgram = ShaderManager::InvalidProgram;
    ScissorBox currentScissorBox;
    int positionLocation = -1, texCoordLocation = -1, colorLocation = -1;

    auto batchStart = sortedQuads.begin();
    while (batchStart != sortedQuadsEnd)
    {
        const auto *batchTexture = (*batchStart)->texture;
        const auto batchProgram = (*batchStart)->program;
        const auto scissorBox = (*batchStart)->scissorBox;
        const auto batchEnd = std::find_if(
            batchStart + 1, sortedQuadsEnd, [batchTexture, batchProgram, &scissorBox](const Sprite *sprite) {
                return sprite->texture != batchTexture || sprite->program != batchProgram ||
                       sprite->scissorBox != scissorBox;
            });

        const auto quadCount = batchEnd - batchStart;
        const auto bufferRangeSize = quadCount * GLQuadSize;

        if (!m_bufferAllocated || (m_bufferOffset + bufferRangeSize > BufferCapacity))
        {
            // orphan the old buffer and grab a new memory block
            m_buffer.allocate(BufferCapacity * sizeof(GLfloat));
            m_bufferOffset = 0;
            m_bufferAllocated = true;
        }

        auto *data =
            m_buffer.mapRange<GLfloat>(m_bufferOffset, bufferRangeSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        for (auto it = batchStart; it != batchEnd; ++it)
        {
            auto *quadPtr = *it;

            const auto emitVertex = [&data, fgColor = quadPtr->fgColor,
                                     bgColor = quadPtr->bgColor](const glm::vec2 &position, const glm::vec2 &texCoord) {
                *data++ = position.x;
                *data++ = position.y;

                *data++ = texCoord.x;
                *data++ = texCoord.y;

                *data++ = fgColor.x;
                *data++ = fgColor.y;
                *data++ = fgColor.z;
                *data++ = fgColor.w;

                *data++ = bgColor.x;
                *data++ = bgColor.y;
                *data++ = bgColor.z;
                *data++ = bgColor.w;
            };

            const auto &p0 = quadPtr->quad[0];
            const auto &p1 = quadPtr->quad[1];
            const auto &p2 = quadPtr->quad[2];
            const auto &p3 = quadPtr->quad[3];

            const auto &t0 = quadPtr->texRect.min;
            const auto &t1 = quadPtr->texRect.max;

            emitVertex(p0, {t0.x, t0.y});
            emitVertex(p1, {t1.x, t0.y});
            emitVertex(p2, {t1.x, t1.y});

            emitVertex(p2, {t1.x, t1.y});
            emitVertex(p3, {t0.x, t1.y});
            emitVertex(p0, {t0.x, t0.y});
        }
        m_buffer.unmap();

        if (currentTexture != batchTexture)
        {
            currentTexture = batchTexture;
            if (currentTexture)
                currentTexture->bind();
        }

        if (currentProgram != batchProgram)
        {
            currentProgram = batchProgram;
            auto *shaderManager = getShaderManager();
            shaderManager->useProgram(batchProgram);
            shaderManager->setUniform("mvp", m_transformMatrix);
            if (currentTexture)
                shaderManager->setUniform("baseColorTexture", 0);
        }

        if (currentScissorBox != scissorBox)
        {
            currentScissorBox = scissorBox;
            glScissor(scissorBox.position.x, scissorBox.position.y, scissorBox.size.x, scissorBox.size.y);
        }

        glDrawArrays(GL_TRIANGLES, m_bufferOffset / GLVertexSize, quadCount * 6);

        m_bufferOffset += bufferRangeSize;
        batchStart = batchEnd;
    }

    m_quadCount = 0;
}

} // namespace muui
