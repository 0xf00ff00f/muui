#include "testwindow.h"

#include "panic.h"

#include <muui/font.h>
#include <muui/item.h>
#include <muui/painter.h>
#include <muui/screen.h>
#include <muui/textureatlas.h>

#include <fmt/core.h>
#include <fmt/xchar.h>

#include <iostream>
#include <memory>
#include <string>

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace muui;

namespace
{

constexpr glm::vec3 rgbToColor(unsigned color)
{
    const auto r = static_cast<float>((color >> 16) & 0xff);
    const auto g = static_cast<float>((color >> 8) & 0xff);
    const auto b = static_cast<float>(color & 0xff);
    return (1.0f / 255.0f) * glm::vec3(r, g, b);
}

std::unique_ptr<Item> buildUI(Font *smallFont, Font *bigFont, float width, float height)
{
    constexpr auto headingColor = rgbToColor(0x04568e);
    constexpr auto textColor = rgbToColor(0x040a18);
    constexpr auto outerMargin = 40;

    auto makeLabel = [](std::u32string_view text, Font *font, const glm::vec3 &color) {
        auto l = std::make_unique<Label>(font, text);
        l->brush = glm::vec4(color, 1);
        return l;
    };

    auto outerContainer = std::make_unique<Column>();
    outerContainer->setMargins(Margins{outerMargin, outerMargin, outerMargin, outerMargin});
    outerContainer->setSpacing(5);

    auto title = makeLabel(U"TODAY'S HEROES"sv, bigFont, headingColor);
    title->containerAlignment = Alignment::VCenter | Alignment::HCenter;

    auto innerContainer = std::make_unique<Column>();
    innerContainer->containerAlignment = Alignment::VCenter | Alignment::HCenter;
    innerContainer->setSpacing(5);

    {
        constexpr auto rowMargin = 10;

        constexpr auto indexColumnWidth = 40;
        constexpr auto scoreColumnWidth = 300;
        constexpr auto accuracyColumnWidth = 300;
        const auto nameColumnWidth = [=] {
            const auto columnCount = 4;
            const auto usedWidth = 2 * outerMargin + 2 * rowMargin + indexColumnWidth + scoreColumnWidth +
                                   accuracyColumnWidth + (columnCount - 1);
            return width - usedWidth;
        }();

        auto makeSeparator = [&textColor] {
            auto r = std::make_unique<Rectangle>();
            r->fillBackground = true;
            r->backgroundBrush = glm::vec4{textColor, 0.25};
            r->setSize(1, 30);
            return r;
        };

        auto headerRow = std::make_unique<Row>();

        {
            auto r = std::make_unique<Rectangle>();
            r->setSize(rowMargin + indexColumnWidth + 1, 1);
            headerRow->appendChild(std::move(r));
        }

        {
            auto l = makeLabel(U"NAME"sv, smallFont, textColor);
            l->setAlignment(Alignment::Left);
            l->setFixedWidth(nameColumnWidth);
            headerRow->appendChild(std::move(l));
        }

        headerRow->appendChild(makeSeparator());

        {
            auto l = makeLabel(U"SCORE"sv, smallFont, textColor);
            l->setAlignment(Alignment::HCenter);
            l->setFixedWidth(scoreColumnWidth);
            headerRow->appendChild(std::move(l));
        }

        headerRow->appendChild(makeSeparator());

        {
            auto l = makeLabel(U"ACCURACY"sv, smallFont, textColor);
            l->setAlignment(Alignment::Right);
            l->setFixedWidth(accuracyColumnWidth);
            headerRow->appendChild(std::move(l));
        }

        {
            auto r = std::make_unique<Rectangle>();
            r->setSize(rowMargin, 1);
            headerRow->appendChild(std::move(r));
        }

        assert(headerRow->width() == width - 2 * outerMargin);

        // entries column

        auto entryColumn = std::make_unique<Column>();
        entryColumn->setSpacing(5);

        struct Entry
        {
            std::u32string name;
            int score;
            float accuracy;
        };
        static std::vector<Entry> entries = [] {
            static std::vector<const char32_t *> names = {U"ALICE", U"BOB",  U"CHARLIE", U"DAVID", U"EVE",
                                                          U"FRANK", U"GINA", U"HARRIET", U"IVAN"};
            std::vector<Entry> entries;
            for (const auto *name : names)
            {
                Entry entry;
                entry.name = name;
                entry.score = std::rand();
                entry.accuracy = 100.0f * static_cast<float>(std::rand()) / RAND_MAX;
                entries.push_back(entry);
            }
            std::ranges::sort(entries, [](const auto &lhs, const auto &rhs) { return lhs.score > rhs.score; });
            return entries;
        }();

        for (std::size_t i = 0; i < entries.size(); ++i)
        {
            const auto &entry = entries[i];

            auto row = std::make_unique<Row>();
            row->fillBackground = true;
            row->shape = Item::Shape::RoundedRectangle;
            row->cornerRadius = 8;
            row->backgroundBrush = glm::vec4{1, 1, 1, 0.75};
            row->setMargins(Margins{rowMargin, rowMargin, rowMargin, rowMargin});
            row->setSpacing(1);

            auto index = makeLabel(fmt::format(U"{}.", i + 1), smallFont, textColor);
            index->setFixedWidth(indexColumnWidth);
            row->appendChild(std::move(index));

            auto nameLabel = makeLabel(entry.name, smallFont, headingColor);
            nameLabel->setFixedWidth(nameColumnWidth);
            row->appendChild(std::move(nameLabel));

            auto formatThousands = [](int value) -> std::u32string {
                std::u32string text;
                while (value)
                {
                    auto s = fmt::format(value >= 1000 ? U"{:03}" : U"{}", value % 1000);
                    if (text.empty())
                        text = std::move(s);
                    else
                        text = s + U"," + text;
                    value /= 1000;
                }
                return text;
            };

            auto scoreLabel = makeLabel(formatThousands(entry.score), smallFont, textColor);
            scoreLabel->setFixedWidth(scoreColumnWidth);
            scoreLabel->setAlignment(Alignment::HCenter);
            row->appendChild(std::move(scoreLabel));

            auto accuracyLabel = makeLabel(fmt::format(U"{:.2f}%", entry.accuracy), smallFont, textColor);
            accuracyLabel->setFixedWidth(accuracyColumnWidth);
            accuracyLabel->setAlignment(Alignment::Right);
            row->appendChild(std::move(accuracyLabel));

            entryColumn->appendChild(std::move(row));
        }

        assert(entryColumn->width() == width - 2 * outerMargin);

        const auto containerHeight = height - 2 * outerMargin - (title->height() + outerContainer->spacing());

        const auto viewportWidth = entryColumn->width();
        const auto viewportHeight = containerHeight - (headerRow->height() + innerContainer->spacing());

        auto scrollArea = std::make_unique<ScrollArea>(std::move(entryColumn));
        scrollArea->setViewportSize({viewportWidth, viewportHeight});

        innerContainer->appendChild(std::move(headerRow));
        innerContainer->appendChild(std::move(scrollArea));

        assert(innerContainer->width() == viewportWidth);
        assert(innerContainer->height() == containerHeight);
    }

    outerContainer->appendChild(std::move(title));
    outerContainer->appendChild(std::move(innerContainer));
    assert(outerContainer->width() == width);
    assert(outerContainer->height() == height);

    return outerContainer;
}

} // namespace

