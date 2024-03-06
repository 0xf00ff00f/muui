#include "item.h"

#include "fontcache.h"
#include "painter.h"
#include "pixmapcache.h"
#include "shadereffect.h"
#include "system.h"

#include <algorithm>
#include <memory>

namespace muui
{
namespace
{
Font *defaultFont()
{
    return sys::fontCache()->font("OpenSans_Regular", 40);
}
} // namespace

Item::LayoutItem::LayoutItem(std::unique_ptr<Item> item, Item *parent)
    : m_item(std::move(item))
    , m_resizedConnection{m_item->resizedSignal.connect([parent](Size) { parent->handleChildUpdated(); })}
    , m_anchorChangedConnection{m_item->anchorChangedSignal.connect([parent] { parent->updateLayout(); })}
    , m_alignmentChangedConnection{m_item->alignmentChangedSignal.connect([parent] { parent->updateLayout(); })}
{
}

Item::LayoutItem::~LayoutItem()
{
    m_resizedConnection.disconnect();
    m_anchorChangedConnection.disconnect();
    m_alignmentChangedConnection.disconnect();
}

Item::Item() = default;
Item::~Item() = default;

void Item::update(float elapsed)
{
    for (auto &layoutItem : m_layoutItems)
        layoutItem.item()->update(elapsed);
}

void Item::removeChild(std::size_t index)
{
    if (index >= m_layoutItems.size())
        return;
    m_layoutItems.erase(std::next(m_layoutItems.begin(), index));
    handleChildUpdated();
}

std::unique_ptr<Item> Item::takeChildAt(std::size_t index)
{
    if (index >= m_layoutItems.size())
        return {};
    auto it = std::next(m_layoutItems.begin(), index);
    auto item = it->takeItem();
    m_layoutItems.erase(it);
    handleChildUpdated();
    return item;
}

Item *Item::childAt(std::size_t index) const
{
    return index < m_layoutItems.size() ? m_layoutItems[index].item() : nullptr;
}

RectF Item::childRect(std::size_t index)
{
    if (index >= m_layoutItems.size())
        return {};
    const auto &layoutItem = m_layoutItems[index];
    const auto &size = layoutItem.item()->m_size;
    return RectF{layoutItem.offset, layoutItem.offset + glm::vec2{size.width, size.height}};
}

std::vector<Item *> Item::children() const
{
    std::vector<Item *> children;
    children.reserve(m_layoutItems.size());
    std::transform(m_layoutItems.begin(), m_layoutItems.end(), std::back_inserter(children),
                   [](auto &layoutItem) { return layoutItem.item(); });
    return children;
}

bool Item::renderBackground(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!fillBackground)
        return false;
    const auto rect = RectF{pos, pos + glm::vec2(width(), height())};
    switch (shape)
    {
    case Shape::Rectangle:
        painter->drawRect(rect, depth);
        return true;
    case Shape::Capsule:
        painter->drawCapsule(rect, depth);
        return true;
    case Shape::Circle:
        painter->drawCircle(rect.center(), 0.5f * std::max(rect.width(), rect.height()), depth);
        return true;
    case Shape::RoundedRectangle:
        painter->drawRoundedRect(rect, cornerRadius, depth);
        return true;
    default:
        return false;
    }
}

bool Item::renderContents(Painter *, const glm::vec2 &, int)
{
    return false;
}

void Item::setSize(Size size)
{
    if (size == m_size)
        return;
    m_size = size;
    updateLayout();
    resizedSignal(m_size);
}

void Item::handleChildUpdated()
{
    updateLayout();
}

namespace
{

Brush adjustToRect(const LinearGradient &gradient, const RectF &rect)
{
    const auto start = glm::mix(rect.min, rect.max, gradient.start);
    const auto end = glm::mix(rect.min, rect.max, gradient.end);
    return LinearGradient{.texture = gradient.texture, .start = start, .end = end};
}

Brush adjustToRect(const Color &color, const RectF &)
{
    return color;
}

} // namespace

