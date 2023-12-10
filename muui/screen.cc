#include "screen.h"

#include "miniui.h"
#include "painter.h"

#include "gl.h"

namespace miniui
{
Screen::Screen()
    : m_painter(std::make_unique<Painter>())
{
}

Screen::~Screen() = default;

void Screen::resize(int width, int height)
{
    m_painter->setWindowSize(width, height);
}

int Screen::width() const
{
    return m_painter->windowWidth();
}

int Screen::height() const
{
    return m_painter->windowHeight();
}

void Screen::render() const
{
    if (!m_rootItem)
        return;

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    m_painter->begin();
    m_rootItem->render(m_painter.get(), glm::vec2(0, 0));
    m_painter->end();

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
}

void Screen::setRootItem(miniui::Item *rootItem)
{
    m_rootItem = rootItem;
}

bool Screen::handleTouchEvent(TouchAction action, int x, int y)
{
    if (!m_rootItem)
        return false;

    const auto pos = glm::vec2(x, y);
    switch (action)
    {
    case TouchAction::Down: {
        if (!m_rootItem->rect().contains(pos))
            return false;
        assert(!m_clickTarget);
        m_clickTarget = m_rootItem->mouseEvent({TouchEvent::Type::Press, pos});
        assert(!m_grabTarget);
        m_grabTarget = nullptr;
        m_dragStarted = false;
        m_lastTouchPosition = pos;
        return m_clickTarget != nullptr;
    }
    case TouchAction::Up: {
        bool clicked = false;
        if (m_rootItem->rect().contains(pos))
        {
            m_rootItem->mouseEvent({TouchEvent::Type::Release, pos});
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
            m_grabTarget = m_rootItem->mouseEvent({TouchEvent::Type::DragBegin, m_lastTouchPosition});
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

} // namespace miniui
