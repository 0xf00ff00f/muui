#include "item.h"

#include "fontcache.h"
#include "painter.h"
#include "pixmapcache.h"
#include "system.h"

#include <algorithm>
#include <memory>

namespace muui
{
namespace
{
Font *defaultFont()
{
    return getFontCache()->font("OpenSans_Regular", 40);
}
} // namespace

Item::~Item() = default;

void Item::update(float) {}

std::vector<Item *> Item::children() const
{
    return {};
}

void Item::renderBackground(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!fillBackground)
        return;
    const auto rect = RectF{pos, pos + glm::vec2(width(), height())};
    switch (shape)
    {
    case Shape::Rectangle:
        painter->drawRect(rect, backgroundColor, depth);
        break;
    case Shape::Capsule:
        painter->drawCapsule(rect, backgroundColor, depth);
        break;
    case Shape::Circle:
        painter->drawCircle(rect.center(), 0.5f * std::max(rect.width(), rect.height()), backgroundColor, depth);
        break;
    case Shape::RoundedRectangle:
        painter->drawRoundedRect(rect, cornerRadius, backgroundColor, depth);
        break;
    default:
        break;
    }
}

void Item::setSize(Size size)
{
    if (size == m_size)
        return;
    m_size = size;
    if (!m_effects.empty())
    {
        const auto w = static_cast<int>(std::ceil(size.width));
        const auto h = static_cast<int>(std::ceil(size.height));
        for (auto &effect : m_effects)
            effect->resize(w, h);
    }
    resizedSignal(m_size);
}

void Item::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!visible)
        return;
    const auto rect = RectF{pos, pos + glm::vec2(width(), height())};
    if (!painter->clipRect().intersects(rect))
        return;
    if (m_effects.empty())
    {
        doRender(painter, pos, depth);
    }
    else
    {
        auto *effect = m_effects.front().get(); // TODO: multiple effects
        effect->render(*this, painter, pos, depth);
    }
}

void Item::doRender(Painter *painter, const glm::vec2 &pos, int depth)
{
    renderBackground(painter, pos, depth);
    renderContents(painter, pos, depth);
}

Item *Item::mouseEvent(const TouchEvent &event)
{
    if (!visible)
        return nullptr;
    return handleMouseEvent(event);
}

Item *Item::handleMouseEvent(const TouchEvent &)
{
    return nullptr;
}

Rectangle::Rectangle()
    : Rectangle(Size{})
{
}

Rectangle::Rectangle(float width, float height)
    : Rectangle(Size{width, height})
{
}

Rectangle::Rectangle(Size size)
{
    setSize(size);
}

void Rectangle::setSize(float width, float height)
{
    setSize({width, height});
}

void Rectangle::setWidth(float width)
{
    setSize({width, m_size.height});
}

void Rectangle::setHeight(float height)
{
    setSize({m_size.width, height});
}

void Rectangle::renderContents(Painter *, const glm::vec2 &, int) {}

Label::Label(std::u32string_view text)
    : Label(defaultFont(), text)
{
}

Label::Label(Font *font, std::u32string_view text)
    : m_font(font)
    , m_text(text)
{
    updateSizeAndOffset();
}

Item *Label::handleMouseEvent(const TouchEvent &event)
{
    switch (event.type)
    {
    case TouchEvent::Type::Press:
    case TouchEvent::Type::Release:
        return rect().contains(event.position) ? this : nullptr;
    case TouchEvent::Type::Click:
        clickedSignal();
        return this;
    default:
        return nullptr;
    }
}

void Label::setFont(Font *font)
{
    if (font == m_font)
        return;
    m_font = font;
    updateSizeAndOffset();
}

void Label::setText(std::u32string_view text)
{
    if (text == m_text)
        return;
    m_text = text;
    updateSizeAndOffset();
}

void Label::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSizeAndOffset();
}

void Label::setFixedWidth(float width)
{
    if (width == m_fixedWidth)
        return;
    m_fixedWidth = width;
    updateSizeAndOffset();
}

void Label::setFixedHeight(float height)
{
    if (height == m_fixedHeight)
        return;
    m_fixedHeight = height;
    updateSizeAndOffset();
}

