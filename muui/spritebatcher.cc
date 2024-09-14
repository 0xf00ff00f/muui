#include "spritebatcher.h"
#include "abstracttexture.h"
#include "system.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <optional>
#include <span>

namespace muui
{

SpriteBatcher::SpriteBatcher()
    : m_vertexBuffer(gl::Buffer::Type::Vertex, gl::Buffer::Usage::DynamicDraw)
    , m_indexBuffer(gl::Buffer::Type::Index, gl::Buffer::Usage::StaticDraw)
{
    std::vector<uint32_t> indices(MaxQuadsPerBatch * 6);
    for (std::size_t i = 0; i < MaxQuadsPerBatch; ++i)
    {
        indices[i * 6 + 0] = i * 4 + 0;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;

        indices[i * 6 + 3] = i * 4 + 2;
        indices[i * 6 + 4] = i * 4 + 3;
        indices[i * 6 + 5] = i * 4 + 0;
    }
    m_indexBuffer.bind();
    m_indexBuffer.allocate(std::as_bytes(std::span<uint32_t>(indices)));

    gl::VertexArray::Binder binder(&m_vao);
    m_vertexBuffer.bind();
    m_indexBuffer.bind();

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

SpriteBatcher::~SpriteBatcher() = default;

void SpriteBatcher::setMvp(const glm::mat4 &mvp)
{
    m_mvp = mvp;
}

void SpriteBatcher::setTransform(const Transform &transform)
{
    m_transform = transform;
}

void SpriteBatcher::translate(const glm::vec2 &p)
{
    m_transform.translate(p);
}

void SpriteBatcher::rotate(float angle)
{
    m_transform.rotate(angle);
}

void SpriteBatcher::scale(const glm::vec2 &v)
{
    m_transform.scale(v);
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

void SpriteBatcher::setBatchBlendFunc(BlendFunc blendFunc)
{
    m_batchBlendFunc = blendFunc;
}

void SpriteBatcher::begin()
{
    m_quadCount = 0;
    m_transform.reset();
    m_batchProgram = ShaderManager::ProgramHandle::Invalid;
    m_batchTexture = nullptr;
    m_batchGradientTexture = nullptr;
    // assume shaders output premultiplied alpha by default
    m_batchBlendFunc = {BlendFunc::Factor::One, BlendFunc::Factor::OneMinusSourceAlpha};
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
        return std::tie(a->depth, a->texture, a->gradientTexture, a->program) <
               std::tie(b->depth, b->texture, b->gradientTexture, b->program);
    });

    m_vertexBuffer.bind();
    gl::VertexArray::Binder binder(&m_vao);

    const AbstractTexture *currentTexture = nullptr;
    const AbstractTexture *currentGradientTexture = nullptr;
    ShaderManager::ProgramHandle currentProgram = ShaderManager::ProgramHandle::Invalid;
    std::optional<BlendFunc> currentBlendMode;

    auto batchStart = sortedQuads.begin();
    while (batchStart != sortedQuadsEnd)
    {
        const auto *batchTexture = (*batchStart)->texture;
        const auto *batchGradientTexture = (*batchStart)->gradientTexture;
        const auto batchProgram = (*batchStart)->program;
        const auto blendFunc = (*batchStart)->blendFunc;
        const auto batchEnd =
            std::find_if(batchStart + 1, sortedQuadsEnd,
                         [batchTexture, batchGradientTexture, batchProgram, blendFunc](const Sprite *sprite) {
                             return sprite->texture != batchTexture ||
                                    sprite->gradientTexture != batchGradientTexture ||
                                    sprite->program != batchProgram || sprite->blendFunc != blendFunc;
                         });

        const auto quadCount = batchEnd - batchStart;

        if (!m_bufferAllocated || (m_quadIndex + quadCount > MaxQuadsPerBatch))
        {
            // orphan the old buffer and grab a new memory block
            m_vertexBuffer.allocate(MaxQuadsPerBatch * 4 * sizeof(SpriteVertex));
            m_quadIndex = 0;
            m_bufferAllocated = true;
        }

        auto *data = m_vertexBuffer.mapRange<GLfloat>(m_quadIndex * 4 * GLVertexSize, quadCount * 4 * GLVertexSize,
                                                      gl::Buffer::Access::Write | gl::Buffer::Access::Unsynchronized);
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

            emitVertex(quadPtr->vertices[0]);
            emitVertex(quadPtr->vertices[1]);
            emitVertex(quadPtr->vertices[2]);
            emitVertex(quadPtr->vertices[3]);
        }
        m_vertexBuffer.unmap();

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
            auto *shaderManager = sys::shaderManager();
            shaderManager->useProgram(batchProgram);
            shaderManager->setUniform("mvp", m_mvp);
            if (currentTexture)
                shaderManager->setUniform("baseColorTexture", 0);
            if (currentGradientTexture)
                shaderManager->setUniform("gradientTexture", 1);
        }

        if (currentBlendMode != blendFunc)
        {
            currentBlendMode = blendFunc;
            glBlendFunc(static_cast<GLenum>(blendFunc.sourceFactor), static_cast<GLenum>(blendFunc.destFactor));
        }

        glDrawElements(GL_TRIANGLES, 6 * quadCount, GL_UNSIGNED_INT,
                       reinterpret_cast<void *>(m_quadIndex * 6 * sizeof(uint32_t)));

        m_quadIndex += quadCount;
        batchStart = batchEnd;
    }

    m_quadCount = 0;
}

} // namespace muui