Brush Item::adjustBrushToRect(const Brush &brush, const glm::vec2 &topLeft) const
{
    const RectF rect{topLeft, topLeft + glm::vec2(m_size.width, m_size.height)};
    return std::visit([&rect](auto &brush) { return adjustToRect(brush, rect); }, brush);
}

void Item::updateLayout()
{
    for (auto &layoutItem : m_layoutItems)
    {
        const auto *item = layoutItem.item();
        const auto anchorX = [this, &item]() -> float {
            const auto &position = item->m_horizontalAnchor.position;
            switch (position.type)
            {
            case Length::Type::Pixels:
            default:
                return position.value;
            case Length::Type::Percent:
                return position.value * m_size.width;
            }
        }();
        const auto offsetX = [this, &item, anchorX]() -> float {
            const auto &anchor = item->m_horizontalAnchor;
            switch (anchor.type)
            {
            case HorizontalAnchor::Type::Left:
            default:
                return anchorX;
            case HorizontalAnchor::Type::Center:
                return anchorX - 0.5f * item->m_size.width;
            case HorizontalAnchor::Type::Right:
                return anchorX - item->m_size.width;
            }
        }();
        const auto anchorY = [this, &item]() -> float {
            const auto &position = item->m_verticalAnchor.position;
            switch (position.type)
            {
            case Length::Type::Pixels:
            default:
                return position.value;
            case Length::Type::Percent:
                return position.value * m_size.height;
            }
        }();
        const auto offsetY = [this, &item, anchorY]() -> float {
            const auto &anchor = item->m_verticalAnchor;
            switch (anchor.type)
            {
            case VerticalAnchor::Type::Top:
            default:
                return anchorY;
            case VerticalAnchor::Type::Center:
                return anchorY - 0.5f * item->m_size.height;
            case VerticalAnchor::Type::Bottom:
                return anchorY - item->m_size.height;
            }
        }();
        layoutItem.offset = glm::vec2{offsetX, offsetY};
    }
}

void Item::render(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!visible)
        return;
    const auto rect = RectF{pos, pos + glm::vec2(width(), height())};
    if (!painter->clipRect().intersects(rect))
        return;
    if (!m_effect)
    {
        doRender(painter, pos, depth);
    }
    else
    {
        m_effect->render(painter, pos, depth);
    }
}

namespace
{
class PainterBrushSaver
{
public:
    explicit PainterBrushSaver(Painter *painter)
        : m_painter(painter)
        , m_backgroundBrush(painter->backgroundBrush())
        , m_foregroundBrush(painter->foregroundBrush())
        , m_outlineBrush(painter->outlineBrush())
    {
    }

    ~PainterBrushSaver()
    {
        m_painter->setBackgroundBrush(m_backgroundBrush);
        m_painter->setForegroundBrush(m_foregroundBrush);
        m_painter->setOutlineBrush(m_outlineBrush);
    }

private:
    Painter *m_painter;
    std::optional<Brush> m_backgroundBrush;
    std::optional<Brush> m_foregroundBrush;
    std::optional<Brush> m_outlineBrush;
};
}; // namespace

void Item::doRender(Painter *painter, const glm::vec2 &pos, int depth)
{
    PainterBrushSaver saver(painter);
    if (backgroundBrush)
        painter->setBackgroundBrush(adjustBrushToRect(*backgroundBrush, pos));
    if (foregroundBrush)
        painter->setForegroundBrush(adjustBrushToRect(*foregroundBrush, pos));
    if (outlineBrush)
        painter->setOutlineBrush(adjustBrushToRect(*outlineBrush, pos));
    if (renderBackground(painter, pos, depth))
        ++depth;
    if (renderContents(painter, pos, depth))
        ++depth;
    for (auto &layoutItem : m_layoutItems)
        layoutItem.item()->render(painter, pos + layoutItem.offset, depth);
}