void Label::setAlignment(Alignment alignment)
{
    if (alignment == m_alignment)
        return;
    m_alignment = alignment;
    updateSizeAndOffset();
}

void Label::updateSizeAndOffset()
{
    m_contentHeight = m_font->pixelHeight();
    m_contentWidth = m_font->textWidth(m_text);
    const float height = [this] {
        if (m_fixedHeight > 0)
            return m_fixedHeight;
        return m_contentHeight + m_margins.top + m_margins.bottom;
    }();
    const float width = [this] {
        if (m_fixedWidth > 0)
            return m_fixedWidth;
        return m_contentWidth + m_margins.left + m_margins.right;
    }();
    setSize({width, height});

    const auto availableWidth = std::max(m_size.width - (m_margins.left + m_margins.right), 0.0f);
    const auto availableHeight = std::max(m_size.height - (m_margins.top + m_margins.bottom), 0.0f);

    const auto xOffset = [this, availableWidth] {
        const auto horizAlignment = m_alignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
        switch (horizAlignment)
        {
        case Alignment::Left:
        default:
            return 0.0f;
        case Alignment::HCenter:
            return 0.5f * (availableWidth - m_contentWidth);
        case Alignment::Right:
            return availableWidth - m_contentWidth;
        }
    }();
    const auto yOffset = [this, availableHeight] {
        const auto vertAlignment = m_alignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
        switch (vertAlignment)
        {
        case Alignment::Top:
            return 0.0f;
        case Alignment::VCenter:
        default:
            return 0.5f * (availableHeight - m_contentHeight);
        case Alignment::Bottom:
            return availableHeight - m_contentHeight;
        }
    }();
    m_offset = glm::vec2(m_margins.left, m_margins.top) + glm::vec2(xOffset, yOffset);
}

void Label::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return;

    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    if (availableHeight < 0.0f)
        return;

    const bool clipped = availableWidth < m_contentWidth - 0.5f || availableHeight < m_contentHeight - 0.5f;
    RectF prevClipRect;
    if (clipped)
    {
        prevClipRect = painter->clipRect();
        const auto p = pos + glm::vec2(m_margins.left, m_margins.top);
        const auto rect = RectF{p, p + glm::vec2(availableWidth, availableHeight)};
        painter->setClipRect(prevClipRect.intersected(rect));
    }

    const auto textPos = pos + m_offset;
    painter->setFont(m_font);
    if (shadowEnabled)
        painter->drawText(m_text, textPos + shadowOffset, shadowColor, depth + 1);
    painter->drawText(m_text, textPos, color, depth + 2);

    if (clipped)
        painter->setClipRect(prevClipRect);
}

Image::Image() = default;

Image::Image(std::string_view source)
{
    setSource(source);
}

Item *Image::handleMouseEvent(const TouchEvent &)
{
    return nullptr;
}

void Image::setSource(std::string_view source)
{
    if (source == m_source)
        return;
    m_source = source;
    m_pixmap = [this] {
        auto *cache = getPixmapCache();
        return cache->pixmap(m_source);
    }();
    updateSizeAndOffset();
}

void Image::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSizeAndOffset();
}

void Image::setFixedWidth(float width)
{
    if (width == m_fixedWidth)
        return;
    m_fixedWidth = width;
    updateSizeAndOffset();
}

void Image::setFixedHeight(float height)
{
    if (height == m_fixedHeight)
        return;
    m_fixedHeight = height;
    updateSizeAndOffset();
}

void Image::setAlignment(Alignment alignment)
{
    if (alignment == m_alignment)
        return;
    m_alignment = alignment;
    updateSizeAndOffset();
}

