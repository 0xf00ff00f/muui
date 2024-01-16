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

    auto outerContainer = std::make_unique<Column>();
    outerContainer->setMargins(Margins{outerMargin, outerMargin, outerMargin, outerMargin});
    outerContainer->setSpacing(5);

    auto *title = outerContainer->appendChild<Label>(bigFont, U"TODAY'S HEROES"sv);
    title->brush = glm::vec4{headingColor, 1};
    title->setContainerAlignment(Alignment::VCenter | Alignment::HCenter);

    auto *innerContainer = outerContainer->appendChild<Column>();
    innerContainer->setContainerAlignment(Alignment::VCenter | Alignment::HCenter);
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

        auto appendSeparator = [&textColor](Item *parent) {
            auto *r = parent->appendChild<Rectangle>();
            r->fillBackground = true;
            r->backgroundBrush = glm::vec4{textColor, 0.25};
            r->setSize(1, 30);
        };

        auto *headerRow = innerContainer->appendChild<Row>();

        {
            auto *r = headerRow->appendChild<Rectangle>();
            r->setSize(rowMargin + indexColumnWidth + 1, 1);
        }

        {
            auto *l = headerRow->appendChild<Label>(smallFont, U"NAME"sv);
            l->brush = glm::vec4{textColor, 1.0f};
            l->setAlignment(Alignment::Left);
            l->setFixedWidth(nameColumnWidth);
        }

        appendSeparator(headerRow);

        {
            auto *l = headerRow->appendChild<Label>(smallFont, U"SCORE"sv);
            l->brush = glm::vec4{textColor, 1.0f};
            l->setAlignment(Alignment::HCenter);
            l->setFixedWidth(scoreColumnWidth);
        }

        appendSeparator(headerRow);

        {
            auto *l = headerRow->appendChild<Label>(smallFont, U"ACCURACY"sv);
            l->brush = glm::vec4{textColor, 1.0f};
            l->setAlignment(Alignment::Right);
            l->setFixedWidth(accuracyColumnWidth);
        }

        {
            auto *r = headerRow->appendChild<Rectangle>();
            r->setSize(rowMargin, 1);
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

            auto *row = entryColumn->appendChild<Row>();
            row->fillBackground = true;
            row->shape = Item::Shape::RoundedRectangle;
            row->cornerRadius = 8;
            row->backgroundBrush = glm::vec4{1, 1, 1, 0.75};
            row->setMargins(Margins{rowMargin, rowMargin, rowMargin, rowMargin});
            row->setSpacing(1);

            auto *index = row->appendChild<Label>(smallFont, fmt::format(U"{}.", i + 1));
            index->brush = glm::vec4{textColor, 1.0f};
            index->setFixedWidth(indexColumnWidth);

            auto *nameLabel = row->appendChild<Label>(smallFont, entry.name);
            nameLabel->brush = glm::vec4{headingColor, 1.0f};
            nameLabel->setFixedWidth(nameColumnWidth);

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

            auto *scoreLabel = row->appendChild<Label>(smallFont, formatThousands(entry.score));
            scoreLabel->brush = glm::vec4{textColor, 1.0f};
            scoreLabel->setFixedWidth(scoreColumnWidth);
            scoreLabel->setAlignment(Alignment::HCenter);

            auto *accuracyLabel = row->appendChild<Label>(smallFont, fmt::format(U"{:.2f}%", entry.accuracy));
            accuracyLabel->brush = glm::vec4{textColor, 1.0f};
            accuracyLabel->setFixedWidth(accuracyColumnWidth);
            accuracyLabel->setAlignment(Alignment::Right);
        }

        assert(entryColumn->width() == width - 2 * outerMargin);

        const auto containerHeight = height - 2 * outerMargin - (title->height() + outerContainer->spacing());

        const auto viewportWidth = entryColumn->width();
        const auto viewportHeight = containerHeight - (headerRow->height() + innerContainer->spacing());

        auto *scrollArea = innerContainer->appendChild<ScrollArea>(std::move(entryColumn));
        scrollArea->setViewportSize({viewportWidth, viewportHeight});

        assert(innerContainer->width() == viewportWidth);
        assert(innerContainer->height() == containerHeight);
    }

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
