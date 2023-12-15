#include "spritebatcher.h"
#include "abstracttexture.h"
#include "system.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace muui
{

SpriteBatcher::SpriteBatcher()
    : m_buffer(gl::Buffer::Type::Vertex, gl::Buffer::Usage::DynamicDraw)
{
    glGenVertexArrays(1, &m_vao);

    m_buffer.bind();
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), reinterpret_cast<GLvoid *>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                          reinterpret_cast<GLvoid *>(2 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                          reinterpret_cast<GLvoid *>(4 * sizeof(GLfloat)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex),
                          reinterpret_cast<GLvoid *>(8 * sizeof(GLfloat)));
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

void SpriteBatcher::setBatchTexture(const AbstractTexture *texture)
{
    m_batchTexture = texture;
}

void SpriteBatcher::setBatchGradientTexture(const AbstractTexture *texture)
{
    m_batchGradientTexture = texture;
}

void SpriteBatcher::setBatchScissorBox(const ScissorBox &scissorBox)
{
    m_batchScissorBox = scissorBox;
}

void SpriteBatcher::begin()
{
    m_quadCount = 0;
    m_batchProgram = ShaderManager::InvalidProgram;
    m_batchTexture = nullptr;
    m_batchGradientTexture = nullptr;
    m_batchScissorBox = ScissorBox{};
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
        return std::tie(a->depth, a->texture, a->gradientTexture, a->program) <
               std::tie(b->depth, b->texture, b->gradientTexture, b->program);
    });

    m_buffer.bind();
    glBindVertexArray(m_vao);

    const AbstractTexture *currentTexture = nullptr;
    const AbstractTexture *currentGradientTexture = nullptr;
    ShaderManager::ProgramHandle currentProgram = ShaderManager::InvalidProgram;
    ScissorBox currentScissorBox;

    auto batchStart = sortedQuads.begin();
    while (batchStart != sortedQuadsEnd)
    {
        const auto *batchTexture = (*batchStart)->texture;
        const auto *batchGradientTexture = (*batchStart)->gradientTexture;
        const auto batchProgram = (*batchStart)->program;
        const auto scissorBox = (*batchStart)->scissorBox;
        const auto batchEnd =
            std::find_if(batchStart + 1, sortedQuadsEnd,
                         [batchTexture, batchGradientTexture, batchProgram, &scissorBox](const Sprite *sprite) {
                             return sprite->texture != batchTexture ||
                                    sprite->gradientTexture != batchGradientTexture ||
                                    sprite->program != batchProgram || sprite->scissorBox != scissorBox;
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

            const auto emitVertex = [&data](const SpriteVertex &vertex) {
                *data++ = vertex.position.x;
                *data++ = vertex.position.y;

                *data++ = vertex.texCoord.x;
                *data++ = vertex.texCoord.y;

                *data++ = vertex.fgColor.x;
                *data++ = vertex.fgColor.y;
                *data++ = vertex.fgColor.z;
                *data++ = vertex.fgColor.w;

                *data++ = vertex.bgColor.x;
                *data++ = vertex.bgColor.y;
                *data++ = vertex.bgColor.z;
                *data++ = vertex.bgColor.w;
            };

            const auto &v0 = quadPtr->vertices[0];
            const auto &v1 = quadPtr->vertices[1];
            const auto &v2 = quadPtr->vertices[2];
            const auto &v3 = quadPtr->vertices[3];

            emitVertex(v0);
            emitVertex(v1);
            emitVertex(v2);

            emitVertex(v2);
            emitVertex(v3);
            emitVertex(v0);
        }
        m_buffer.unmap();

        if (currentTexture != batchTexture)
        {
            currentTexture = batchTexture;
            if (currentTexture)
                currentTexture->bind(0);
        }

        if (currentGradientTexture != batchGradientTexture)
        {
            currentGradientTexture = batchGradientTexture;
            if (currentGradientTexture)
                currentGradientTexture->bind(1);
        }

        if (currentProgram != batchProgram)
        {
            currentProgram = batchProgram;
            auto *shaderManager = getShaderManager();
            shaderManager->useProgram(batchProgram);
            shaderManager->setUniform("mvp", m_transformMatrix);
            if (currentTexture)
                shaderManager->setUniform("baseColorTexture", 0);
            if (currentGradientTexture)
                shaderManager->setUniform("gradientTexture", 1);
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