void Image::updateSizeAndOffset()
{
    const float height = [this] {
        if (m_fixedHeight > 0)
            return m_fixedHeight;
        float height = m_margins.top + m_margins.bottom;
        if (m_pixmap)
            height += m_pixmap->height;
        return height;
    }();
    const float width = [this] {
        if (m_fixedWidth > 0)
            return m_fixedWidth;
        float width = m_margins.left + m_margins.right;
        if (m_pixmap)
            width += m_pixmap->width;
        return width;
    }();
    setSize({width, height});

    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);

    const auto xOffset = [this, availableWidth] {
        const auto horizAlignment = m_alignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
        switch (horizAlignment)
        {
        case Alignment::Left:
        default:
            return 0.0f;
        case Alignment::HCenter:
            return 0.5f * (availableWidth - m_pixmap->width);
        case Alignment::Right:
            return availableWidth - m_pixmap->width;
        }
    }();
    const auto yOffset = [this, availableHeight] {
        const auto vertAlignment = m_alignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
        switch (vertAlignment)
        {
        case Alignment::Top:
            return 0.0f;
        case Alignment::VCenter:
        default:
            return 0.5f * (availableHeight - m_pixmap->height);
        case Alignment::Bottom:
            return availableHeight - m_pixmap->height;
        }
    }();
    m_offset = glm::vec2(xOffset, yOffset);
}

void Image::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!m_pixmap)
        return;
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    const bool clipped = availableWidth < m_pixmap->width - 0.5f || availableHeight < m_pixmap->height - 0.5f;

    const auto topLeft = pos + glm::vec2(m_margins.left, m_margins.top);
    const auto imagePos = topLeft + m_offset;
    const auto rect = RectF{imagePos, imagePos + glm::vec2(m_pixmap->width, m_pixmap->height)};
    if (!clipped)
    {
        painter->drawPixmap(*m_pixmap, rect, color, depth);
    }
    else
    {
        const auto clipRect = RectF{topLeft, topLeft + glm::vec2(availableWidth, availableHeight)};
        painter->drawPixmap(*m_pixmap, rect, clipRect, color, depth);
    }
}

void Container::update(float elapsed)
{
    for (auto &layoutItem : m_layoutItems)
        layoutItem->item->update(elapsed);
}

Item *Container::handleMouseEvent(const TouchEvent &event)
{
    switch (event.type)
    {
    case TouchEvent::Type::Press:
    case TouchEvent::Type::Release:
    case TouchEvent::Type::DragBegin: {
        const auto &pos = event.position;
        if (!rect().contains(pos))
            return nullptr;
        // back to front, we want items that are drawn last to be tested first
        for (auto it = m_layoutItems.rbegin(); it != m_layoutItems.rend(); ++it)
        {
            const auto &item = (*it)->item;
            const auto &offset = (*it)->offset;
            const auto childRect = RectF{offset, offset + glm::vec2(item->width(), item->height())};
            if (childRect.contains(pos))
            {
                TouchEvent childEvent = event;
                childEvent.position -= offset;
                if (auto *handler = item->mouseEvent(childEvent); handler)
                    return handler;
            }
        }
        return nullptr;
    }
    default:
        return nullptr;
    }
}

std::vector<Item *> Container::children() const
{
    std::vector<Item *> children;
    children.reserve(m_layoutItems.size());
    std::transform(m_layoutItems.begin(), m_layoutItems.end(), std::back_inserter(children),
                   [](auto &layoutItem) { return layoutItem->item.get(); });
    return children;
}

void Container::insert(std::size_t index, std::unique_ptr<Item> item)
{
    auto resizedConnection = item->resizedSignal.connect([this](Size size) { updateLayout(); });
    auto layoutItem = std::make_unique<LayoutItem>(LayoutItem{{}, std::move(item), resizedConnection});
    m_layoutItems.insert(std::next(m_layoutItems.begin(), index), std::move(layoutItem));
    updateLayout();
}

void Container::append(std::unique_ptr<Item> item)
{
    insert(m_layoutItems.size(), std::move(item));
}

std::unique_ptr<Item> Container::takeAt(std::size_t index)
{
    if (index >= m_layoutItems.size())
        return {};
    auto it = std::next(m_layoutItems.begin(), index);
    (*it)->resizedConnection.disconnect();
    auto item = std::move((*it)->item);
    m_layoutItems.erase(it);
    return item;
}

Item *Container::at(std::size_t index) const
{
    return index < m_layoutItems.size() ? m_layoutItems[index]->item.get() : nullptr;
}

void Container::remove(std::size_t index)
{
    if (index >= m_layoutItems.size())
        return;
    m_layoutItems.erase(std::next(m_layoutItems.begin(), index));
    updateLayout();
}

void Container::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateLayout();
}

void Container::setSpacing(float spacing)
{
    if (spacing == m_spacing)
        return;
    m_spacing = spacing;
    updateLayout();
}

