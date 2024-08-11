#pragma once

#include <type_traits>

namespace muui
{

template<typename EnumT>
requires std::is_enum_v<EnumT>
class Flags
{
public:
    using UnderlyingType = std::underlying_type_t<EnumT>;

    Flags() = default;
    Flags(EnumT flag)
        : Flags(static_cast<UnderlyingType>(flag))
    {
    }

    bool testFlag(EnumT flag) const { return static_cast<EnumT>(m_value & static_cast<UnderlyingType>(flag)) == flag; }

    friend inline Flags operator|(const Flags &lhs, const Flags &rhs) { return {lhs.m_value | rhs.m_value}; }

    Flags &operator|=(const Flags &rhs)
    {
        *this = *this | rhs;
        return *this;
    }

    friend inline Flags operator&(const Flags &lhs, const Flags &rhs) { return {lhs.m_value & rhs.m_value}; }

    Flags &operator&=(const Flags &rhs)
    {
        *this = *this & rhs;
        return *this;
    }

    explicit operator UnderlyingType() const { return static_cast<UnderlyingType>(m_value); }

    bool operator==(const Flags &) const = default;

private:
    Flags(UnderlyingType flags)
        : m_value(flags)
    {
    }
    UnderlyingType m_value{};
};

#define MUUI_DEFINE_FLAGS(FlagsName, Enum)                                                                             \
    inline Flags<Enum> operator|(Enum lhs, Enum rhs)                                                                   \
    {                                                                                                                  \
        return Flags<Enum>(lhs) | rhs;                                                                                 \
    }                                                                                                                  \
    inline Flags<Enum> operator&(Enum lhs, Enum rhs)                                                                   \
    {                                                                                                                  \
        return Flags<Enum>(lhs) & rhs;                                                                                 \
    }                                                                                                                  \
    using FlagsName = Flags<Enum>;

} // namespace muui
