#pragma once

#include "gl.h"

namespace muui::gl
{

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray &) = delete;
    VertexArray &operator=(const VertexArray &) = delete;

    VertexArray(VertexArray &&other);
    VertexArray &operator=(VertexArray &&other);

    void bind() const;
    void unbind() const;

    GLuint handle() const { return m_handle; }

    class Binder
    {
    public:
        Binder(const VertexArray *vao)
            : m_vao(vao)
        {
            m_vao->bind();
        }

        Binder(const Binder &) = delete;
        Binder &operator=(const Binder &) = delete;

        ~Binder() { m_vao->unbind(); }

    private:
        const VertexArray *m_vao{nullptr};
    };

private:
    GLuint m_handle{0};
};

} // namespace muui::gl