void Container::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    for (auto &layoutItem : m_layoutItems)
        layoutItem->item->render(painter, pos + layoutItem->offset, depth + 1);
}

void Column::setMinimumWidth(float width)
{
    if (width == m_minimumWidth)
        return;
    m_minimumWidth = width;
    updateLayout();
}

void Column::updateLayout()
{
    // update size
    float width = std::max(m_minimumWidth - (m_margins.left + m_margins.right), 0.0f);
    float height = 0;
    for (auto &layoutItem : m_layoutItems)
    {
        auto &item = layoutItem->item;
        width = std::max(width, item->width());
        height += item->height();
    }
    if (!m_layoutItems.empty())
        height += (m_layoutItems.size() - 1) * m_spacing;
    width += m_margins.left + m_margins.right;
    height += m_margins.top + m_margins.bottom;
    setSize({width, height});

    // update item offsets
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, &item = layoutItem->item] {
            const auto alignment = item->containerAlignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
            const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
            switch (alignment)
            {
            case Alignment::Left:
            default:
                return 0.0f;
            case Alignment::HCenter:
                return 0.5f * (availableWidth - item->width());
            case Alignment::Right:
                return availableWidth - item->width();
            }
        }();
        layoutItem->offset = p + glm::vec2(offset, 0.0f);
        p.y += layoutItem->item->height() + m_spacing;
    }
}

void Row::setMinimumHeight(float height)
{
    if (height == m_minimumHeight)
        return;
    m_minimumHeight = height;
    updateLayout();
}

void Row::updateLayout()
{
    // update size
    float width = 0;
    float height = std::max(m_minimumHeight - (m_margins.top + m_margins.bottom), 0.0f);
    for (auto &layoutItem : m_layoutItems)
    {
        auto &item = layoutItem->item;
        width += item->width();
        height = std::max(height, item->height());
    }
    if (!m_layoutItems.empty())
        width += (m_layoutItems.size() - 1) * m_spacing;
    width += m_margins.left + m_margins.right;
    height += m_margins.top + m_margins.bottom;
    setSize({width, height});

    // update item offsets
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, &item = layoutItem->item] {
            const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
            const auto alignment = item->containerAlignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
            switch (alignment)
            {
            case Alignment::Top:
                return 0.0f;
            case Alignment::VCenter:
            default:
                return 0.5f * (availableHeight - item->height());
            case Alignment::Bottom:
                return availableHeight - item->height();
            }
        }();
        layoutItem->offset = p + glm::vec2(0.0f, offset);
        p.x += layoutItem->item->width() + m_spacing;
    }
}

ScrollArea::ScrollArea(float viewportWidth, float viewportHeight, std::unique_ptr<Item> contentItem)
    : m_contentItem(std::move(contentItem))
{
    updateSize();
}

ScrollArea::ScrollArea(std::unique_ptr<Item> contentItem)
    : ScrollArea(0, 0, std::move(contentItem))
{
}

void ScrollArea::update(float elapsed)
{
    m_contentItem->update(elapsed);
}

void ScrollArea::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto viewportPos = pos + glm::vec2(m_margins.left, m_margins.top);
    const auto prevClipRect = painter->clipRect();
    const auto viewportRect = RectF{viewportPos, viewportPos + glm::vec2(m_viewportSize.width, m_viewportSize.height)};
    painter->setClipRect(prevClipRect.intersected(viewportRect));
    m_contentItem->render(painter, viewportPos + m_viewportOffset, depth + 1);
    painter->setClipRect(prevClipRect);
}

Item *ScrollArea::handleMouseEvent(const TouchEvent &event)
{
    const auto &pos = event.position;
    switch (event.type)
    {
    case TouchEvent::Type::DragEnd:
        return this;
    case TouchEvent::Type::DragMove: {
        m_viewportOffset += pos;
        m_viewportOffset = glm::max(m_viewportOffset, glm::vec2(m_viewportSize.width - m_contentItem->width(),
                                                                m_viewportSize.height - m_contentItem->height()));
        m_viewportOffset = glm::min(m_viewportOffset, glm::vec2(0, 0));
        return this;
    }
    case TouchEvent::Type::Press:
    case TouchEvent::Type::Release:
    case TouchEvent::Type::DragBegin: {
        if (!rect().contains(pos))
            return nullptr;
        TouchEvent childEvent = event;
        childEvent.position -= m_viewportOffset;
        if (auto *handler = m_contentItem->mouseEvent(childEvent); handler)
            return handler;
        return event.type == TouchEvent::Type::DragBegin ? this : nullptr;
    }
    default:
        return nullptr;
    }
}

