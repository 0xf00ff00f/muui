#pragma once

#include "font.h"
#include "shadereffect.h"
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

class Item
{
public:
    virtual ~Item();

    Size size() const { return m_size; }
    float width() const { return m_size.width; }
    float height() const { return m_size.height; }
    RectF rect() const { return RectF{{0, 0}, {m_size.width, m_size.height}}; }

    void render(Painter *painter, const glm::vec2 &pos, int depth = 0);

    Item *mouseEvent(const TouchEvent &event);
    virtual void update(float elapsed);
    virtual std::vector<Item *> children() const;

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

    template<typename EffectT, typename... Args>
        requires std::derived_from<EffectT, ShaderEffect>
    EffectT *setShaderEffect(Args &&...args)
    {
        auto effect = std::make_unique<EffectT>(std::forward<Args>(args)...);
        const auto w = static_cast<int>(std::ceil(m_size.width));
        const auto h = static_cast<int>(std::ceil(m_size.height));
        effect->resize(w, h);
        m_effect = std::move(effect);
        return static_cast<EffectT *>(m_effect.get());
    }

    void clearShaderEffect();

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
    glm::vec4 backgroundColor;
    float cornerRadius = 0.0f;
    Alignment containerAlignment = Alignment::VCenter | Alignment::Left;

    muslots::Signal<Size> resizedSignal;

protected:
    void setSize(Size size);
    void renderBackground(Painter *painter, const glm::vec2 &pos, int depth);
    virtual void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) = 0;
    virtual Item *handleMouseEvent(const TouchEvent &event);

    Size m_size;

private:
    void doRender(Painter *painter, const glm::vec2 &pos, int depth);

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

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;
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

    glm::vec4 color = glm::vec4(0, 0, 0, 1);
    bool shadowEnabled = false;
    glm::vec2 shadowOffset = glm::vec2(4, 4);
    glm::vec4 shadowColor = glm::vec4(0.5, 0.5, 0.5, 1);

    muslots::Signal<> clickedSignal;

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;
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

    glm::vec4 color = glm::vec4(1, 1, 1, 1);

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;
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
    std::vector<Item *> children() const override;

    void append(std::unique_ptr<Item> item);
    void insert(std::size_t index, std::unique_ptr<Item> item);
    void remove(std::size_t index);
    std::unique_ptr<Item> takeAt(std::size_t index);
    Item *at(std::size_t index) const;
    std::size_t size() const { return m_layoutItems.size(); }

    void setMargins(Margins margins);
    Margins margins() const { return m_margins; }

    void setSpacing(float spacing);
    float spacing() const { return m_spacing; }

protected:
    void update(float elapsed) override;
    virtual void updateLayout() = 0;
    Item *handleMouseEvent(const TouchEvent &event) override;

    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;

    struct LayoutItem
    {
        glm::vec2 offset;
        std::unique_ptr<Item> item;
        muslots::Connection resizedConnection;
    };
    std::vector<std::unique_ptr<LayoutItem>> m_layoutItems;
    Margins m_margins;
    float m_spacing = 0.0f;
};

class Column : public Container
{
public:
    void setMinimumWidth(float width);
    float minimumWidth() const { return m_minimumWidth; }

private:
    void updateLayout() override;

    float m_minimumWidth = 0;
};

class Row : public Container
{
public:
    void setMinimumHeight(float height);
    float minimumHeight() const { return m_minimumHeight; }

private:
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
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;
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

    glm::vec4 color = glm::vec4(0, 0, 0, 1);
    Alignment alignment = Alignment::VCenter | Alignment::Left;

protected:
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;

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
    void renderContents(Painter *painter, const glm::vec2 &pos, int depth = 0) override;
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
