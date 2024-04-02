#pragma once

#include "util.h"

namespace muui
{

class Transform
{
public:
    void reset();
    void translate(const glm::vec2 &p);
    void rotate(float angle);

    glm::vec2 map(const glm::vec2 &p) const;

private:
    enum class Type
    {
        Identity,
        Translation,
        General
    };
    Type m_type{Type::Identity};
    glm::vec2 m_translation{0.0f}; // if m_type == Type::Translation
    glm::mat3 m_transform{1.0f};   // if m_type == Type::General
};

} // namespace muui
