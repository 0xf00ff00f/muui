#pragma once

#include "item.h"
#include "uiinput.h"

#include <glm/glm.hpp>

#include <memory>

namespace muui
{
class Item;
class Painter;

class Screen : public Rectangle
{
public:
    Screen();
    ~Screen();

    void render();
    bool handleTouchEvent(TouchAction type, int x, int y);

public:
    std::unique_ptr<muui::Painter> m_painter;
    Item *m_grabTarget = nullptr;
    Item *m_clickTarget = nullptr;
    bool m_dragStarted = false;
    glm::vec2 m_lastTouchPosition;
};

} // namespace muui