Item *Item::mouseEvent(const TouchEvent &event)
{
    if (!visible)
        return nullptr;
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
            auto *item = it->item();
            const auto &offset = it->offset;
            const auto childRect = RectF{offset, offset + glm::vec2(item->width(), item->height())};
            if (childRect.contains(pos))
            {
                TouchEvent childEvent = event;
                childEvent.position -= offset;
                if (auto *handler = item->mouseEvent(childEvent); handler)
                    return handler;
            }
        }
        break;
    }
    default:
        break;
    }
    return handleMouseEvent(event);
}

Item *Item::handleMouseEvent(const TouchEvent &)
{
    return nullptr;
}

void Item::setContainerAlignment(Alignment alignment)
{
    if (alignment == m_containerAlignment)
        return;
    m_containerAlignment = alignment;
    alignmentChangedSignal();
}

void Item::setLeft(const Length &position)
{
    setHorizontalAnchor({HorizontalAnchor::Type::Left, position});
}

void Item::setHorizontalCenter(const Length &position)
{
    setHorizontalAnchor({HorizontalAnchor::Type::Center, position});
}

void Item::setRight(const Length &position)
{
    setHorizontalAnchor({HorizontalAnchor::Type::Right, position});
}

void Item::setHorizontalAnchor(const HorizontalAnchor &anchor)
{
    if (anchor == m_horizontalAnchor)
        return;
    m_horizontalAnchor = anchor;
    anchorChangedSignal();
}

void Item::setTop(const Length &position)
{
    setVerticalAnchor({VerticalAnchor::Type::Top, position});
}

void Item::setVerticalCenter(const Length &position)
{
    setVerticalAnchor({VerticalAnchor::Type::Center, position});
}

void Item::setBottom(const Length &position)
{
    setVerticalAnchor({VerticalAnchor::Type::Bottom, position});
}

void Item::setVerticalAnchor(const VerticalAnchor &anchor)
{
    if (anchor == m_verticalAnchor)
        return;
    m_verticalAnchor = anchor;
    anchorChangedSignal();
}

void Item::setShaderEffect(std::unique_ptr<ShaderEffect> effect)
{
    m_effect = std::move(effect);
    m_effect->setSource(this);
}

ShaderEffect *Item::shaderEffect() const
{
    return m_effect.get();
}

void Item::clearShaderEffect()
{
    m_effect.reset();
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

bool Label::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return false;

    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    if (availableHeight < 0.0f)
        return false;

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
    painter->drawText(m_text, textPos, depth);

    if (clipped)
        painter->setClipRect(prevClipRect);

    return true;
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
        auto *cache = sys::pixmapCache();
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

bool Image::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    if (!m_pixmap)
        return false;

    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    const bool clipped = availableWidth < m_pixmap->width - 0.5f || availableHeight < m_pixmap->height - 0.5f;

    const auto topLeft = pos + glm::vec2(m_margins.left, m_margins.top);
    const auto imagePos = topLeft + m_offset;
    const auto rect = RectF{imagePos, imagePos + glm::vec2(m_pixmap->width, m_pixmap->height)};
    if (!clipped)
    {
        painter->drawPixmap(*m_pixmap, rect, depth);
    }
    else
    {
        const auto clipRect = RectF{topLeft, topLeft + glm::vec2(availableWidth, availableHeight)};
        painter->drawPixmap(*m_pixmap, rect, clipRect, depth);
    }

    return true;
}

void Container::setMargins(Margins margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    updateSize();
}

void Container::setSpacing(float spacing)
{
    if (spacing == m_spacing)
        return;
    m_spacing = spacing;
    updateSize();
}

void Container::handleChildUpdated()
{
    updateSize();
    Item::handleChildUpdated();
}

void Column::setMinimumWidth(float width)
{
    if (width == m_minimumWidth)
        return;
    m_minimumWidth = width;
    updateSize();
}

