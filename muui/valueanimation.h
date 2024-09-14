#pragma once

#include <muslots/muslots.h>

#include <glm/glm.hpp>

namespace muui
{

template<typename F>
class ValueAnimation
{
public:
    using T = typename F::Type;

    T startValue = T{};
    T endValue = T{};
    float duration = 0.0f;

    T value() const
    {
        constexpr F tweener;
        float t = std::min(m_t / duration, 1.0f);
        return glm::mix(startValue, endValue, tweener(t));
    }

    void update(float elapsed)
    {
        if (!m_active)
            return;
        float t = m_t + elapsed;
        if (t == m_t)
            return;
        if (t > duration)
        {
            t = duration;
            m_active = false;
        }
        m_t = t;
        valueChangedSignal(value());
        if (!m_active)
            finishedSignal();
    }

    void start()
    {
        m_t = 0.0f;
        m_active = true;
        valueChangedSignal(value());
    }

    void stop() { m_active = false; }

    bool active() const { return m_active; }

    muslots::Signal<const T &> valueChangedSignal;
    muslots::Signal<> finishedSignal;

private:
    float m_t = 0.0f;
    bool m_active = false;
};
} // namespace muui
