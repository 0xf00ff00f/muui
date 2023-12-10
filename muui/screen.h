#pragma once

#include "uiinput.h"

#include <glm/glm.hpp>

#include <memory>

namespace muui
{
class Item;
class Painter;

class Screen
{
public:
    Screen();
    ~Screen();

    void resize(int width, int height);
    void render() const;
    bool handleTouchEvent(TouchAction type, int x, int y);

    int width() const;
    int height() const;

    muui::Item *rootItem() const { return m_rootItem; }
    void setRootItem(Item *rootItem);

public:
    std::unique_ptr<muui::Painter> m_painter;
    Item *m_rootItem = nullptr;
    Item *m_grabTarget = nullptr;
    Item *m_clickTarget = nullptr;
    bool m_dragStarted = false;
    glm::vec2 m_lastTouchPosition;
};

} // namespace muui
