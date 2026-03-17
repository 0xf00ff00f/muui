#include "screen.h"

#include "item.h"
#include "painter.h"

#include "gl.h"

namespace muui
{
Screen::Screen()
    : m_painter(std::make_unique<Painter>())
{
    resizedSignal.connect([this](Size size) { m_painter->setWindowSize(size.width, size.height); });
}

Screen::~Screen() = default;

void Screen::render()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    m_painter->begin();
    Rectangle::render(m_painter.get());
    m_painter->end();

    glDisable(GL_BLEND);
}

bool Screen::handleTouchEvent(TouchAction action, int x, int y)
{
    const auto pos = glm::vec2(x, y);
    switch (action)
    {
    case TouchAction::Down: {
        assert(!m_clickTarget);
        for (auto &layoutItem : m_layoutItems)
        {
            auto *item = layoutItem.item();
            const auto size = item->size();
            const auto rect = RectF{layoutItem.offset, layoutItem.offset + glm::vec2{size.width, size.height}};
            if (rect.contains(pos))
            {
                m_clickTarget = item->mouseEvent({TouchEvent::Type::Press, pos - layoutItem.offset});
                if (m_clickTarget)
                    break;
            }
        }
        assert(!m_grabTarget);
        m_grabTarget = nullptr;
        m_dragStarted = false;
        m_lastTouchPosition = pos;
        return m_clickTarget != nullptr;
    }
    case TouchAction::Up: {
        bool clicked = false;
        for (auto &layoutItem : m_layoutItems)
        {
            auto *item = layoutItem.item();
            const auto size = item->size();
            const auto rect = RectF{layoutItem.offset, layoutItem.offset + glm::vec2{size.width, size.height}};
            if (rect.contains(pos))
            {
                item->mouseEvent({TouchEvent::Type::Release, pos - layoutItem.offset});
            }
        }
        if (m_clickTarget)
        {
            m_clickTarget->mouseEvent({TouchEvent::Type::Click, {}});
            clicked = true;
        }
        else if (m_grabTarget)
        {
            m_grabTarget->mouseEvent({TouchEvent::Type::DragEnd, {}});
        }
        m_clickTarget = nullptr;
        m_grabTarget = nullptr;
        m_dragStarted = false;
        return clicked;
    }
    case TouchAction::Move: {
        constexpr auto StartDragDistance = 10;
        if (!m_dragStarted && glm::distance(pos, m_lastTouchPosition) > StartDragDistance)
        {
            for (auto &layoutItem : m_layoutItems)
            {
                auto *item = layoutItem.item();
                const auto size = item->size();
                const auto rect = RectF{layoutItem.offset, layoutItem.offset + glm::vec2{size.width, size.height}};
                if (rect.contains(pos))
                {
                    m_grabTarget =
                        item->mouseEvent({TouchEvent::Type::DragBegin, m_lastTouchPosition - layoutItem.offset});
                    if (m_clickTarget)
                        break;
                }
            }
            m_clickTarget = nullptr;
            m_dragStarted = true;
        }
        if (m_grabTarget)
        {
            m_grabTarget->mouseEvent({TouchEvent::Type::DragMove, pos - m_lastTouchPosition});
            m_lastTouchPosition = pos;
        }
        break;
    }
    default:
        break;
    }

    return false;
}

} // namespace muui