class LeaderboardTest : public TestWindow
{
public:
    using TestWindow::TestWindow;

    void initialize() override;
    void render() override;
    void mouseButtonEvent(int button, int action, int mods) override;
    void mouseMoveEvent(double x, double y) override;

private:
    std::unique_ptr<TextureAtlas> m_textureAtlas;
    std::unique_ptr<Font> m_smallFont, m_bigFont;
    std::unique_ptr<Item> m_rootItem;
    std::unique_ptr<Screen> m_screen;
};

void LeaderboardTest::initialize()
{
    constexpr auto fontPath = ASSETSDIR "OpenSans_Bold.ttf";

    m_textureAtlas = std::make_unique<TextureAtlas>(512, 512, PixelType::Grayscale);
    m_smallFont = std::make_unique<Font>(m_textureAtlas.get());
    if (!m_smallFont->load(fontPath, 50))
        panic("Failed to load font\n");
    m_bigFont = std::make_unique<Font>(m_textureAtlas.get());
    if (!m_bigFont->load(fontPath, 70))
        panic("Failed to load font\n");

    m_rootItem = buildUI(m_smallFont.get(), m_bigFont.get(), m_width, m_height);

    m_screen = std::make_unique<Screen>();
    m_screen->resize(m_width, m_height);
    m_screen->setRootItem(m_rootItem.get());
}

void LeaderboardTest::render()
{
    glClearColor(0.8, 0.95, 1.0, 1);
    glViewport(0, 0, m_screen->width(), m_screen->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screen->render();
}

void LeaderboardTest::mouseButtonEvent(int button, int action, [[maybe_unused]] int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    switch (action)
    {
    case GLFW_PRESS: {
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);
        m_screen->handleTouchEvent(TouchAction::Down, x, y);
        break;
    }
    case GLFW_RELEASE: {
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);
        m_screen->handleTouchEvent(TouchAction::Up, x, y);
        break;
    }
    default:
        break;
    }
}

void LeaderboardTest::mouseMoveEvent(double x, double y)
{
    int state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
        m_screen->handleTouchEvent(TouchAction::Move, x, y);
}

int main(int argc, char *argv[])
{
    LeaderboardTest w(1223, 512, "hello");
    w.run();
}