void Column::updateSize()
{
    float width = std::max(m_minimumWidth - (m_margins.left + m_margins.right), 0.0f);
    float height = 0;
    for (auto &layoutItem : m_layoutItems)
    {
        auto *item = layoutItem.item();
        width = std::max(width, item->width());
        height += item->height();
    }
    if (!m_layoutItems.empty())
        height += (m_layoutItems.size() - 1) * m_spacing;
    width += m_margins.left + m_margins.right;
    height += m_margins.top + m_margins.bottom;
    setSize({width, height});
}

void Column::updateLayout()
{
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, item = layoutItem.item()] {
            const auto alignment =
                item->containerAlignment() & (Alignment::Left | Alignment::HCenter | Alignment::Right);
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
        layoutItem.offset = p + glm::vec2(offset, 0.0f);
        p.y += layoutItem.item()->height() + m_spacing;
    }
}

void Row::setMinimumHeight(float height)
{
    if (height == m_minimumHeight)
        return;
    m_minimumHeight = height;
    updateSize();
}

void Row::updateSize()
{
    float width = 0;
    float height = std::max(m_minimumHeight - (m_margins.top + m_margins.bottom), 0.0f);
    for (auto &layoutItem : m_layoutItems)
    {
        auto *item = layoutItem.item();
        width += item->width();
        height = std::max(height, item->height());
    }
    if (!m_layoutItems.empty())
        width += (m_layoutItems.size() - 1) * m_spacing;
    width += m_margins.left + m_margins.right;
    height += m_margins.top + m_margins.bottom;
    setSize({width, height});
}

void Row::updateLayout()
{
    auto p = glm::vec2(m_margins.left, m_margins.top);
    for (auto &layoutItem : m_layoutItems)
    {
        const float offset = [this, item = layoutItem.item()] {
            const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
            const auto alignment =
                item->containerAlignment() & (Alignment::Top | Alignment::VCenter | Alignment::Bottom);
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
        layoutItem.offset = p + glm::vec2(0.0f, offset);
        p.x += layoutItem.item()->width() + m_spacing;
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
    Item::update(elapsed);
    m_contentItem->update(elapsed);
}

bool ScrollArea::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto viewportPos = pos + glm::vec2(m_margins.left, m_margins.top);
    const auto prevClipRect = painter->clipRect();
    const auto viewportRect = RectF{viewportPos, viewportPos + glm::vec2(m_viewportSize.width, m_viewportSize.height)};
    painter->setClipRect(prevClipRect.intersected(viewportRect));
    m_contentItem->render(painter, viewportPos + m_viewportOffset, depth);
    painter->setClipRect(prevClipRect);

    return true;
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
    auto children = Item::children();
    children.insert(children.begin(), m_contentItem.get());
    return children;
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
    backgroundBrush = glm::vec4(0, 0, 0, 1);

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
    Item::update(elapsed);
    m_animation.update(elapsed);
}

bool Switch::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const float radius = 0.5f * m_size.height;
    const float indicatorRadius = 0.75f * radius;
    const float centerX = radius + m_indicatorPosition * (m_size.width - 2 * radius);
    const auto center = pos + glm::vec2(centerX, 0.5f * m_size.height);
    auto oldBrush = painter->backgroundBrush();
    painter->setBackgroundBrush(indicatorColor);
    painter->drawCircle(center, indicatorRadius, depth);
    painter->setBackgroundBrush(oldBrush);
    return true;
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

bool MultiLineText::renderContents(Painter *painter, const glm::vec2 &pos, int depth)
{
    const auto availableWidth = m_size.width - (m_margins.left + m_margins.right);
    if (availableWidth < 0.0f)
        return false;

    const auto availableHeight = m_size.height - (m_margins.top + m_margins.bottom);
    if (availableHeight < 0.0f)
        return false;

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
        painter->drawText(line.text, textPos + glm::vec2(offset, 0), depth);
        textPos.y += m_font->pixelHeight();
    }

    if (clipped)
        painter->setClipRect(prevClipRect);

    return true;
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
