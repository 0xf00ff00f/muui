#pragma once

#include "brush.h"
#include "font.h"
#include "textureatlas.h"
#include "touchevent.h"
#include "tweening.h"
#include "valueanimation.h"

#include <muslots/muslots.h>

#include <glm/glm.hpp>

#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace muui
{
class Painter;
class ShaderEffect;
class Transform;

enum class Alignment : unsigned
{
    None = 0,
    Left = 1 << 0,
    HCenter = 1 << 1,
    Right = 1 << 2,
    Top = 1 << 3,
    VCenter = 1 << 4,
    Bottom = 1 << 5,
    Center = HCenter | VCenter
};

constexpr Alignment operator&(Alignment x, Alignment y)
{
    using UT = typename std::underlying_type_t<Alignment>;
    return static_cast<Alignment>(static_cast<UT>(x) & static_cast<UT>(y));
}

constexpr Alignment &operator&=(Alignment &x, Alignment y)
{
    return x = x & y;
}

constexpr Alignment operator|(Alignment x, Alignment y)
{
    using UT = typename std::underlying_type_t<Alignment>;
    return static_cast<Alignment>(static_cast<UT>(x) | static_cast<UT>(y));
}

constexpr Alignment &operator|=(Alignment &x, Alignment y)
{
    return x = x | y;
}

struct Margins
{
    float top = 0;
    float bottom = 0;
    float left = 0;
    float right = 0;

    bool operator==(const Margins &other) const = default;

    Margins &operator*=(float s)
    {
        top *= s;
        bottom *= s;
        left *= s;
        right *= s;
        return *this;
    }

    Margins operator*(float s) const { return Margins(*this) *= s; }
};

inline Margins operator*(float s, const Margins &margins)
{
    return margins * s;
}

struct Size
{
    float width = 0;
    float height = 0;

    bool operator==(const Size &other) const = default;

    Size &operator*=(float s)
    {
        width *= s;
        height *= s;
        return *this;
    }

    Size operator*(float s) const { return Size(*this) *= s; }
};

inline Size operator*(float s, const Size &size)
{
    return size * s;
}

struct Length
{
    static constexpr Length pixels(float value) { return {Type::Pixels, value}; }
    static constexpr Length percent(float value) { return {Type::Percent, value * 0.01f}; }

    enum class Type
    {
        Pixels,
        Percent,
    };
    Type type{Type::Pixels};
    float value{0.0f};

    bool operator==(const Length &other) const = default;
};

constexpr Length operator""_px(long double value)
{
    return Length::pixels(value);
}

constexpr Length operator""_pct(long double value)
{
    return Length::percent(value);
}

class Item
{
public:
    Item();
    virtual ~Item();

    Size size() const { return m_size; }
    float width() const { return m_size.width; }
    float height() const { return m_size.height; }
    RectF rect() const { return RectF{{0, 0}, {m_size.width, m_size.height}}; }

    void render(Painter *painter, int depth = 0);

    Item *mouseEvent(const TouchEvent &event);
    virtual void update(float elapsed);

    template<typename ChildT, typename... Args>
        requires std::derived_from<ChildT, Item>
    ChildT *appendChild(Args &&...args)
    {
        return insertChild<ChildT>(m_layoutItems.size(), std::forward<Args>(args)...);
    }

    template<typename ChildT, typename... Args>
        requires std::derived_from<ChildT, Item>
    ChildT *insertChild(std::size_t index, Args &&...args)
    {
        auto it = m_layoutItems.emplace(std::next(m_layoutItems.begin(), index),
                                        std::make_unique<ChildT>(std::forward<Args>(args)...), this);
        handleChildUpdated();
        return static_cast<ChildT *>(it->item());
    }

    void removeChild(std::size_t index);
    std::unique_ptr<Item> takeChildAt(std::size_t index);
    Item *childAt(std::size_t index) const;
    std::size_t childCount() const { return m_layoutItems.size(); }
    virtual std::vector<Item *> children() const;
    RectF childRect(std::size_t index);

    template<typename ChildT>
        requires std::derived_from<ChildT, Item>
    ChildT *findChild(std::string_view name)
    {
        if (auto *typedItem = dynamic_cast<ChildT *>(this); typedItem)
        {
            if (typedItem->objectName == name)
                return typedItem;
        }
        const auto children = this->children();
        for (auto *child : children)
        {
            if (auto *result = child->findChild<ChildT>(name); result)
                return result;
        }
        return nullptr;
    }

    void setRotation(float angle);
    float rotation() const { return m_rotation; }

    void setTransformOrigin(const glm::vec2 &transformOrigin);
    glm::vec2 transformOrigin() const { return m_transformOrigin; }

    void setContainerAlignment(Alignment alignment);
    Alignment containerAlignment() const { return m_containerAlignment; }

    void setLeft(const Length &position);
    void setHorizontalCenter(const Length &position);
    void setRight(const Length &position);

    void setTop(const Length &position);
    void setVerticalCenter(const Length &position);
    void setBottom(const Length &position);

    void setShaderEffect(std::unique_ptr<ShaderEffect> effect);
    void clearShaderEffect();
    ShaderEffect *shaderEffect() const;

    std::string objectName;
    bool visible = true;
    enum class Shape
    {
        Rectangle,
        Capsule,
        Circle,
        RoundedRectangle,
    };
    Shape shape = Shape::Rectangle;
    bool fillBackground = false;
    std::optional<Brush> backgroundBrush;
    std::optional<Brush> foregroundBrush;
    std::optional<Brush> outlineBrush;
    float cornerRadius = 0.0f;

    muslots::Signal<Size> resizedSignal;
    muslots::Signal<> anchorChangedSignal;
    muslots::Signal<> alignmentChangedSignal;

protected:
    struct HorizontalAnchor
    {
        enum class Type
        {
            Left,
            Center,
            Right
        };
        Type type{Type::Left};
        Length position{Length::pixels(0.0f)};

        bool operator==(const HorizontalAnchor &) const = default;
    };

    struct VerticalAnchor
    {
        enum class Type
        {
            Top,
            Center,
            Bottom
        };
        Type type{Type::Top};
        Length position{Length::pixels(0.0f)};

        bool operator==(const VerticalAnchor &) const = default;
    };

    void setSize(Size size);
    void setHorizontalAnchor(const HorizontalAnchor &anchor);
    void setVerticalAnchor(const VerticalAnchor &anchor);
    virtual void updateLayout();
    bool renderBackground(Painter *painter, int depth);
    virtual bool renderContents(Painter *painter, int depth = 0);
    virtual Item *handleMouseEvent(const TouchEvent &event);
    virtual void handleChildUpdated();
    Brush adjustBrushToRect(const Brush &brush, const Transform &transform) const;

    class LayoutItem
    {
    public:
        LayoutItem(std::unique_ptr<Item> item, Item *parent);
        ~LayoutItem();

        LayoutItem(LayoutItem &) = delete;
        LayoutItem &operator=(LayoutItem &) = delete;

        LayoutItem(LayoutItem &&other) = default;
        LayoutItem &operator=(LayoutItem &&other) = default;

        glm::vec2 offset{0.0f};

        Item *item() const { return m_item.get(); }
        std::unique_ptr<Item> takeItem() { return std::move(m_item); }

    private:
        std::unique_ptr<Item> m_item;
        muslots::Connection m_resizedConnection;
        muslots::Connection m_anchorChangedConnection;
        muslots::Connection m_alignmentChangedConnection;
    };

    Size m_size;
    float m_rotation{0.0f};
    glm::vec2 m_transformOrigin{0.0f};
    Alignment m_containerAlignment{Alignment::VCenter | Alignment::Left};
    std::vector<LayoutItem> m_layoutItems;
    HorizontalAnchor m_horizontalAnchor{};
    VerticalAnchor m_verticalAnchor{};

private:
    void doRender(Painter *painter, int depth);

    std::unique_ptr<ShaderEffect> m_effect;

    friend class ShaderEffect;
};

class Rectangle : public Item
{
public:
    Rectangle();
    Rectangle(Size size);
    Rectangle(float width, float height);

    using Item::setSize;
    void setSize(float width, float height);
    void setWidth(float width);
    void setHeight(float height);
};

class Label : public Item
{
public:
    explicit Label(std::u32string_view text = {});
    Label(Font *font, std::u32string_view text = {});

    void setFont(Font *font);
    Font *font() const { return m_font; }

    void setText(std::u32string_view text);
    const std::u32string &text() const { return m_text; }

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setFixedWidth(float width);
    float fixedWidth() const { return m_fixedWidth; }

    void setFixedHeight(float height);
    float fixedHeight() const { return m_fixedHeight; }

    void setAlignment(Alignment alignment);
    Alignment alignment() { return m_alignment; }

    bool shadowEnabled = false;
    glm::vec2 shadowOffset = glm::vec2(4, 4);
    glm::vec4 shadowColor = glm::vec4(0.5, 0.5, 0.5, 1);

    muslots::Signal<> clickedSignal;

protected:
    bool renderContents(Painter *painter, int depth = 0) override;
    Item *handleMouseEvent(const TouchEvent &event) override;
    void updateSizeAndOffset();

    Alignment m_alignment = Alignment::VCenter | Alignment::Left;
    Font *m_font;
    std::u32string m_text;
    Margins m_margins;
    float m_contentWidth = 0;
    float m_contentHeight = 0;
    glm::vec2 m_offset;
    float m_fixedWidth = -1;  // ignored if < 0
    float m_fixedHeight = -1; // ignored if < 0
};

class Image : public Item
{
public:
    Image();
    explicit Image(std::string_view source);

    void setSource(std::string_view source);
    const std::string &source() const { return m_source; }

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setFixedWidth(float width);
    float fixedWidth() const { return m_fixedWidth; }

    void setFixedHeight(float height);
    float fixedHeight() const { return m_fixedHeight; }

    void setAlignment(Alignment alignment);
    Alignment alignment() { return m_alignment; }

protected:
    bool renderContents(Painter *painter, int depth = 0) override;
    Item *handleMouseEvent(const TouchEvent &event) override;
    void updateSizeAndOffset();

    Alignment m_alignment = Alignment::VCenter | Alignment::Left;
    std::string m_source;
    std::optional<PackedPixmap> m_pixmap;
    Margins m_margins;
    glm::vec2 m_offset;
    float m_fixedWidth = -1;  // ignored if < 0
    float m_fixedHeight = -1; // ignored if < 0
};

class Container : public Item
{
public:
    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setSpacing(float spacing);
    float spacing() const { return m_spacing; }

protected:
    void handleChildUpdated() override;
    virtual void updateSize() = 0;

    Margins m_margins;
    float m_spacing = 0.0f;
};

class Column : public Container
{
public:
    void setMinimumWidth(float width);
    float minimumWidth() const { return m_minimumWidth; }

private:
    void updateSize() override;
    void updateLayout() override;

    float m_minimumWidth = 0;
};

class Row : public Container
{
public:
    void setMinimumHeight(float height);
    float minimumHeight() const { return m_minimumHeight; }

private:
    void updateSize() override;
    void updateLayout() override;

    float m_minimumHeight = 0;
};

class ScrollArea : public Item
{
public:
    explicit ScrollArea(std::unique_ptr<Item> contentItem);
    ScrollArea(float viewportWidth, float viewportHeight, std::unique_ptr<Item> contentItem);

    std::vector<Item *> children() const override;

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setViewportSize(Size size);
    Size viewportSize() const;

protected:
    void update(float elapsed) override;
    bool renderContents(Painter *painter, int depth = 0) override;
    Item *handleMouseEvent(const TouchEvent &event) override;

private:
    void updateSize();

    std::unique_ptr<Item> m_contentItem;
    Margins m_margins;
    Size m_viewportSize;
    glm::vec2 m_viewportOffset = glm::vec2(0, 0);
};

class MultiLineText : public Item
{
public:
    explicit MultiLineText(std::u32string_view text = {});
    MultiLineText(Font *font, std::u32string_view text = {});

    void setFont(Font *font);
    Font *font() const { return m_font; }

    void setText(std::u32string_view text);
    const std::u32string &text() const { return m_text; }

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setFixedWidth(float width);
    float fixedWidth() const { return m_fixedWidth; }

    void setFixedHeight(float height);
    float fixedHeight() const { return m_fixedHeight; }

    Alignment alignment = Alignment::VCenter | Alignment::Left;

protected:
    bool renderContents(Painter *painter, int depth = 0) override;

private:
    void updateSize();
    void breakTextLines();

    Font *m_font;
    std::u32string m_text;
    Margins m_margins;
    float m_fixedWidth = 200.0f; // mandatory!
    float m_fixedHeight = -1;    // ignored if < 0
    float m_contentWidth = 0.0f;
    float m_contentHeight = 0.0f;
    struct TextLine
    {
        std::u32string_view text;
        float width;
    };
    std::vector<TextLine> m_lines;
};

class Switch : public Item
{
public:
    Switch();
    Switch(Size size);
    Switch(float width, float height);

    void setChecked(bool checked);
    bool isChecked() { return m_checked; }
    void toggle();

    glm::vec4 indicatorColor = glm::vec4(1, 1, 1, 1);

    muslots::Signal<bool> toggledSignal;

protected:
    void update(float elapsed) override;
    bool renderContents(Painter *painter, int depth = 0) override;
    Item *handleMouseEvent(const TouchEvent &event) override;

    bool m_checked = false;
    ValueAnimation<tweening::InQuadratic<float>, float> m_animation;
    float m_indicatorPosition = 0.0f;
};

class Button : public Label
{
public:
    using Label::Label;

    muslots::Signal<> clickedSignal;

protected:
    Item *handleMouseEvent(const TouchEvent &event) override;
};

class ImageButton : public Image
{
public:
    using Image::Image;

    muslots::Signal<> clickedSignal;

protected:
    Item *handleMouseEvent(const TouchEvent &event) override;
};

} // namespace muui