std::vector<Item *> ScrollArea::children() const
{
    return {m_contentItem.get()};
}

void ScrollArea::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSize();
}

void ScrollArea::setViewportSize(Size size)
{
    if (size == m_viewportSize)
        return;
    m_viewportSize = size;
    updateSize();
}

void ScrollArea::updateSize()
{
    float height = m_viewportSize.height + m_margins.top + m_margins.bottom;
    float width = m_viewportSize.width + m_margins.left + m_margins.right;
    setSize({width, height});
}

Switch::Switch()
    : Switch(Size{80, 32})
{
}

Switch::Switch(float width, float height)
    : Switch(Size{width, height})
{
}

Switch::Switch(Size size)
{
    setSize(size);

    fillBackground = true;
    shape = Shape::Capsule;
    backgroundColor = glm::vec4(0, 0, 0, 1);

    m_animation.valueChangedSignal.connect([this](float value) { m_indicatorPosition = value; });
    m_animation.duration = 0.2f;
}

Item *Switch::handleMouseEvent(const TouchEvent &event)
{
    switch (event.type)
    {
    case TouchEvent::Type::Press:
    case TouchEvent::Type::Release:
        return rect().contains(event.position) ? this : nullptr;
    case TouchEvent::Type::Click:
        toggle();
        return this;
    default:
        return nullptr;
    }
}

void Switch::toggle()
{
    setChecked(!m_checked);
}

void Switch::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;

    if (m_checked)
    {
        m_animation.startValue = 0.0f;
        m_animation.endValue = 1.0f;
    }
    else
    {
        m_animation.startValue = 1.0f;
        m_animation.endValue = 0.0f;
    }
    m_animation.start();
    toggledSignal(checked);
}

void Switch::update(float elapsed)
{
    m_animation.update(elapsed);
}

void Switch::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const float radius = 0.5f * m_size.height;
    const float indicatorRadius = 0.75f * radius;
    const float centerX = radius + m_indicatorPosition * (m_size.width - 2 * radius);
    const auto center = pos + glm::vec2(centerX, 0.5f * m_size.height);
    painter->drawCircle(center, indicatorRadius, indicatorColor, depth + 1);
}

MultiLineText::MultiLineText(std::u32string_view text)
    : MultiLineText(defaultFont(), text)
{
}

MultiLineText::MultiLineText(Font *font, std::u32string_view text)
    : m_font(font)
    , m_text(text)
{
    updateSize();
}

void MultiLineText::setFont(Font *font)
{
    if (font == m_font)
        return;
    m_font = font;
    updateSize();
}

void MultiLineText::setText(std::u32string_view text)
{
    if (text == m_text)
        return;
    m_text = text;
    updateSize();
}

void MultiLineText::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSize();
}

void MultiLineText::setFixedWidth(float width)
{
    if (width == m_fixedWidth)
        return;
    m_fixedWidth = width;
    updateSize();
}

void MultiLineText::setFixedHeight(float height)
{
    if (height == m_fixedHeight)
        return;
    m_fixedHeight = height;
    updateSize();
}

void MultiLineText::updateSize()
{
    breakTextLines();
    m_contentWidth = 0.0f;
    for (const auto &line : m_lines)
        m_contentWidth = std::max(line.width, m_contentWidth);
    m_contentHeight = m_lines.size() * m_font->pixelHeight();
    const float height = [this] {
        if (m_fixedHeight > 0)
            return m_fixedHeight;
        return m_contentHeight + m_margins.top + m_margins.bottom;
    }();
    setSize({m_fixedWidth, height});
}

