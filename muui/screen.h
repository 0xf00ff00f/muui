#pragma once

#include "uiinput.h"

#include <glm/glm.hpp>

#include <memory>

namespace miniui
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

    miniui::Item *rootItem() const { return m_rootItem; }
    void setRootItem(miniui::Item *rootItem);

public:
    std::unique_ptr<miniui::Painter> m_painter;
    miniui::Item *m_rootItem = nullptr;
    miniui::Item *m_grabTarget = nullptr;
    miniui::Item *m_clickTarget = nullptr;
    bool m_dragStarted = false;
    glm::vec2 m_lastTouchPosition;
};

} // namespace miniui
