#include "transform.h"

#include <glm/gtx/matrix_transform_2d.hpp>

#include <cassert>

namespace muui
{

void Transform::reset()
{
    m_type = Type::Identity;
}

void Transform::translate(const glm::vec2 &p)
{
    if (p == glm::vec2(0.0f))
        return;
    switch (m_type)
    {
    case Type::Identity:
        m_type = Type::Translation;
        m_translation = p;
        break;
    case Type::Translation:
        m_translation += p;
        break;
    case Type::General:
        const auto t = glm::translate(glm::mat3(1.0), p);
        m_transform *= t;
        break;
    }
}

void Transform::rotate(float angle)
{
    if (angle == 0.0f)
        return;
    const auto r = glm::rotate(glm::mat3(1.0), angle);
    switch (m_type)
    {
    case Type::Identity:
        m_transform = r;
        m_type = Type::General;
        break;
    case Type::Translation: {
        const auto t = glm::translate(glm::mat3(1.0), m_translation);
        m_transform = t * r;
        m_type = Type::General;
        break;
    }
    default:
    case Type::General:
        m_transform *= r;
        break;
    }
}

void Transform::scale(const glm::vec2 &v)
{
    if (v == glm::vec2(1.0f))
        return;
    const auto s = glm::scale(glm::mat3(1.0), v);
    switch (m_type)
    {
    case Type::Identity:
        m_transform = s;
        m_type = Type::General;
        break;
    case Type::Translation: {
        const auto t = glm::translate(glm::mat3(1.0), m_translation);
        m_transform = t * s;
        m_type = Type::General;
        break;
    }
    default:
    case Type::General:
        m_transform *= s;
        break;
    }
}

glm::vec2 Transform::map(const glm::vec2 &p) const
{
    switch (m_type)
    {
    case Type::Identity:
        return p;
    case Type::Translation:
        return p + m_translation;
    default:
    case Type::General:
        return glm::vec2(m_transform * glm::vec3(p, 1.0f));
    }
}

} // namespace muui