void MultiLineText::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return;

    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    if (availableHeight < 0.0f)
        return;

    const bool clipped = availableWidth < m_contentWidth - 0.5f || availableHeight < m_contentHeight - 0.5f;
    RectF prevClipRect;
    if (clipped)
    {
        prevClipRect = painter->clipRect();
        const auto p = pos + glm::vec2(m_margins.left, m_margins.top);
        const auto rect = RectF{p, p + glm::vec2(availableWidth, availableHeight)};
        painter->setClipRect(prevClipRect.intersected(rect));
    }

    const auto yOffset = [this, availableHeight] {
        const auto vertAlignment = alignment & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
        switch (vertAlignment)
        {
        case Alignment::Top:
            return 0.0f;
        case Alignment::VCenter:
        default:
            return 0.5f * (availableHeight - m_contentHeight);
        case Alignment::Bottom:
            return availableHeight - m_contentHeight;
        }
    }();
    auto textPos = pos + glm::vec2(m_margins.left, m_margins.top) + glm::vec2(0.0f, yOffset);
    painter->setFont(m_font);
    for (const auto &line : m_lines)
    {
        const auto offset = [this, &line, availableWidth] {
            const auto horizAlignment = alignment & (Alignment::Left | Alignment::HCenter | Alignment::Right);
            switch (horizAlignment)
            {
            case Alignment::Left:
            default:
                return 0.0f;
            case Alignment::HCenter:
                return 0.5f * (availableWidth - line.width);
            case Alignment::Right:
                return availableWidth - line.width;
            }
        }();
        painter->drawText(line.text, textPos + glm::vec2(offset, 0), color, depth + 1);
        textPos.y += m_font->pixelHeight();
    }

    if (clipped)
        painter->setClipRect(prevClipRect);
}

void MultiLineText::breakTextLines()
{
    assert(m_font);

    m_lines.clear();

    const float availableWidth = m_fixedWidth - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return;

    struct Position
    {
        std::u32string::const_iterator it;
        float x;
    };
    Position rowStart{m_text.begin(), 0.0f};
    std::optional<Position> lastBreak;

    const auto spaceWidth = m_font->glyph(' ')->advanceWidth;

    const auto makeLine = [](Position start, Position end) {
        const auto *from = &*start.it;
        const auto count = std::distance(start.it, end.it);
        return TextLine{std::u32string_view(from, count), end.x - start.x};
    };

    float lineWidth = 0.0f;
    for (auto it = m_text.begin(); it != m_text.end(); ++it)
    {
        const auto ch = *it;
        if (ch == ' ')
        {
            if (lineWidth - rowStart.x > availableWidth)
            {
                if (lastBreak)
                {
                    m_lines.push_back(makeLine(rowStart, *lastBreak));
                    rowStart = {lastBreak->it + 1, lastBreak->x + spaceWidth};
                    lastBreak = {it, lineWidth};
                }
                else
                {
                    m_lines.push_back(makeLine(rowStart, Position{it, lineWidth}));
                    rowStart = {it + 1, lineWidth + spaceWidth};
                }
            }
            else
            {
                lastBreak = {it, lineWidth};
            }
        }
        lineWidth += m_font->glyph(ch)->advanceWidth;
    }
    if (rowStart.it != m_text.end())
    {
        if (lineWidth - rowStart.x > availableWidth && lastBreak)
        {
            m_lines.push_back(makeLine(rowStart, *lastBreak));
            m_lines.push_back(
                makeLine(Position{lastBreak->it + 1, lastBreak->x + spaceWidth}, Position{m_text.end(), lineWidth}));
        }
        else
        {
            m_lines.push_back(makeLine(rowStart, Position{m_text.end(), lineWidth}));
        }
    }
}

Item *Button::handleMouseEvent(const TouchEvent &event)
{
    switch (event.type)
    {
    case TouchEvent::Type::Press:
    case TouchEvent::Type::Release:
        return rect().contains(event.position) ? this : nullptr;
    case TouchEvent::Type::Click:
        clickedSignal();
        return this;
    default:
        return nullptr;
    }
}

Item *ImageButton::handleMouseEvent(const TouchEvent &event)
{
    switch (event.type)
    {
    case TouchEvent::Type::Press:
    case TouchEvent::Type::Release:
        return rect().contains(event.position) ? this : nullptr;
    case TouchEvent::Type::Click:
        clickedSignal();
        return this;
    default:
        return nullptr;
    }
}

} // namespace muui
